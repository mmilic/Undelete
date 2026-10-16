# End-to-end check of the NTFS recovery engine, no admin rights needed:
#   1. builds an NTFS disk image with deleted test files (WSL + ntfs-3g)
#   2. builds the harness, which runs CNTFSDrive/CMFTRecord against the image
#   3. recovers every deleted file and compares the test files with their original checksums
# Run from anywhere: powershell -ExecutionPolicy Bypass -File tests\ntfs-image\run.ps1
# -UseExistingImage: skip step 1 and use out\ntfs-test\test.img + expected.sha256 made elsewhere
# (the GitHub Actions build makes them on Linux - its Windows machines have no WSL)
param([switch]$UseExistingImage)
$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Resolve-Path (Join-Path $Here "..\..")
$OutDir = Join-Path $Root "out\ntfs-test"

if ($UseExistingImage) {
    foreach ($f in "test.img", "expected.sha256") {
        if (-not (Test-Path (Join-Path $OutDir $f))) { throw "-UseExistingImage: $OutDir\$f is missing" }
    }
} else {
    Write-Host "Building test image (WSL, as root)..."
    $wslScript = (& wsl -e wslpath -a ($Here -replace '\\', '/')) + "/make_image.sh"
    & wsl -u root -e sh $wslScript
    if ($LASTEXITCODE -ne 0) { throw "make_image.sh failed - is ntfs-3g installed in WSL?" }
}

& (Join-Path $Here "build_harness.ps1")

$RecDir = Join-Path $OutDir "recovered"
if (Test-Path $RecDir) { Remove-Item -Recurse -Force $RecDir }
New-Item -ItemType Directory -Force -Path $RecDir | Out-Null

Write-Host "Recovering deleted files from the image..."
& (Join-Path $OutDir "harness-new.exe") (Join-Path $OutDir "test.img") $RecDir | Out-File -Encoding utf8 (Join-Path $OutDir "run.log")
if ($LASTEXITCODE -ne 0) { throw "harness failed - see $OutDir\run.log" }

# recoverability the engine reported for each file (from the harness's tab separated FILE lines)
$recoverability = @{}
$folder = @{}
$content = @{}
$preview = @{}
$progress = $null
foreach ($line in Get-Content -Encoding UTF8 (Join-Path $OutDir "run.log")) {
    $f = $line -split "`t"
    if ($f[0] -eq "FILE") { $recoverability[$f[2]] = $f[3]; $folder[$f[2]] = $f[6]; $content[$f[2]] = $f[7] }
    if ($f[0] -eq "PROGRESS") { $progress = $f }
    if ($f[0] -eq "PREVIEW") { $preview[$f[1]] = $f[2] }
}

$failed = 0

# the image preview reads deleted data and decodes it: the real PNG must decode, random bytes named .jpg must not
foreach ($check in @(@("preview.png", "120x80"), @("фотографија.jpg", "no image"))) {
    $got = $preview[$check[0]]
    if ($got -eq $check[1]) { Write-Host "Preview of $($check[0]): $got - OK" }
    else { Write-Host "Preview of $($check[0]): '$got' instead of '$($check[1])' - WRONG"; $failed++ }
}

# the folder each file was deleted from - from the MFT parent references (also through a deleted folder),
# and for Recycle Bin files from the original path in their $I file
foreach ($check in @(@("fragmented.bin", "\"), @("holiday.png", "\Photos\2014"), @("notes.txt", "\Old Stuff"),
        @("Летовање 2014.jpg", "\Users\test\Pictures"), @("Izveštaj.docx", "\Users\test\Documents"))) {
    $got = $folder[$check[0]]
    if ($got -eq $check[1]) { Write-Host "Folder of $($check[0]): $got - OK" }
    else { Write-Host "Folder of $($check[0]): '$got' instead of '$($check[1])' - WRONG"; $failed++ }
}

# every $I file gave its $R file the original name back - they are bookkeeping and must not be listed
$infoFiles = @($recoverability.Keys | Where-Object { $_ -like '$I*' })
if ($infoFiles.Count) { Write-Host "Recycle Bin info files listed: $($infoFiles -join ', ') - WRONG"; $failed++ }
else { Write-Host "Recycle Bin info files not listed - OK" }

# the content check: real files of a known type look valid, random bytes under such a name don't
foreach ($check in @(@("preview.png", "Looks valid"), @("holiday.png", "Looks valid"), @("notes.txt", "Looks valid"),
        @("letter.pdf", "Looks valid"), @("фотографија.jpg", "Damaged"), @("Čačak ćevapi.txt", "Damaged"),
        @("Izveštaj.docx", "Damaged"), @("fragmented.bin", "Can't check"))) {
    $got = $content[$check[0]]
    if ($got -eq $check[1]) { Write-Host "Contents of $($check[0]): $got - OK" }
    else { Write-Host "Contents of $($check[0]): '$got' instead of '$($check[1])' - WRONG"; $failed++ }
}

# the copy progress counter must match what was written (whole KB, so up to 1 KB per file may be missing)
$kbCounted, $kbExpected, $files = [long]$progress[1], [long]$progress[2], [long]$progress[3]
if ($kbCounted -le $kbExpected -and $kbExpected - $kbCounted -le $files) {
    Write-Host "Progress counter: $kbCounted KB counted for $kbExpected KB recovered ($files files) - OK"
} else {
    Write-Host "Progress counter: $kbCounted KB counted for $kbExpected KB recovered ($files files) - WRONG"; $failed++
}
foreach ($line in Get-Content -Encoding UTF8 (Join-Path $OutDir "expected.sha256")) {
    $hash, $name = $line -split '\s+', 2
    $file = Join-Path $RecDir $name
    $shortName = if ($name.Length -gt 40) { $name.Substring(0, 40) + "..." } else { $name }
    if (-not (Test-Path -LiteralPath $file)) {
        Write-Host "MISSING  $shortName"; $failed++; continue
    }
    $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $file).Hash.ToLower()
    # nothing was written to the volume after these files were deleted, so their clusters must all be free
    $recov = $recoverability[$name]
    if ($actual -ne $hash) { Write-Host "MISMATCH $shortName"; $failed++ }
    elseif ($recov -ne "Good") { Write-Host "OK, but recoverability '$recov' instead of 'Good': $shortName"; $failed++ }
    else { Write-Host "OK       $shortName  (Good)" }
}

# the 64 KB filler files freed before the test files were written - their clusters were reused by the test files
$fillers = $recoverability.Keys | Where-Object { $_ -like "fill_*" }
$overwritten = @($fillers | Where-Object { $recoverability[$_] -ne "Good" }).Count
Write-Host "Deleted filler files whose clusters were reused: $overwritten of $(@($fillers).Count) flagged as not Good"
if ($overwritten -eq 0) { Write-Host "Expected some overwritten filler files"; $failed++ }

if ($failed) { throw "$failed check(s) failed" }
Write-Host "All test files recovered byte-for-byte and flagged Good; reused files flagged."
