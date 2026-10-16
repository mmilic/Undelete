# Dot-sourced by build.ps1 and tests\ntfs-image\build_harness.ps1: finds Visual Studio's CMake and vcpkg
# and sets $Cmake. Needs Visual Studio 2022 (or its Build Tools) with "Desktop development with C++".

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) { throw "Visual Studio 2022 (or Build Tools) is not installed - see README.md." }
$VsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $VsPath) { throw "No Visual Studio installation with the C++ x64 compiler ('Desktop development with C++') - see README.md." }

$Cmake = Join-Path $VsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if (-not (Test-Path $Cmake)) { $Cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source }
if (-not $Cmake) { throw "CMake not found - install the 'C++ CMake tools for Windows' component of Visual Studio." }

# vcpkg: a separate checkout if VCPKG_ROOT is set, otherwise the one that comes with Visual Studio
if (-not $env:VCPKG_ROOT) { $env:VCPKG_ROOT = Join-Path $VsPath "VC\vcpkg" }
if (-not (Test-Path (Join-Path $env:VCPKG_ROOT "scripts\buildsystems\vcpkg.cmake"))) {
    throw "vcpkg not found at $env:VCPKG_ROOT - install the 'vcpkg package manager' component of Visual Studio, or set VCPKG_ROOT."
}

# keep vcpkg's downloads and prebuilt packages in out\ too, not in the user profile
$BuildOut = Join-Path $PSScriptRoot "out"
foreach ($v in @(@("VCPKG_DOWNLOADS", "vcpkg-downloads"), @("VCPKG_DEFAULT_BINARY_CACHE", "vcpkg-cache"))) {
    $dir = Join-Path $BuildOut $v[1]
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    Set-Item "env:$($v[0])" $dir
}
