# End-to-end check of the NTFS recovery engine, no admin rights needed:
#   1. builds NTFS disk images with deleted test files (WSL + ntfs-3g) - the same files on three formats:
#      the usual one, 4096-byte sectors and 256 KB clusters (make_image.sh)
#   2. builds the harness, which runs CNTFSDrive/CMFTRecord against an image
#   3. recovers every deleted file from each image and compares the test files with their original checksums
# Run from anywhere: powershell -ExecutionPolicy Bypass -File tests\ntfs-image\run.ps1
# -UseExistingImage: skip step 1 and use the images and their .sha256 files in out\ntfs-test, made elsewhere
# (the GitHub Actions build makes them on Linux - its Windows machines have no WSL)
param([switch]$UseExistingImage)
$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Resolve-Path (Join-Path $Here "..\..")
$OutDir = Join-Path $Root "out\ntfs-test"
$Images = "test", "4k-sectors", "256k-clusters"

if (-not $UseExistingImage) {
    Write-Host "Building test images (WSL, as root)..."
    $wslScript = (& wsl -e wslpath -a ($Here -replace '\\', '/')) + "/make_image.sh"
    & wsl -u root -e sh $wslScript
    if ($LASTEXITCODE -ne 0) { throw "make_image.sh failed - is ntfs-3g installed in WSL?" }
}
foreach ($img in $Images) {
    foreach ($f in "$img.img", "$img.sha256") {
        if (-not (Test-Path (Join-Path $OutDir $f))) { throw "$OutDir\$f is missing" }
    }
}

& (Join-Path $Here "build_harness.ps1")

$failed = 0
foreach ($img in $Images) {
    Write-Host "${img}:"
    $RecDir = Join-Path $OutDir "recovered-$img"
    # rd with \\?\ - Remove-Item of Windows PowerShell 5.1 can't delete the long-named file inside
    if (Test-Path $RecDir) { cmd /c "rd /s /q `"\\?\$RecDir`"" }
    New-Item -ItemType Directory -Force -Path $RecDir | Out-Null

    $Log = Join-Path $OutDir "run-$img.log"
    & (Join-Path $OutDir "harness-new.exe") (Join-Path $OutDir "$img.img") $RecDir | Out-File -Encoding utf8 $Log
    if ($LASTEXITCODE -ne 0) { throw "harness failed on $img - see $Log" }

    # recoverability the engine reported for each file (from the harness's tab separated FILE lines)
    $recoverability = @{}
    $folder = @{}
    $content = @{}
    $preview = @{}
    $progress = $null
    foreach ($line in Get-Content -Encoding UTF8 $Log) {
        $f = $line -split "`t"
        if ($f[0] -eq "FILE") { $recoverability[$f[2]] = $f[3]; $folder[$f[2]] = $f[6]; $content[$f[2]] = $f[7] }
        if ($f[0] -eq "PROGRESS") { $progress = $f }
        if ($f[0] -eq "PREVIEW") { $preview[$f[1]] = $f[2] }
    }

    # the image preview reads deleted data and decodes it: the real PNG must decode, random bytes named .jpg must not
    foreach ($check in @(@("preview.png", "120x80"), @("фотографија.jpg", "no image"))) {
        $got = $preview[$check[0]]
        if ($got -eq $check[1]) { Write-Host "  Preview of $($check[0]): $got - OK" }
        else { Write-Host "  Preview of $($check[0]): '$got' instead of '$($check[1])' - WRONG"; $failed++ }
    }

    # the folder each file was deleted from - from the MFT parent references (also through a deleted folder),
    # and for Recycle Bin files from the original path in their $I file
    foreach ($check in @(@("fragmented.bin", "\"), @("holiday.png", "\Photos\2014"), @("notes.txt", "\Old Stuff"),
            @("Летовање 2014.jpg", "\Users\test\Pictures"), @("Izveštaj.docx", "\Users\test\Documents"))) {
        $got = $folder[$check[0]]
        if ($got -eq $check[1]) { Write-Host "  Folder of $($check[0]): $got - OK" }
        else { Write-Host "  Folder of $($check[0]): '$got' instead of '$($check[1])' - WRONG"; $failed++ }
    }

    # every $I file gave its $R file the original name back - they are bookkeeping and must not be listed
    $infoFiles = @($recoverability.Keys | Where-Object { $_ -like '$I*' })
    if ($infoFiles.Count) { Write-Host "  Recycle Bin info files listed: $($infoFiles -join ', ') - WRONG"; $failed++ }
    else { Write-Host "  Recycle Bin info files not listed - OK" }

    # the content check: real files of a known type look valid, random bytes under such a name don't
    foreach ($check in @(@("preview.png", "Looks valid"), @("holiday.png", "Looks valid"), @("notes.txt", "Looks valid"),
            @("letter.pdf", "Looks valid"), @("фотографија.jpg", "Damaged"), @("Čačak ćevapi.txt", "Damaged"),
            @("Izveštaj.docx", "Damaged"), @("fragmented.bin", "Can't check"))) {
        $got = $content[$check[0]]
        if ($got -eq $check[1]) { Write-Host "  Contents of $($check[0]): $got - OK" }
        else { Write-Host "  Contents of $($check[0]): '$got' instead of '$($check[1])' - WRONG"; $failed++ }
    }

    # the copy progress counter must match what was written (whole KB, so up to 1 KB per file may be missing)
    $kbCounted, $kbExpected, $files = [long]$progress[1], [long]$progress[2], [long]$progress[3]
    if ($kbCounted -le $kbExpected -and $kbExpected - $kbCounted -le $files) {
        Write-Host "  Progress counter: $kbCounted KB counted for $kbExpected KB recovered ($files files) - OK"
    } else {
        Write-Host "  Progress counter: $kbCounted KB counted for $kbExpected KB recovered ($files files) - WRONG"; $failed++
    }
    foreach ($line in Get-Content -Encoding UTF8 (Join-Path $OutDir "$img.sha256")) {
        $hash, $name = $line -split '\s+', 2
        # \\?\ - the 199-character name makes a path over 260 characters when the clone isn't near the drive's root
        $file = "\\?\" + (Join-Path $RecDir $name)
        $shortName = if ($name.Length -gt 40) { $name.Substring(0, 40) + "..." } else { $name }
        if (-not (Test-Path -LiteralPath $file)) {
            Write-Host "  MISSING  $shortName"; $failed++; continue
        }
        $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $file).Hash.ToLower()
        # nothing was written to the volume after these files were deleted, so their clusters must all be free
        $recov = $recoverability[$name]
        if ($actual -ne $hash) { Write-Host "  MISMATCH $shortName"; $failed++ }
        elseif ($recov -ne "Good") { Write-Host "  OK, but recoverability '$recov' instead of 'Good': $shortName"; $failed++ }
        else { Write-Host "  OK       $shortName  (Good)" }
    }

    # the filler files freed before the test files were written - their clusters were reused by the test files
    $fillers = $recoverability.Keys | Where-Object { $_ -like "fill_*" }
    $overwritten = @($fillers | Where-Object { $recoverability[$_] -ne "Good" }).Count
    Write-Host "  Deleted filler files whose clusters were reused: $overwritten of $(@($fillers).Count) flagged as not Good"
    if ($overwritten -eq 0) { Write-Host "  Expected some overwritten filler files"; $failed++ }
}

if ($failed) { throw "$failed check(s) failed" }
Write-Host "All NTFS images: test files recovered byte-for-byte and flagged Good; reused files flagged."
