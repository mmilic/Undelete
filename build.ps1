# Builds GetMyFilesBack.exe (UndeleteWizzard) using the toolchain vendored in tools/.
# No installs required beyond what's in this repo. Run from the repo root:
#   powershell -ExecutionPolicy Bypass -File build.ps1
#
# Build is out-of-source: all object files, the compiled resource, and the final
# exe land in out\, leaving src\ untouched. Produces: out\GetMyFilesBack.exe

$ErrorActionPreference = "Stop"
$Root    = Split-Path -Parent $MyInvocation.MyCommand.Path
$MinGW   = Join-Path $Root "tools\MinGW"
$Wx      = Join-Path $Root "tools\wxWidgets-3.0.1_static"
$Src     = Join-Path $Root "src\UndeleteWizzard"
$Out     = Join-Path $Root "out"

$env:PATH = "$MinGW\bin;$env:PATH"
New-Item -ItemType Directory -Force -Path $Out | Out-Null

# IMPORTANT: must be GCC 4.6.2 (the vendored tools\MinGW). The static wxWidgets
# libs in tools\wxWidgets-3.0.1_static were built with 4.6.2; linking the app with
# a different GCC (e.g. 4.8.x) compiles and links with no errors, but the exe
# segfaults inside wx's own pre-main static initializers (libstdc++ ABI mismatch).
$gccVersion = & "$MinGW\bin\g++.exe" -dumpversion
Write-Host "Using g++ $gccVersion from $MinGW\bin"
if ($gccVersion -notlike "4.6.*") {
    Write-Warning "Expected GCC 4.6.2 - got $gccVersion. The build may link but crash at startup (ABI mismatch with the static wxWidgets libs). See README.md."
}

Write-Host "Compiling resource file..."
& windres -i "$Src\undelete.rc" -o "$Out\undelete.res" -O coff --include-dir "$Src"
if ($LASTEXITCODE -ne 0) { throw "windres failed" }

$cppFiles = @(
    "main.cpp", "MyWizzard.cpp", "NTFSDrive.cpp", "MFTRecord.cpp", "MyThreadClass.cpp",
    "CustomButton.cpp", "IsValidFileName.cpp", "Page_GreetingPage.cpp",
    "Page_SelectSourceDrivePage.cpp", "Page_SelectDestinationDirPage.cpp",
    "Page_SelectFilesPage.cpp", "Page_EndPage.cpp",
    "languages\LanguageDialog.cpp", "languages\LanguageFactory.cpp", "languages\LanguagesImpl.cpp"
)

# The FAT/ folder is testdisk/PhotoRec's vendored source (kept in full under src\ as
# reference/for future FAT-support work - see README.md "FAT support" section). This
# build does NOT link testdisk's engine; FAT/phmain.h's photorecz() is stubbed below,
# because the real photorecz() delegates into testdisk's own interactive ncurses UI,
# which was never wired up to this wx GUI and needs a vendored PDCurses build this
# repo doesn't include.
$stubPath = Join-Path $Out "photorecz_stub.c"
@"
/* Build-only stub: the real photorecz() lives in FAT/phmain.c and delegates into
 * testdisk's interactive ncurses UI (do_curses_photorec in FAT/pdisksel.c), which
 * isn't wired up to this wx GUI and needs PDCurses, not vendored in this repo.
 * This lets the NTFS recovery path (the actual finished feature) build and link. */
int photorecz(char drive_letter, const char *dest_path) {
    (void)drive_letter;
    (void)dest_path;
    return -1;
}
"@ | Set-Content -Path $stubPath -Encoding ASCII

$includeArgs = @(
    "-I", "$MinGW\include",
    "-I", "$Wx\lib\wx\include\msw-unicode-static-3.0",
    "-I", "$Wx\include"
)
$defineArgs = @(
    "-D", "UNICODE",
    "-D_LARGEFILE_SOURCE=unknown", "-DwxDEBUG_LEVEL=0", "-D__WXMSW__", "-mthreads"
)

Write-Host "Compiling C++ sources..."
$objFiles = @()
foreach ($f in $cppFiles) {
    $srcFile = Join-Path $Src $f
    $obj = Join-Path $Out ([System.IO.Path]::GetFileNameWithoutExtension($f) + ".o")
    $gppArgs = @("-c", $srcFile) + $defineArgs + $includeArgs + @("-o", $obj)
    & g++ @gppArgs
    if ($LASTEXITCODE -ne 0) { throw "g++ failed compiling $f" }
    $objFiles += $obj
}

Write-Host "Compiling photorecz stub..."
$stubObj = Join-Path $Out "photorecz_stub.o"
& gcc -c $stubPath -o $stubObj
if ($LASTEXITCODE -ne 0) { throw "gcc failed compiling stub" }
$objFiles += $stubObj

Write-Host "Linking GetMyFilesBack.exe..."
$wxLibNames = @(
    "libwx_mswu_xrc-3.0.a","libwx_mswu_webview-3.0.a","libwx_mswu_qa-3.0.a",
    "libwx_baseu_net-3.0.a","libwx_mswu_html-3.0.a","libwx_mswu_adv-3.0.a",
    "libwx_mswu_core-3.0.a","libwx_baseu_xml-3.0.a","libwx_baseu-3.0.a"
)
$wxLibs = $wxLibNames | ForEach-Object { Join-Path "$Wx\lib" $_ }

$exePath = Join-Path $Out "GetMyFilesBack.exe"
$resPath = Join-Path $Out "undelete.res"

$linkArgs = @()
$linkArgs += $objFiles
$linkArgs += $resPath
$linkArgs += @("-mthreads", "-L", "$Wx\lib")
$linkArgs += @("-Wl,--subsystem,windows", "-mwindows")
$linkArgs += $wxLibs
$linkArgs += @("-static", "-lwxregexu-3.0", "-lwxexpat-3.0", "-lwxtiff-3.0", "-lwxjpeg-3.0", "-lwxpng-3.0", "-lwxzlib-3.0")
$linkArgs += @("-lrpcrt4", "-loleaut32", "-lole32", "-luuid", "-lwinspool", "-lwinmm", "-lshell32", "-lcomctl32", "-lcomdlg32", "-ladvapi32", "-lwsock32", "-lgdi32")
$linkArgs += @("-o", $exePath)

& g++ @linkArgs
if ($LASTEXITCODE -ne 0) { throw "link failed" }

Write-Host "Built: $exePath"
Write-Host "Note: the embedded manifest requests elevation (highestAvailable) - launch it from an elevated shell/Explorer, and expect a UAC prompt, since it needs raw NTFS volume access."
