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

$failed = 0
foreach ($line in Get-Content -Encoding UTF8 (Join-Path $OutDir "expected.sha256")) {
    $hash, $name = $line -split '\s+', 2
    $file = Join-Path $RecDir $name
    $shortName = if ($name.Length -gt 40) { $name.Substring(0, 40) + "..." } else { $name }
    if (-not (Test-Path -LiteralPath $file)) {
        Write-Host "MISSING  $shortName"; $failed++; continue
    }
    $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $file).Hash.ToLower()
    if ($actual -eq $hash) { Write-Host "OK       $shortName" } else { Write-Host "MISMATCH $shortName"; $failed++ }
}

if ($failed) { throw "$failed test file(s) not recovered correctly" }
Write-Host "All test files recovered byte-for-byte."
