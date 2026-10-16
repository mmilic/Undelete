# Builds GetMyFilesBack.exe (UndeleteWizzard): 64-bit, MSVC, static wxWidgets from vcpkg.
# Needs Visual Studio 2022 (or its Build Tools) with "Desktop development with C++" - it brings MSVC,
# CMake and vcpkg. Run from the repo root:
#   powershell -ExecutionPolicy Bypass -File build.ps1            (Release)
#   powershell -ExecutionPolicy Bypass -File build.ps1 -Debug
#
# Everything generated goes to out\: the CMake build (out\build\x64), the vcpkg packages (out\vcpkg_installed,
# built once - the first build takes a while) and its download/binary caches. Produces out\GetMyFilesBack.exe.
param([switch]$Debug)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
. (Join-Path $Root "build-env.ps1") # sets $Cmake and the vcpkg variables

$config = if ($Debug) { "Debug" } else { "Release" }
Push-Location $Root
try {
    & $Cmake --preset x64
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
    & $Cmake --build --preset $config.ToLower() --target GetMyFilesBack
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
} finally {
    Pop-Location
}

$exe = Join-Path $Root "out\GetMyFilesBack.exe"
Copy-Item (Join-Path $Root "out\build\x64\bin\$config\GetMyFilesBack.exe") $exe -Force
Write-Host "Built: $exe ($config, x64)"
Write-Host "Note: the embedded manifest requires administrator rights (raw NTFS volume access) - launch it from Explorer or an elevated shell, and expect a UAC prompt."
