# End-to-end check of the exFAT recovery engine, no admin rights needed:
#   1. builds the exFAT image with deleted test files (WSL as root + exfatprogs - make_image.sh)
#   2. builds the harness (tests\ntfs-image\build_harness.ps1 - one harness for all the file systems)
#   3. recovers every deleted file and checks it against exfat.expected.tsv
# Run from anywhere: powershell -ExecutionPolicy Bypass -File tests\exfat-image\run.ps1
# -UseExistingImage: skip step 1 and use out\exfat-test\exfat.img (the GitHub Actions build makes it on Linux)
param([switch]$UseExistingImage)
$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Resolve-Path (Join-Path $Here "..\..")
$OutDir = Join-Path $Root "out\exfat-test"
$Img = Join-Path $OutDir "exfat.img"
$Tsv = Join-Path $OutDir "exfat.expected.tsv"

if (-not $UseExistingImage) {
    Write-Host "Building the exFAT test image (WSL, as root)..."
    $wslScript = (& wsl -e wslpath -a ($Here -replace '\\', '/')) + "/make_image.sh"
    & wsl -u root -e sh $wslScript
    if ($LASTEXITCODE -ne 0) { throw "make_image.sh failed - is exfatprogs installed in WSL (sudo apt-get install exfatprogs)?" }
}
foreach ($f in $Img, $Tsv) { if (-not (Test-Path $f)) { throw "$f is missing" } }

& (Join-Path $Root "tests\ntfs-image\build_harness.ps1")
$Harness = Join-Path $Root "out\ntfs-test\harness-new.exe"

$failed = 0
function Check($what, $got, $want) {
    if ($got -eq $want) { Write-Host "  $what`: $got - OK" }
    else { Write-Host "  $what`: '$got' instead of '$want' - WRONG"; $script:failed++ }
}

# a long folder: with the test's 200-character file name the path is over 260 characters - only the \\?\ form reaches it
$RecDir = Join-Path $OutDir ("recovered-" + ("long folder " * 6).Trim().Replace(" ", "-"))
if (Test-Path $RecDir) { cmd /c "rd /s /q `"\\?\$RecDir`"" }
New-Item -ItemType Directory -Force -Path $RecDir | Out-Null

$Log = Join-Path $OutDir "run-exfat.log"
& $Harness $Img $RecDir | Out-File -Encoding utf8 $Log
if ($LASTEXITCODE -ne 0) { throw "harness failed - see $Log" }

$found = @{}
$preview = @{}
$fragments = @{}
$progress = $null
$fileSystem = $null
foreach ($line in Get-Content -Encoding UTF8 $Log) {
    $f = $line -split "`t"
    if ($f[0] -eq "FILE") { $found[$f[2]] = @{ Recov = $f[3]; Result = $f[5]; Folder = $f[6]; Content = $f[7]; Saved = $f[8]; Modified = [long]$f[9] } }
    if ($f[0] -eq "PROGRESS") { $progress = $f }
    if ($f[0] -eq "PREVIEW") { $preview[$f[1]] = $f[2] }
    if ($line -like "File system: *") { $fileSystem = $line.Substring(13) }
    if ($line -match "^Extracting file '.*\\([^\\]+)' \(\d+ bytes in (\d+) fragments\)") { $fragments[$Matches[1]] = [int]$Matches[2] }
}
Check "file system" $fileSystem "exFAT"

foreach ($line in Get-Content -Encoding UTF8 $Tsv) {
    $name, $hash, $recov, $folder, $content, $saved, $mtime = $line -split "`t"
    $file = $found[$name]
    $short = if ($name.Length -gt 40) { $name.Substring(0, 40) + "..." } else { $name }
    if (-not $file) { Write-Host "  MISSING from the list: $short"; $failed++; continue }
    Check "$short - recoverability" $file.Recov $recov
    Check "$short - folder" $file.Folder $folder
    if ($content -ne "-") { Check "$short - contents" $file.Content $content }
    Check "$short - saved as" $file.Saved $saved
    # the engine's modification time is the one Linux wrote (exFAT keeps 10 ms, stat gives whole seconds)
    $want = ([long]$mtime + 11644473600) * 10000000
    Check "$short - modified time within a second" ([math]::Abs($file.Modified - $want) -lt 10000000) $true
    $path = "\\?\" + (Join-Path $RecDir $saved)
    if ($file.Result -ne "ok" -or -not (Test-Path -LiteralPath $path)) { Write-Host "  $short - NOT RECOVERED"; $failed++; continue }
    Check "$short - data" ((Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash.ToLower() -eq $hash) $true
    Check "$short - date" ((Get-Item -LiteralPath $path).LastWriteTimeUtc.ToFileTimeUtc() -eq $file.Modified) $true
}

# the fragmented file really is: its chain came from the FAT
Check "fragmented.bin in more than one fragment" ($fragments["fragmented.bin"] -gt 1) $true
$infoFiles = @($found.Keys | Where-Object { $_ -like '$I*' })
Check "Recycle Bin info files listed" ($infoFiles -join ", ") ""
Check "preview of preview.png" $preview["preview.png"] "120x80"
$fillers = @($found.Keys | Where-Object { $_ -like "fill_*" })
$overwritten = @($fillers | Where-Object { $found[$_].Recov -ne "Good" }).Count
Write-Host "  deleted filler files whose clusters were reused: $overwritten of $($fillers.Count) flagged as not Good"
Check "some fillers flagged as overwritten" ($overwritten -gt 0) $true
$kbCounted, $kbExpected, $files = [long]$progress[1], [long]$progress[2], [long]$progress[3]
Check "progress counter in range" ($kbCounted -le $kbExpected -and $kbExpected - $kbCounted -le $files) $true

if ($failed) { throw "$failed check(s) failed" }
Write-Host "exFAT: all test files found, recovered byte-for-byte and judged as expected."
