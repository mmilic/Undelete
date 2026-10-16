# End-to-end check of the NTFS recovery engine, no admin rights needed:
#   1. builds an NTFS disk image with deleted test files (WSL + ntfs-3g)
#   2. builds the harness, which runs CNTFSDrive/CMFTRecord against the image
#   3. recovers every deleted file and compares the test files with their original checksums
# Run from anywhere: powershell -ExecutionPolicy Bypass -File tests\ntfs-image\run.ps1
$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Resolve-Path (Join-Path $Here "..\..")
$OutDir = Join-Path $Root "out\ntfs-test"

Write-Host "Building test image (WSL, as root)..."
$wslScript = (& wsl -e wslpath -a ($Here -replace '\\', '/')) + "/make_image.sh"
& wsl -u root -e sh $wslScript
if ($LASTEXITCODE -ne 0) { throw "make_image.sh failed - is ntfs-3g installed in WSL?" }

& (Join-Path $Here "build_harness.ps1")

$RecDir = Join-Path $OutDir "recovered"
if (Test-Path $RecDir) { Remove-Item -Recurse -Force $RecDir }
New-Item -ItemType Directory -Force -Path $RecDir | Out-Null

Write-Host "Recovering deleted files from the image..."
& (Join-Path $OutDir "harness-new.exe") (Join-Path $OutDir "test.img") $RecDir | Out-File -Encoding utf8 (Join-Path $OutDir "run.log")
if ($LASTEXITCODE -ne 0) { throw "harness failed - see $OutDir\run.log" }

# recoverability the engine reported for each file (from the harness's tab separated FILE lines)
$recoverability = @{}
$progress = $null
foreach ($line in Get-Content -Encoding UTF8 (Join-Path $OutDir "run.log")) {
    $f = $line -split "`t"
    if ($f[0] -eq "FILE") { $recoverability[$f[2]] = $f[3] }
    if ($f[0] -eq "PROGRESS") { $progress = $f }
}

$failed = 0

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
