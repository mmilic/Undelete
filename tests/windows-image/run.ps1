# End-to-end check of the recovery engines on volumes that Windows itself formatted and wrote - the NTFS and FAT
# tests make theirs with ntfs-3g and pyfatfs. NTFS and exFAT for now (see $Images for FAT32). Needs administrator rights (it attaches virtual disks); the
# GitHub Actions build runs it on every push.
#   1. for each image: creates a virtual disk (VHD), formats it with Windows, writes the test files - fragmented,
#      in folders, through the Recycle Bin, and on NTFS compressed and EFS-encrypted ones - deletes them, then
#      copies the volume out of the VHD into a plain image file
#   2. builds the harness (tests\ntfs-image\build_harness.ps1 - one harness for all the tests)
#   3. recovers every deleted file from each image and checks it against <image>.expected.tsv
# Run from anywhere, in an elevated PowerShell: powershell -ExecutionPolicy Bypass -File tests\windows-image\run.ps1
# -UseExistingImages: skip step 1 and use the images in out\windows-test (no admin rights needed then)
param([switch]$UseExistingImages)
$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Resolve-Path (Join-Path $Here "..\..")
$OutDir = Join-Path $Root "out\windows-test"
# name, file system, cluster size, MB - add @("fat32", "FAT32", 512, 64) once the FAT engine can recover it: on
# FAT32 Windows clears the high word of a deleted file's first cluster, so the files past cluster 65535 point to
# the wrong clusters (the FAT test doesn't see it - its images are small, and pyfatfs keeps the high word)
$Images = @(@("ntfs", "NTFS", 4096, 64), @("exfat", "exFAT", 4096, 64))
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$Rng = New-Object System.Random 20141017
function RandomBytes([int]$n) { $b = New-Object byte[] $n; $Rng.NextBytes($b); return ,$b }
function Sha256([byte[]]$b) {
    $sha = [Security.Cryptography.SHA256]::Create()
    return (($sha.ComputeHash($b) | ForEach-Object { $_.ToString("x2") }) -join "")
}
# does Windows store this name on FAT as an 8.3 entry alone? (8.3, no spaces, base and extension each in one case)
function Fits83([string]$n) {
    if ($n -notmatch '^([A-Za-z0-9_-]{1,8})(\.([A-Za-z0-9_-]{1,3}))?$') { return $false }
    foreach ($part in $Matches[1], $Matches[3]) {
        if ($part -and $part -cne $part.ToUpper() -and $part -cne $part.ToLower()) { return $false }
    }
    return $true
}
function PngBytes([int]$w, [int]$h) {
    Add-Type -AssemblyName System.Drawing
    $bmp = New-Object System.Drawing.Bitmap $w, $h
    for ($x = 0; $x -lt $w; $x++) { for ($y = 0; $y -lt $h; $y++) { $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb($x * 2, $y * 3, 128)) } }
    $ms = New-Object IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    return ,$ms.ToArray()
}

# deleting to the Recycle Bin the way Explorer does, but never with a dialog - nobody could click it in CI
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class RecycleBin {
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct SHFILEOPSTRUCT {
        public IntPtr hwnd; public uint wFunc; public string pFrom; public string pTo; public ushort fFlags;
        public bool fAnyOperationsAborted; public IntPtr hNameMappings; public string lpszProgressTitle;
    }
    [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
    static extern int SHFileOperation(ref SHFILEOPSTRUCT op);
    public static int Delete(string path) {
        SHFILEOPSTRUCT op = new SHFILEOPSTRUCT();
        op.wFunc = 3; // FO_DELETE
        op.pFrom = path + "\0\0";
        op.fFlags = 0x0040 | 0x0010 | 0x0004 | 0x0400; // FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI
        return SHFileOperation(ref op);
    }
}
"@

function New-TestImage($name, $fs, $cluster, $mb) {
    $vhd = Join-Path $OutDir "$name.vhd"
    $img = Join-Path $OutDir "$name.img"
    $tsv = Join-Path $OutDir "$name.expected.tsv"
    Write-Host "=== $name`: $fs, $cluster-byte clusters, $mb MB ==="
    if (Test-Path $vhd) { Dismount-DiskImage -ImagePath $vhd -ErrorAction SilentlyContinue | Out-Null; Remove-Item -Force $vhd }
    Remove-Item -Force $img, $tsv -ErrorAction SilentlyContinue

    # a fixed VHD is the raw disk plus a footer - diskpart makes one without Hyper-V
    $script = Join-Path $OutDir "diskpart.txt"
    Set-Content -Encoding ascii -Path $script -Value "create vdisk file=`"$vhd`" maximum=$mb type=fixed"
    diskpart /s $script | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "diskpart could not create $vhd" }
    $disk = Mount-DiskImage -ImagePath $vhd -PassThru | Get-Disk
    try {
        Initialize-Disk -Number $disk.Number -PartitionStyle MBR
        $part = New-Partition -DiskNumber $disk.Number -UseMaximumSize -AssignDriveLetter
        Format-Volume -Partition $part -FileSystem $fs -AllocationUnitSize $cluster -NewFileSystemLabel "UNDELTEST" -Confirm:$false | Out-Null
        $part = Get-Partition -DiskNumber $disk.Number -PartitionNumber $part.PartitionNumber
        $V = "$($part.DriveLetter):"
        Write-Host "Formatted $V"
        $expected = New-Object System.Collections.Generic.List[string]
        # <name> <data> <recoverability> <folder> <error code> <name it is saved under>; "-" = not checked
        function Expect($n, $data, $recov, $folder, $err, $saved) {
            $hash = if ($data) { Sha256 $data } else { "-" }
            $expected.Add("$n`t$hash`t$recov`t$folder`t$err`t$saved")
        }

        Write-Host "Filling the volume..."
        # fill the volume with 64 KB files, then free every other one -> 64 KB holes between files in use
        $filler = RandomBytes 65536
        $i = 0
        while ($true) {
            try { [IO.File]::WriteAllBytes("$V\fill_$i.bin", $filler) } catch { Remove-Item -Force "$V\fill_$i.bin" -ErrorAction SilentlyContinue; break }
            $i++
        }
        for ($j = 0; $j -lt $i; $j += 2) { Remove-Item -Force "$V\fill_$j.bin" }
        $free = [long][math]::Floor($i / 2) * 65536
        Write-Host "$i filler files, every other one freed: $free bytes in holes"

        # the test files - they are deleted last, so nothing written after them can take their clusters
        $files = [ordered]@{}
        # fragmented into the holes: NTFS knows its fragments, FAT has to skip the clusters in use, so it is
        # never "Good" there, but its data comes back right
        $files["fragmented.bin"] = RandomBytes ([int]($free * 2 / 5) + 1234)
        $files["Čačak ćevapi.txt"] = RandomBytes 3000
        $files["фотографија.jpg"] = RandomBytes 3000
        $files["preview.png"] = PngBytes 120 80
        $files[("long name " * 15).Trim() + ".bin"] = RandomBytes 5000
        # a name that fits 8.3 in one case: on FAT Windows writes no long name (only the lower case flags for
        # "preview.png"), so the first letter is lost for good - "_EADME.TXT", "_review.png"
        $files["README.TXT"] = RandomBytes 2000
        New-Item -ItemType Directory -Force -Path "$V\Photos\2014", "$V\Old Stuff" | Out-Null
        $files["Photos\2014\holiday.png"] = PngBytes 64 48
        $files["Old Stuff\notes.txt"] = [Text.Encoding]::ASCII.GetBytes("Shopping list`r`n- milk`r`n- bread`r`n")
        foreach ($f in $files.Keys) { [IO.File]::WriteAllBytes("$V\$f", $files[$f]) }
        # onto the disk: a file deleted while its data is still only in the cache never had any data on the disk
        Write-VolumeCache -DriveLetter $part.DriveLetter
        Write-Host "Test files written"

        # NTFS keeps its fragments, and Windows leaves a deleted file's FAT chain on exFAT (v2.7's CI run: Good, 201
        # fragments); FAT has to skip the clusters in use
        $fragRecov = if ($fs -eq "NTFS" -or $fs -eq "exFAT") { "Good" } else { "Partly overwritten" }
        foreach ($f in $files.Keys) {
            $leaf = Split-Path -Leaf $f
            $folder = "\" + (Split-Path -Parent $f)
            $saved = if ($fs -like "FAT*" -and (Fits83 $leaf)) { "_" + $leaf.Substring(1) } else { $leaf }
            $recov = if ($leaf -eq "fragmented.bin") { $fragRecov } else { "Good" }
            Expect $saved $files[$f] $recov $folder 0 $saved
        }

        if ($fs -eq "NTFS") {
            # their clusters don't hold the plain data - the engine must refuse them, not copy garbage
            $text = [Text.Encoding]::ASCII.GetBytes(("compressible text " * 4000))
            [IO.File]::WriteAllBytes("$V\compressed.txt", $text)
            Write-Host "Compressing and encrypting..."
            compact /c "$V\compressed.txt" | Out-Null
            if ($LASTEXITCODE -ne 0) { throw "compact /c failed" }
            [IO.File]::WriteAllBytes("$V\secret.txt", $text)
            cipher /e /a "$V\secret.txt" | Out-Null
            if ($LASTEXITCODE -ne 0) { throw "cipher /e failed" }
            if (-not ((Get-Item "$V\secret.txt").Attributes -band [IO.FileAttributes]::Encrypted)) { throw "secret.txt was not encrypted" }
            Expect "compressed.txt" $null "-" "\" 50 "-"   # ERROR_NOT_SUPPORTED
            Expect "secret.txt" $null "-" "\" 6002 "-"     # ERROR_FILE_ENCRYPTED
            $files["compressed.txt"] = $null
            $files["secret.txt"] = $null
        }

        # the Recycle Bin, by Windows: deleting to it renames the file to $R..., and an $I... file keeps its path;
        # emptying it deletes both - the engine must give the $R file its original name and folder back.
        # NTFS only: on FAT the shell's delete to the Recycle Bin hangs on the CI machine, and USB sticks and memory
        # cards - the FAT drives - have no Recycle Bin anyway (the FAT test covers $R/$I files with its own)
        if ($fs -eq "NTFS") {
            New-Item -ItemType Directory -Force -Path "$V\Docs" | Out-Null
            $report = RandomBytes 7000
            [IO.File]::WriteAllBytes("$V\Docs\Report 2014.docx", $report)
            Write-VolumeCache -DriveLetter $part.DriveLetter
            Write-Host "Deleting to the Recycle Bin..."
            $err = [RecycleBin]::Delete("$V\Docs\Report 2014.docx")
            if ($err) { throw "SHFileOperation to the Recycle Bin failed: $err" }
            $recycled = @(Get-ChildItem -Force -Recurse -LiteralPath "$V\`$Recycle.Bin" -Filter '$*' -File -ErrorAction SilentlyContinue)
            if (-not ($recycled | Where-Object { $_.Name -like '$R*' })) { throw "the file did not go to the Recycle Bin" }
            $recycled | ForEach-Object { Remove-Item -Force -LiteralPath $_.FullName }
            Expect "Report 2014.docx" $report "Good" "\Docs" 0 "Report 2014.docx"
        }

        Write-Host "Deleting the test files..."
        foreach ($f in $files.Keys) { Remove-Item -Force -LiteralPath "$V\$f" }
        Remove-Item -Force -Recurse -LiteralPath "$V\Old Stuff"
        [IO.File]::WriteAllLines($tsv, $expected, (New-Object Text.UTF8Encoding $false))
    } finally {
        Dismount-DiskImage -ImagePath $vhd | Out-Null
    }

    # the volume, out of the VHD - the harness reads a volume, not a partitioned disk
    $in = [IO.File]::OpenRead($vhd)
    $out = [IO.File]::Create($img)
    try {
        $in.Position = $part.Offset
        $buf = New-Object byte[] (1MB)
        $left = [long]$part.Size
        while ($left -gt 0) {
            $n = $in.Read($buf, 0, [int][math]::Min($buf.Length, $left))
            if ($n -le 0) { throw "$vhd ended early" }
            $out.Write($buf, 0, $n)
            $left -= $n
        }
    } finally { $in.Close(); $out.Close() }
    Remove-Item -Force $vhd
    Write-Host "image ready: $img"
}

if (-not $UseExistingImages) {
    $admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $admin) { throw "Making the images needs an elevated PowerShell (it attaches virtual disks)" }
    foreach ($i in $Images) { New-TestImage $i[0] $i[1] $i[2] $i[3] }
}

& (Join-Path $Root "tests\ntfs-image\build_harness.ps1")
$Harness = Join-Path $Root "out\ntfs-test\harness-new.exe"

$failed = 0
function Check($what, $got, $want) {
    if ($got -eq $want) { Write-Host "  $what`: $got - OK" }
    else { Write-Host "  $what`: '$got' instead of '$want' - WRONG"; $script:failed++ }
}

foreach ($i in $Images) {
    $img = $i[0]
    Write-Host "${img}:"
    $RecDir = Join-Path $OutDir "recovered-$img"
    if (Test-Path $RecDir) { Remove-Item -Recurse -Force $RecDir }
    New-Item -ItemType Directory -Force -Path $RecDir | Out-Null

    $Log = Join-Path $OutDir "run-$img.log"
    & $Harness (Join-Path $OutDir "$img.img") $RecDir | Out-File -Encoding utf8 $Log
    if ($LASTEXITCODE -ne 0) { throw "harness failed on $img - see $Log" }

    $found = @{}
    $preview = @{}
    foreach ($line in Get-Content -Encoding UTF8 $Log) {
        $f = $line -split "`t"
        if ($f[0] -eq "FILE") { $found[$f[2]] = @{ Recov = $f[3]; Folder = $f[6]; Content = $f[7]; Saved = $f[8]; Error = $f[10] } }
        if ($f[0] -eq "PREVIEW") { $preview[$f[1]] = $f[2] }
    }

    foreach ($line in Get-Content -Encoding UTF8 (Join-Path $OutDir "$img.expected.tsv")) {
        $name, $hash, $recov, $folder, $err, $saved = $line -split "`t"
        $file = $found[$name]
        $short = if ($name.Length -gt 40) { $name.Substring(0, 40) + "..." } else { $name }
        if (-not $file) { Write-Host "  MISSING from the list: $short"; $failed++; continue }
        if ($recov -ne "-") { Check "$short - recoverability" $file.Recov $recov }
        else { Write-Host "  $short - recoverability (not checked): $($file.Recov)" }
        Check "$short - folder" $file.Folder $folder
        Check "$short - error code" $file.Error $err
        if ($hash -ne "-") {
            $path = Join-Path $RecDir $saved
            if (-not (Test-Path -LiteralPath $path)) { Write-Host "  $short - NOT RECOVERED"; $failed++ }
            else { Check "$short - data" ((Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash.ToLower() -eq $hash) $true }
        } else {
            # a refused file leaves nothing behind
            Check "$short - no file left" (Test-Path -LiteralPath (Join-Path $RecDir $name)) $false
        }
    }

    $infoFiles = @($found.Keys | Where-Object { $_ -like '$I*' })
    Check "Recycle Bin info files listed" ($infoFiles -join ", ") ""
    $png = if ($img -like "fat*") { "_review.png" } else { "preview.png" }
    Check "preview of $png" $preview[$png] "120x80"
    Check "contents of $png" $found[$png].Content "Looks valid"
    $fillers = @($found.Keys | Where-Object { $_ -like "fill_*" })
    Write-Host "  deleted filler files listed: $($fillers.Count)"
}

if ($failed) { throw "$failed check(s) failed" }
Write-Host "All images written by Windows: test files found, recovered and judged as expected."
