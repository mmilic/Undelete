# End-to-end check of the FAT recovery engine (FAT12, FAT16, FAT32), no admin rights or WSL needed:
#   1. builds the FAT images with deleted test files (Python + pyfatfs, in a venv under out\fat-test)
#   2. builds the harness (tests\ntfs-image\build_harness.ps1 - one harness for both file systems)
#   3. recovers every deleted file from each image and checks it against <image>.expected.tsv
# Run from anywhere: powershell -ExecutionPolicy Bypass -File tests\fat-image\run.ps1
# -UseExistingImages: skip step 1 and use the images in out\fat-test
param([switch]$UseExistingImages)
$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Resolve-Path (Join-Path $Here "..\..")
$OutDir = Join-Path $Root "out\fat-test"
$Images = "fat12", "fat16", "fat32"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

if (-not $UseExistingImages) {
    $Venv = Join-Path $OutDir "venv"
    $Python = Join-Path $Venv "Scripts\python.exe"
    if (-not (Test-Path $Python)) {
        Write-Host "Setting up Python with pyfatfs in $Venv..."
        & python -m venv $Venv
        if ($LASTEXITCODE -ne 0) { throw "python -m venv failed - is Python 3 installed?" }
        # pyfatfs needs fs, which still imports pkg_resources - gone from setuptools 81 on
        & $Python -m pip install --quiet --disable-pip-version-check "pyfatfs==1.1.0" "setuptools<81"
        if ($LASTEXITCODE -ne 0) { throw "pip install pyfatfs failed" }
    }
    Write-Host "Building the FAT test images..."
    & $Python -W ignore -I (Join-Path $Here "make_image.py") $OutDir
    if ($LASTEXITCODE -ne 0) { throw "make_image.py failed" }
}
foreach ($img in $Images) {
    foreach ($f in "$img.img", "$img.expected.tsv") {
        if (-not (Test-Path (Join-Path $OutDir $f))) { throw "$OutDir\$f is missing" }
    }
}

& (Join-Path $Root "tests\ntfs-image\build_harness.ps1")
$Harness = Join-Path $Root "out\ntfs-test\harness-new.exe"

$failed = 0
function Check($what, $got, $want) {
    if ($got -eq $want) { Write-Host "  $what`: $got - OK" }
    else { Write-Host "  $what`: '$got' instead of '$want' - WRONG"; $script:failed++ }
}

foreach ($img in $Images) {
    Write-Host "$img`:"
    $RecDir = Join-Path $OutDir "recovered-$img"
    if (Test-Path $RecDir) { Remove-Item -Recurse -Force $RecDir }
    New-Item -ItemType Directory -Force -Path $RecDir | Out-Null

    $Log = Join-Path $OutDir "run-$img.log"
    & $Harness (Join-Path $OutDir "$img.img") $RecDir | Out-File -Encoding utf8 $Log
    if ($LASTEXITCODE -ne 0) { throw "harness failed on $img - see $Log" }

    # what the engine reported for each file (from the harness's tab separated lines)
    $found = @{}
    $preview = @{}
    $progress = $null
    $fatType = $null
    foreach ($line in Get-Content -Encoding UTF8 $Log) {
        $f = $line -split "`t"
        if ($f[0] -eq "FILE") { $found[$f[2]] = @{ Recov = $f[3]; Result = $f[5]; Folder = $f[6]; Content = $f[7] } }
        if ($f[0] -eq "PROGRESS") { $progress = $f }
        if ($f[0] -eq "PREVIEW") { $preview[$f[1]] = $f[2] }
        if ($line -like "FAT type: *") { $fatType = $line.Substring(10) }
    }
    Check "FAT type" "FAT$fatType" $img.ToUpper()

    foreach ($line in Get-Content -Encoding UTF8 (Join-Path $OutDir "$img.expected.tsv")) {
        $name, $hash, $recov, $folder, $content = $line -split "`t"
        $file = $found[$name]
        if (-not $file) { Write-Host "  MISSING from the list: $name"; $failed++; continue }
        Check "$name - recoverability" $file.Recov $recov
        Check "$name - folder" $file.Folder $folder
        if ($content -ne "-") { Check "$name - contents" $file.Content $content }
        if ($hash -ne "-") {
            $path = Join-Path $RecDir $name
            if ($file.Result -ne "ok" -or -not (Test-Path -LiteralPath $path)) {
                Write-Host "  $name - NOT RECOVERED"; $failed++
            } else {
                Check "$name - data" ((Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash.ToLower() -eq $hash) $true
            }
        }
    }

    # every $I file gave its $R file the original name back - they are bookkeeping and must not be listed
    $infoFiles = @($found.Keys | Where-Object { $_ -like '$I*' })
    Check "Recycle Bin info files listed" ($infoFiles -join ", ") ""

    # the image preview reads deleted data and decodes it: the real PNG must decode, random bytes named .jpg must not
    Check "preview of preview.png" $preview["preview.png"] "120x80"
    $randomJpg = [string][char]0x0444 + [char]0x043E + [char]0x0442 + [char]0x043E + [char]0x0433 + [char]0x0440 +
        [char]0x0430 + [char]0x0444 + [char]0x0438 + [char]0x0458 + [char]0x0430 + ".jpg" # the Cyrillic file name, in ASCII source
    Check "preview of $randomJpg" $preview[$randomJpg] "no image"

    # the copy progress counter must match what was written (whole KB, so up to 1 KB per file may be missing)
    $kbCounted, $kbExpected, $files = [long]$progress[1], [long]$progress[2], [long]$progress[3]
    Check "progress counter in range" ($kbCounted -le $kbExpected -and $kbExpected - $kbCounted -le $files) $true
}

if ($failed) { throw "$failed check(s) failed" }
Write-Host "All FAT images: test files found, recovered and judged as expected."
