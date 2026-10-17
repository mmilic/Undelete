# GetMyFilesBack (UndeleteWizzard)

A Windows GUI file-undelete/recovery tool (wxWidgets) that scans an NTFS or FAT (FAT12/16/32)
drive for deleted-but-recoverable files and copies them to a destination you choose.

It builds as one 64-bit exe with everything linked in statically - no DLLs, no installer. That is
on purpose: a recovery tool should run from a USB stick, not be installed onto the drive whose
deleted files you want back.

Contents: [Just use it](#just-use-it) · [1. Install the tools](#1-install-the-tools) ·
[2. Get the source and build](#2-get-the-source-and-build) · [3. Deploy and run](#3-deploy-and-run) ·
[4. Tests](#4-tests) · [More about the build](#more-about-the-build) · [Repo layout](#repo-layout) ·
[FAT support](#fat-support)

## Just use it

No need to build anything: download **`GetMyFilesBack.exe`** from the latest release on the repo's
**Releases** page (https://github.com/mmilic/Undelete/releases), then continue with
[3. Deploy and run](#3-deploy-and-run).

Every release and every push to `master` is built and tested on GitHub's own Windows machines
(`.github/workflows/build.yml`). For a build newer than the latest release: open the **Actions** tab, pick the
latest successful **Build** run and download **GetMyFilesBack** under **Artifacts** - a zip with the exe
(you need to be signed in to GitHub).

## 1. Install the tools

| What | Why |
|---|---|
| Windows 10 or 11, 64-bit | the app and its compiler are Windows-only |
| **Build Tools for Visual Studio 2022** (compiler only, no IDE) - or Visual Studio 2022 with **"Desktop development with C++"** | MSVC x64 compiler, Windows SDK, CMake and vcpkg |
| Git | to clone the repo; vcpkg uses it too |
| Internet access on the first build | vcpkg downloads wxWidgets and its libraries from GitHub |
| ~10 GB free disk space | ~6-8 GB for the Build Tools, ~5 GB for the wxWidgets build and its caches in `out\` |

That's all - no MinGW, no separately installed CMake, no wxWidgets download, no admin rights for building.
Windows PowerShell 5.1, which every Windows 10/11 has, runs the build scripts.

Install them with **winget**, in a **Command Prompt (`cmd`)**. Run the commands **one at a time** - `;` is not
a command separator in cmd, so two commands on one line joined with `;` end up as arguments of the first one.
Windows asks for admin rights once per installer.

```cmd
winget install --exact --id Git.Git --accept-package-agreements --accept-source-agreements
```

```cmd
winget install --exact --id Microsoft.VisualStudio.2022.BuildTools --accept-package-agreements --accept-source-agreements --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --add Microsoft.VisualStudio.Component.VC.CMake.Project --add Microsoft.VisualStudio.Component.Vcpkg"
```

The second one installs the Build Tools for Visual Studio 2022 with the C++ workload (`VCTools`; its
recommended components include the MSVC x64 compiler and the Windows SDK) plus CMake and vcpkg, named
explicitly so they're there even if the defaults change. Its installer shows a progress window and takes a
while (several GB). Already have Visual Studio 2022 with "Desktop development with C++"? Then skip it.

**Close the Command Prompt and open a new one** afterwards, so `git` is on the `PATH`.

Things to know:
- **Visual Studio 2022 specifically.** `CMakePresets.json` uses the "Visual Studio 17 2022" generator; a machine
  with only a newer Visual Studio needs that line changed.
- **The tests need more** - Python, and WSL for the NTFS test. Building the app doesn't. See [4. Tests](#4-tests).

## 2. Get the source and build

**Keep the clone path short**, e.g. `C:\src\Undelete`: vcpkg's build paths inside `out\` are long, and a deeply
nested clone can hit Windows' 260-character path limit while building wxWidgets.

```cmd
git clone https://github.com/mmilic/Undelete.git C:\src\Undelete
```

```cmd
cd /d C:\src\Undelete
```

```cmd
powershell -ExecutionPolicy Bypass -File build.ps1
```

`-ExecutionPolicy Bypass` is needed because Windows' default policy blocks running unsigned local `.ps1`
scripts; it only applies to this one run and doesn't change any system setting. Add `-Debug` at the end for a
debug build.

The **first build takes 10-15 minutes**, because vcpkg compiles wxWidgets and its image libraries. After that
the libraries come from `out\vcpkg-cache` and a build takes about a minute.

The result is **`out\GetMyFilesBack.exe`** - the only file you need to run the app.

## 3. Deploy and run

There is nothing to install: the exe is the whole app.

1. **Copy `GetMyFilesBack.exe` to a USB stick** (or any drive other than the one with the deleted files).
   Every file written to the drive that held the deleted files can overwrite them - so don't download, copy
   or install anything there until the recovery is done.
2. On the computer with the deleted files, **start it from the USB stick**: double-click it in Explorer and
   confirm the Windows prompt. It needs administrator rights to read the drive directly.
   - From a Command Prompt, use one opened with "Run as administrator". From a normal one, Windows refuses to
     start it (error 740) instead of asking.
3. Pick the language, then **pick the drive** that held the files. Its deleted files appear in the list
   while it is being scanned - tick the ones you want.
4. Click **Recover** and choose a folder **on another drive** - the USB stick is a good choice. The app
   refuses a folder on the drive being scanned. A log file of the recovery is saved in that folder too.

**Disk images:** files named on the command line are offered beside the drives, as disk images - e.g. a copy
of a failing memory card, so the card itself is read only once:

```cmd
GetMyFilesBack.exe E:\card.img
```

Reading an image needs no admin rights. To try the app without them (e.g. with the test images), skip the
Windows prompt for this one start:

```cmd
set __COMPAT_LAYER=RunAsInvoker
```

```cmd
out\GetMyFilesBack.exe out\fat-test\fat32.img
```

## 4. Tests

Two end-to-end tests build disk images with deleted files, run the recovery engine against them and check
every recovered file byte-for-byte. The GitHub build runs both on every push.

**FAT test** (FAT12, FAT16, FAT32) - needs Python 3 on Windows. It installs `pyfatfs` into a venv under
`out\fat-test` on its own; no WSL. See `tests\fat-image\README.md`.

```cmd
winget install --exact --id Python.Python.3.13 --accept-package-agreements --accept-source-agreements
```

Open a new Command Prompt, then:

```cmd
powershell -ExecutionPolicy Bypass -File tests\fat-image\run.ps1
```

**NTFS test** - the image is made with `ntfs-3g` in WSL. See `tests\ntfs-image\README.md`. Install WSL with
Ubuntu (asks for admin rights; restart Windows if it asks, then finish Ubuntu's first start, which asks for a
user name and password):

```cmd
wsl --install -d Ubuntu
```

Then `ntfs-3g` and Python inside it:

```cmd
wsl -d Ubuntu -u root -e sh -c "apt-get update && apt-get install -y ntfs-3g python3"
```

```cmd
powershell -ExecutionPolicy Bypass -File tests\ntfs-image\run.ps1
```

## More about the build

Everything the build generates stays in `out\`:
- `out\build\x64\` - the CMake build tree (a Visual Studio solution, `GetMyFilesBack.sln`)
- `out\vcpkg_installed\` - wxWidgets and its libraries, built from source by vcpkg as static libs
- `out\vcpkg-downloads\`, `out\vcpkg-cache\` - vcpkg's downloaded sources and built packages

Versions are pinned: `vcpkg.json` names the vcpkg registry commit (`builtin-baseline`), which fixes the
wxWidgets version (3.3.3) and every library it uses. Change that commit to upgrade.

The project can also be opened directly in Visual Studio ("Open a local folder" - it reads
`CMakePresets.json`), or built by hand from a "Developer Command Prompt for VS 2022":

```cmd
cmake --preset x64
```

```cmd
cmake --build --preset release
```

`build.ps1` does the same after finding Visual Studio's CMake and vcpkg (`build-env.ps1`). To use your own
vcpkg checkout instead of the one that comes with Visual Studio, set `VCPKG_ROOT`.

**Making a release:** the version is written in one place, `project(GetMyFilesBack VERSION ...)` in
`CMakeLists.txt` - the window title and the exe's file properties show it. Raise it, commit and push, then tag
that commit with `v` and the version and push the tag, e.g. for version 2.7:

```cmd
git tag v2.7
```

```cmd
git push origin v2.7
```

The GitHub build then builds and tests the tagged commit and, if the tests pass, makes the release with the
exe. A tag that doesn't match the version in `CMakeLists.txt` fails the build before anything is released.

**Older build:** until October 2026 the repo carried its own 32-bit toolchain in `tools/` (MinGW GCC 4.6.2 and
static wxWidgets 3.0.1). The tag **`last-gcc462-x86`** marks the last commit built that way - check it out to
rebuild that version.

## Repo layout

- `CMakeLists.txt`, `CMakePresets.json`, `vcpkg.json` - the build; `build.ps1` / `build-env.ps1` run it.
- `.github/workflows/build.yml` - the GitHub Actions build and test.
- `src/UndeleteWizzard/` — the application source (the window, NTFS and FAT scan/recovery,
  multi-language UI). See `CLAUDE.md` for the internal architecture.
- `src/UndeleteWizzard/FAT/` — vendored TestDisk/PhotoRec source, kept for reference only.
  **Not linked into the build** — FAT drives are read by `FATDrive.cpp`, see "FAT support" below.
- `tests/ntfs-image/`, `tests/fat-image/` — the end-to-end tests of the recovery engine.

## FAT support

FAT12, FAT16 and FAT32 drives (USB sticks, memory cards) are scanned by `FATDrive.cpp`, the app's own
engine; the app picks it by the drive's boot sector. **exFAT is not supported yet** - an exFAT drive
is shown greyed out, with a note.

What FAT leaves of a deleted file decides what can be recovered:
- Windows marks the file's directory entry as deleted (its first byte becomes `0xE5`) and frees its
  clusters in the FAT. The size, the dates and the first cluster stay.
- **The name**: a long file name comes back in full, and so does the first character of the short (8.3)
  name, via the checksum the long name entries keep. A file with only an 8.3 name loses its first
  character - it is listed as `_EADME.TXT`. In the Recycle Bin the lost character is always `$`, so
  `$R`/`$I` pairs still get their original names back.
- **The data**: the chain of clusters is gone, only the first cluster is known. The engine takes as many
  clusters as the size needs from the first one on, skipping clusters in use now (what DOS UNDELETE did).
  - A contiguous file comes back whole, flagged Good.
  - A file fragmented around files that still exist comes back whole too, but is flagged "Partly
    overwritten": a file that was really partly overwritten looks the same.
  - A file fragmented around space that is free now comes back **wrong**, flagged Good - nothing on the
    volume tells it apart from a contiguous one. Its first cluster is right (the content check passes),
    the rest is whatever was in the free clusters after it.
  - If its first cluster is in use, the file is judged like a contiguous one: "Overwritten" or "Partly
    overwritten".
- **Folders**: the scan walks the folder tree, so every file comes with its folder. A deleted folder is
  walked too, while its first cluster is free and still holds directory entries. A folder that grew beyond
  one cluster lost the chain to its other clusters as well: they are looked for further on (up to 64 MB past
  its first cluster) - free clusters that hold directory entries and no other folder has taken. If two
  deleted folders grew at the same time, a file can end up listed under the wrong one of them.

The TestDisk/PhotoRec source in `FAT/` was the original plan for FAT and isn't used: it is written for GCC,
its recovery runs its own interactive ncurses UI, and it is GPL-licensed - linking it in would put the
whole app under the GPL.
