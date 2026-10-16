# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

**GetMyFilesBack** (internal name "UndeleteWizzard") — a Windows GUI file-undelete/recovery utility written in
C++ with wxWidgets. It scans an NTFS or FAT12/16/32 drive for deleted-but-recoverable files, shows them in a wizard UI,
and copies recovered files to a destination chosen by the user. It was built/sold as a small commercial
product; its expiry check in `main.cpp` has since been removed (see below).

## Build

```powershell
powershell -ExecutionPolicy Bypass -File build.ps1   # -Debug for a debug build
```

64-bit MSVC, CMake (`CMakeLists.txt`, `CMakePresets.json`) and vcpkg (`vcpkg.json`: wxWidgets 3.3, pinned by
`builtin-baseline`). Needs Visual Studio 2022 or its Build Tools with the C++ workload - `build-env.ps1` finds
its CMake and vcpkg. Produces `out\GetMyFilesBack.exe`; all build output, vcpkg packages and caches stay in
`out\`. The first build compiles wxWidgets (10-15 min), later ones reuse `out\vcpkg-cache`.

- **Static on purpose**: triplet `x64-windows-static` plus the static CRT (`CMAKE_MSVC_RUNTIME_LIBRARY`) - a
  single exe that runs from a USB stick without installing anything on the drive being recovered. Keep it so.
- The app's sources are an **explicit list** in `CMakeLists.txt`, not a glob: the source tree may contain
  stale merge-conflict leftovers that must never be compiled (see below). Add new `.cpp` files there - GUI
  files to the `GetMyFilesBack` list, engine files (no GUI) to `ENGINE_SOURCES`, which the test harness uses too.
- `WIN32_LEAN_AND_MEAN` is defined for everything - a full `<windows.h>` drags in `winsock.h`, which clashes
  with wxWidgets' `winsock2.h`. Include `<shellapi.h>`, `<shlobj.h>`... explicitly where needed.
- No `using namespace std;` in headers: with C++17, `std::byte` clashes with the Windows SDK's `byte`.
- `undelete.rc` embeds `undelete.exe.manifest` (requireAdministrator, Common Controls 6) - the linker's own
  manifest is switched off (`/MANIFEST:NO`).
- `.github/workflows/build.yml` builds and tests every push on GitHub (Linux job makes the NTFS test image with
  ntfs-3g, Windows job builds, runs the NTFS test with `-UseExistingImage` and the FAT test) and attaches the
  exe as an artifact.
- The tag `last-gcc462-x86` is the last commit built with the old vendored 32-bit MinGW GCC 4.6.2 +
  wxWidgets 3.0.1 toolchain (`tools/`, removed after it).

The recovery engine has two automated end-to-end tests (see their READMEs), which run the engine against disk
images and check the recovered files byte-for-byte, plus what the engine reports about them:
- `tests\ntfs-image\run.ps1` - needs WSL with `ntfs-3g`. An NTFS image with deleted fragmented, sparse and
  resident files. Run it after touching `NTFSDrive.*`, `MFTRecord.*` or `RecoveryDrive.*`.
- `tests\fat-image\run.ps1` - needs Python 3 on Windows (installs pyfatfs into `out\fat-test\venv`). FAT12,
  FAT16 and FAT32 images. Run it after touching `FATDrive.*` or `RecoveryDrive.*`.

Both use one harness (`tests\ntfs-image\harness.cpp`), which picks the engine by the image's boot sector.
The GUI and Windows-specific behaviour are still tested by hand: run the wizard against a real/virtual drive
and check the recovered files.

## Architecture

- `src/UndeleteWizzard/main.cpp` — `wxApp` entry point (`MyApp::OnInit`). The original commercial build had a
  hardcoded expiry check: a "this version expires" popup after 2016-01-01, and an `exit(0)` after 2016-10-01.
  Both have been removed — the app no longer has any date-based behaviour.
- `MyWizzard.{h,cpp}` — the `wxWizard` (`MyWizard`) driving the UX: select source drive → select destination
  dir → select files to recover → done. Shared state (selected drive/dir, file hash map, the files list,
  recovery counts and failures, active language pack) lives on the wizard instance and is passed down to each
  page via getters/setters — follow this pattern for new wizard-wide state.
- `Page_*.{h,cpp}` — one `wxWizardPage` per step. `Page_SelectFilesPage` is the core page (lists recoverable
  files, filter buttons, triggers scan/copy). `Page_SelectDestinationDirPage` picks the engine by the source
  drive's file system (`GetVolumeInformation`: "NTFS", "FAT", "FAT32"), hands it to the wizard
  (`SetPtrDrive`, which deletes the previous one) and starts the scan. Anything else (exFAT...) is refused there.
- `RecoveryDrive.{h,cpp}` — `CRecoveryDrive`, the engine interface the wizard, threads and pages use
  (`GetPtrDrive()`), and what the engines share: `ST_FILEINFO`/`FileInfoHash`, the volume handle, the content
  check and the Recycle Bin names (`$R`/`$I`, read through the engine's `Read_File`). `iRecordId` is the
  engine's own id of a file: the MFT record on NTFS, an index into the scan's file list on FAT.
- `NTFSDrive.{h,cpp}` / `MFTRecord.{h,cpp}` — the NTFS recovery engine: parses the MFT (`CNTFSDrive`/
  `CMFTRecord`), builds a `FileInfoHash` of recoverable file records, copies a given MFT record's data to disk.
- `FATDrive.{h,cpp}` — `CFATDrive`, the FAT12/16/32 engine. Deleted entries have `0xE5` as their first byte, and
  their cluster chain is freed. The scan walks the folder tree from the root, deleted folders included while
  their first cluster is free. Long names come back whole, and the lost first byte of the 8.3 name is restored
  through the long-name checksum. The data's clusters are guessed (`GuessClusters`): from the first cluster on,
  skipping clusters in use. Clusters had to be skipped → never "Good". First cluster in use → judged as
  contiguous. The FAT is read once per scan; only the in-use bitmap is kept for recovery, so recovery uses the
  same guess the recoverability was judged by. Limits are in README.md's "FAT support".
- `MyThreadClass.{h,cpp}` — background-thread scanning/copying so the GUI stays responsive; reports progress
  with custom events (`wxEVT_SELECT_*_PULSE_DIALOG` / `_STOP_DIALOG`) sent with `ProcessEvent`, so the handlers
  run in the worker thread and only store values (the progress text behind a critical section on the page).
  Threads must not touch wx controls: the scan
  thread only fills the `FileInfoHash`, the page fills the list afterwards; the copy thread gets a
  `std::vector<t_FileInfo*>` collected by the page. Threads are joinable: the page `Wait()`s for them (also
  after Cancel, which they notice via a flag the engine checks) and reads the outcome from `m_iResult` - no
  `wxLogFatalError`/message boxes from threads or the engine; errors are returned and shown by the page.
  Free scan results with `DeleteFileInfoHash()`, never `clear()` alone.
- `DeletedFilesList.{h,cpp}` — `CDeletedFilesList`, the virtual (`wxLC_VIRTUAL`) list of deleted files. It
  holds only pointers and draws visible rows on demand, because a big drive yields hundreds of thousands of
  files. Search, sorting and the "Show" (recoverability) / "Type" filters work on its pointer vector. File
  types come from one extension table, `FileTypeOf()` — the type buttons, the filter and the preview all use it.
- Recoverability (NTFS): `CNTFSDrive` loads `$Bitmap` at the start of a scan and marks each deleted file Good /
  Partly overwritten / Overwritten by whether its clusters are in use now. "Good" can't tell whether a since-deleted file
  used the clusters in between - that is what the content check is for.
- Folders (NTFS): the scan also keeps every folder record (in use and deleted) and, at the end, builds each file's
  folder path from the parent references (`CFolderPaths` in `NTFSDrive.cpp`). A reference only counts if the
  folder record's sequence number still matches (or is one higher and the folder is deleted) - otherwise the
  record was reused and that part of the path is "?". Recycle Bin files take their folder from the `$I` path.
- `FileContents.{h,cpp}` — the content check: compares the first 4 KB of a file with the signature of its type
  (by extension) or checks text files for binary data. Done on demand (`CRecoveryDrive::CheckFileContents`: the
  "Check contents" button and the focused file), not during the scan - one read per file would make the scan of
  a big hard disk take far too long. Files whose first cluster is reused aren't read ("Start overwritten").
- `languages/` — plugin-style i18n: `ILanguage` interface (`getWXText(page, text)`), concrete packs
  (`English`, `Srpski`, `Srpski_Cyr`, `Deutsch`) via `LanguageFactory`/`LanguageDialog`.
- `FAT/` — vendored TestDisk/PhotoRec source (~357 files), kept for reference only and **not linked into the
  build** (GCC-only code, its own ncurses UI, GPL). FAT drives are read by `FATDrive.cpp`.
- Comments/identifiers mix English and Serbian (Latin and occasionally Cyrillic) — normal for this codebase.

### Things that look like source but aren't

`Page_SelectFilesPage.cpp.{BACKUP,BASE,LOCAL,REMOTE}.7468.cpp` would exist here if this were copied from an
unresolved merge in the original tree — they were deliberately excluded when this repo was assembled. If you
ever see similarly-named stray files reappear (e.g. from a careless merge), don't let them get compiled; they
aren't real source and will produce duplicate-symbol link errors if picked up by a glob.
