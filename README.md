# GetMyFilesBack (UndeleteWizzard)

A Windows GUI file-undelete/recovery tool (wxWidgets wizard) that scans a drive's NTFS MFT for
deleted-but-recoverable files and copies them to a destination you choose.

It builds as one 64-bit exe with everything linked in statically - no DLLs, no installer. That is
on purpose: a recovery tool should run from a USB stick, not be installed onto the drive whose
deleted files you want back.

## Build

### What the machine needs

| What | Why | Where to get it |
|---|---|---|
| Windows 10 or 11, 64-bit | the app and its compiler are Windows-only | - |
| **Visual Studio 2022** (Community is free) **or Build Tools for Visual Studio 2022** (compiler only, no IDE, ~6-8 GB) | MSVC x64 compiler, Windows SDK, CMake and vcpkg | <https://visualstudio.microsoft.com/downloads/> |
| ...with the workload **"Desktop development with C++"** | its default components include all four tools above - keep "C++ CMake tools for Windows" and "vcpkg package manager" ticked | Visual Studio Installer |
| Git | to clone the repo; vcpkg uses it too | <https://git-scm.com/download/win> (Visual Studio also brings one) |
| Internet access on the first build | vcpkg downloads wxWidgets and its libraries from GitHub | - |
| ~5 GB free disk space in the clone | the wxWidgets build and its caches in `out\` | - |

That's all - no MinGW, no separately installed CMake, no wxWidgets download, no admin rights (except to
run the finished app). Windows PowerShell 5.1, which every Windows 10/11 has, runs the build scripts.

Things to know:
- **Visual Studio 2022 specifically.** `CMakePresets.json` uses the "Visual Studio 17 2022" generator; a machine
  with only a newer Visual Studio needs that line changed.
- **Keep the clone path short**, e.g. `C:\src\Undelete`. vcpkg's build paths inside `out\` are long, and a
  deeply nested clone can hit Windows' 260-character path limit while building wxWidgets.
- **The tests need more**: WSL with `ntfs-3g` and `python3` - see "Tests" below. Building the app doesn't.

**Without installing anything:** every push to `master` is built on GitHub's own Windows machines
(`.github/workflows/build.yml`). Open the repo's **Actions** tab, pick the latest "Build" run and download
the **GetMyFilesBack** artifact - the exe, built from that commit, after the image test passed.

### Building

```powershell
cd Undelete
powershell -ExecutionPolicy Bypass -File build.ps1          # Release
powershell -ExecutionPolicy Bypass -File build.ps1 -Debug   # Debug
```

`-ExecutionPolicy Bypass` is needed because Windows' default policy blocks running unsigned local
`.ps1` scripts; this only applies to this one invocation and doesn't change any system setting.

The result is `out\GetMyFilesBack.exe`. Everything the build generates stays in `out\`:
- `out\build\x64\` - the CMake build tree (a Visual Studio solution, `GetMyFilesBack.sln`)
- `out\vcpkg_installed\` - wxWidgets and its libraries, built from source by vcpkg as static libs
- `out\vcpkg-downloads\`, `out\vcpkg-cache\` - vcpkg's downloaded sources and built packages

The **first build takes 10-15 minutes**, because vcpkg compiles wxWidgets and its image libraries.
After that the libraries come from `out\vcpkg-cache` and a build takes about a minute.

Versions are pinned: `vcpkg.json` names the vcpkg registry commit (`builtin-baseline`), which fixes
the wxWidgets version (3.3.3) and every library it uses. Change that commit to upgrade.

The project can also be opened directly in Visual Studio ("Open a local folder" - it reads
`CMakePresets.json`), or built by hand:

```powershell
cmake --preset x64
cmake --build --preset release
```

`build.ps1` does the same after finding Visual Studio's CMake and vcpkg (`build-env.ps1`). To use your own
vcpkg checkout instead of the one that comes with Visual Studio, set `VCPKG_ROOT`.

### Older build

Until October 2026 the repo carried its own 32-bit toolchain in `tools/` (MinGW GCC 4.6.2 and static
wxWidgets 3.0.1). The tag **`last-gcc462-x86`** marks the last commit built that way - check it out to
rebuild that version.

### Running it

The built exe's manifest requests `requestedExecutionLevel="requireAdministrator"` (it needs raw NTFS
volume access), so:
- Started from Explorer, Windows shows the UAC prompt (a standard user is asked for admin
  credentials) - the app never runs without admin rights.
- Launching it from a non-elevated shell fails outright (`CreateProcess` error 740) rather than
  prompting for UAC. Launch it from Explorer or an elevated PowerShell/cmd.

## Tests

`tests\ntfs-image\run.ps1` builds an NTFS disk image with deleted files (in WSL, with ntfs-3g), runs the
recovery engine against it and checks every recovered file byte-for-byte - see
`tests\ntfs-image\README.md`. Besides the build requirements it needs WSL (`wsl --install`) with
`sudo apt-get install ntfs-3g python3` inside it. The GitHub build runs the same test on every push.

## Repo layout

- `CMakeLists.txt`, `CMakePresets.json`, `vcpkg.json` - the build; `build.ps1` / `build-env.ps1` run it.
- `.github/workflows/build.yml` - the GitHub Actions build and test.
- `src/UndeleteWizzard/` — the application source (wxWidgets wizard: NTFS MFT scan/recovery,
  multi-language UI, wizard pages). See `CLAUDE.md` for the internal architecture.
- `src/UndeleteWizzard/FAT/` — vendored TestDisk/PhotoRec source, kept in full for reference.
  **Not linked into the current build** — see "FAT support" below.
- `src/UndeleteWizzard/Makefile`, `nbproject/`, `*.conf` — the original NetBeans/MinGW build files of the
  commercial version. They point at paths on the original developer's machine and aren't used.
- `tests/ntfs-image/` — the end-to-end test of the recovery engine.

## FAT support (not implemented)

`Page_SelectDestinationDirPage.cpp` calls `photorecz()` only when the selected source drive is FAT, not
NTFS. The real `photorecz()` (in `FAT/phmain.c`) delegates into testdisk's own **interactive ncurses
disk/partition-selection UI** (`do_curses_photorec` in `FAT/pdisksel.c`) — this was never actually wired
up to the wx wizard, and would need a PDCurses-for-Windows build (not included here) plus a console,
which a windowed app doesn't have. The build links a stub `photorecz()` (`photorecz_stub.c`) instead, so
the real, finished feature (NTFS recovery) works; selecting a FAT drive will just do nothing useful.

The TestDisk code is written for GCC (`__attribute__((packed))`, `typeof`, `<sys/time.h>`...) and does not
compile with MSVC as it is. It is also GPL-licensed: linking it in would put the whole app under the GPL.
Native FAT32/exFAT support written for this app is the more practical route.
