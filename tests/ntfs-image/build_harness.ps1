# Builds out\ntfs-test\harness-new.exe (engine from src\) and, with -WithOld, harness-old.exe
# (engine and harness from git HEAD, to compare before/after a change). Same toolchain as build.ps1.
param([switch]$WithOld)

$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = (Resolve-Path (Join-Path $Here "..\..")).Path
$OutDir = Join-Path $Root "out\ntfs-test"
. (Join-Path $Root "build-env.ps1") # sets $Cmake and the vcpkg variables
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

Push-Location $Root
try {
    & $Cmake --preset x64
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
    & $Cmake --build --preset release --target ntfs_harness
    if ($LASTEXITCODE -ne 0) { throw "build of the harness failed" }
    Copy-Item "out\build\x64\bin\Release\ntfs_harness.exe" (Join-Path $OutDir "harness-new.exe") -Force
    Write-Host "Built $(Join-Path $OutDir 'harness-new.exe')"

    if ($WithOld) {
        # old engine sources and the matching harness straight from the last commit (the engine API may have changed)
        $Old = Join-Path $OutDir "old-src"
        New-Item -ItemType Directory -Force -Path $Old | Out-Null
        $files = @{"NTFSDrive.cpp" = "src/UndeleteWizzard"; "NTFSDrive.h" = "src/UndeleteWizzard"; "MFTRecord.cpp" = "src/UndeleteWizzard";
            "MFTRecord.h" = "src/UndeleteWizzard"; "FileContents.cpp" = "src/UndeleteWizzard"; "FileContents.h" = "src/UndeleteWizzard";
            "harness.cpp" = "tests/ntfs-image"}
        foreach ($f in $files.Keys) {
            # cmd redirection keeps the bytes as they are (a PowerShell 5.1 pipe would re-encode them)
            cmd /c "git -C `"$Root`" show HEAD:$($files[$f])/$f > `"$(Join-Path $Old $f)`" 2>nul"
            if ($LASTEXITCODE -ne 0) {
                Remove-Item -Force (Join-Path $Old $f)
                if ($f -notlike "FileContents.*") { throw "git show HEAD:$($files[$f])/$f failed" }
            }
        }
        # a second build tree with only the harness, on the old engine (the vcpkg packages are shared)
        $OldBuild = Join-Path $Root "out\build\x64-old"
        & $Cmake -S $Root -B $OldBuild -G "Visual Studio 17 2022" -A x64 `
            "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=x64-windows-static" `
            "-DVCPKG_INSTALLED_DIR=$Root\out\vcpkg_installed" "-DUNDELETE_BUILD_APP=OFF" `
            "-DUNDELETE_ENGINE_DIR=$Old" "-DUNDELETE_HARNESS_SOURCE=$Old\harness.cpp"
        if ($LASTEXITCODE -ne 0) { throw "CMake configure of the old engine failed" }
        & $Cmake --build $OldBuild --config Release --target ntfs_harness
        if ($LASTEXITCODE -ne 0) { throw "build of the old harness failed" }
        Copy-Item (Join-Path $OldBuild "bin\Release\ntfs_harness.exe") (Join-Path $OutDir "harness-old.exe") -Force
        Write-Host "Built $(Join-Path $OutDir 'harness-old.exe')"
    }
} finally {
    Pop-Location
}
