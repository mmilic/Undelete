# Builds out\ntfs-test\harness-new.exe (engine from src\) and, with -WithOld, harness-old.exe
# (engine from git HEAD, to compare before/after a change).
param([switch]$WithOld)

$ErrorActionPreference = "Stop"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Resolve-Path (Join-Path $Here "..\..")
$OutDir = Join-Path $Root "out\ntfs-test"
$MinGW = Join-Path $Root "tools\MinGW"
$Wx = Join-Path $Root "tools\wxWidgets-3.0.1_static"
$env:PATH = "$MinGW\bin;$env:PATH"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$variants = @(@{Name = "new"; Src = (Join-Path $Root "src\UndeleteWizzard"); Harness = (Join-Path $Here "harness.cpp")})

if ($WithOld) {
    # old engine sources and the matching harness straight from the last commit (the engine API may have changed)
    $Old = Join-Path $OutDir "old-src"
    New-Item -ItemType Directory -Force -Path $Old | Out-Null
    $files = @{"NTFSDrive.cpp" = "src/UndeleteWizzard"; "NTFSDrive.h" = "src/UndeleteWizzard"; "MFTRecord.cpp" = "src/UndeleteWizzard";
        "MFTRecord.h" = "src/UndeleteWizzard"; "harness.cpp" = "tests/ntfs-image"}
    foreach ($f in $files.Keys) {
        # cmd redirection keeps the bytes as they are (a PowerShell 5.1 pipe would re-encode them)
        cmd /c "git -C `"$Root`" show HEAD:$($files[$f])/$f > `"$(Join-Path $Old $f)`""
        if ($LASTEXITCODE -ne 0) { throw "git show HEAD:$($files[$f])/$f failed" }
    }
    $variants += @{Name = "old"; Src = $Old; Harness = (Join-Path $Old "harness.cpp")}
}

$common = @("-D", "UNICODE", "-DwxDEBUG_LEVEL=0", "-D__WXMSW__", "-mthreads",
    "-I", "$MinGW\include", "-I", "$Wx\lib\wx\include\msw-unicode-static-3.0", "-I", "$Wx\include")
$libs = @("-mthreads", "-L", "$Wx\lib", "-static",
    "-lwx_mswu_core-3.0", "-lwx_baseu-3.0", "-lwxregexu-3.0", "-lwxexpat-3.0", "-lwxtiff-3.0", "-lwxjpeg-3.0", "-lwxpng-3.0", "-lwxzlib-3.0",
    "-lrpcrt4", "-loleaut32", "-lole32", "-luuid", "-lwinspool", "-lwinmm", "-lshell32", "-lcomctl32", "-lcomdlg32", "-ladvapi32", "-lwsock32", "-lgdi32")

foreach ($variant in $variants) {
    $exe = Join-Path $OutDir ("harness-" + $variant.Name + ".exe")
    $sources = @($variant.Harness,(Join-Path $variant.Src "NTFSDrive.cpp"), (Join-Path $variant.Src "MFTRecord.cpp"))
    & g++ @sources @common "-I" $variant.Src @libs -o $exe
    if ($LASTEXITCODE -ne 0) { throw "build of $exe failed" }
    Write-Host "Built $exe"
}
