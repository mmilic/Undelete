# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

**GetMyFilesBack** (internal name "UndeleteWizzard") — a Windows GUI file-undelete/recovery utility written in
C++ with wxWidgets. It scans a drive's NTFS MFT for deleted-but-recoverable files, shows them in a wizard UI,
and copies recovered files to a destination chosen by the user. It was built/sold as a small commercial
product, which is why licensing/expiry logic lives in `main.cpp`.

This repo is self-contained on purpose: `src/` has the application source and `tools/` vendors the exact
compiler and static-library versions needed to build it, so `git clone` + `build.ps1` is enough — no MinGW or
wxWidgets install required. That's a deliberate size-for-convenience tradeoff, not a recommended long-term
layout; see README.md.

## Build

```powershell
powershell -File build.ps1
```

Produces `src\UndeleteWizzard\GetMyFilesBack.exe`. Read `build.ps1` before changing the source file list —
it compiles an explicit list of `.cpp` files (not a glob), because the source tree also contains stale
merge-conflict leftovers that must never be compiled (see below).

**Toolchain version is load-bearing.** `tools\MinGW` is GCC 4.6.2 specifically, matching what the vendored
static wxWidgets 3.0.1 libs (`tools\wxWidgets-3.0.1_static`) were built with. Linking this app with a
*different* GCC (e.g. 4.8.x) compiles and links with zero errors, but the resulting exe segfaults inside
wxWidgets' own pre-`main` static initializers (`wxClassInfo::Register()` constructing a `std::wstring`) — a
libstdc++ ABI mismatch between GCC versions, confirmed via `gdb -batch -ex run -ex bt`. Don't repoint the
build at a system-installed MinGW/MSYS2/newer GCC.

The compiler needs **its own `tools\MinGW\bin` directory on `PATH`**, not just its full path on the command
line — `cc1plus.exe` fails to start (`STATUS_DLL_NOT_FOUND`) if that directory (which holds the compiler's own
support DLLs) isn't on `PATH`. `build.ps1` handles this.

There is no automated test suite — validation is manual (run the wizard against a real/virtual drive and
check recovered files).

## Architecture

- `src/UndeleteWizzard/main.cpp` — `wxApp` entry point (`MyApp::OnInit`). Originally had a hardcoded date gate
  (`DateUsageLimit`, 2016-10-01) that calls `exit(0)` immediately on any later system clock — commented out in
  this repo's copy so the wizard actually runs. If you ever restore it, update the dates first.
- `MyWizzard.{h,cpp}` — the `wxWizard` (`MyWizard`) driving the UX: select source drive → select destination
  dir → select files to recover → done. Shared state (selected drive/dir, file hash map, mutex/critical
  section for cross-thread list updates, log file handle, active language pack) lives on the wizard instance
  and is passed down to each page via getters/setters — follow this pattern for new wizard-wide state.
- `Page_*.{h,cpp}` — one `wxWizardPage` per step. `Page_SelectFilesPage` is the core page (lists recoverable
  files, filter buttons, triggers scan/copy). `Page_SelectDestinationDirPage` is where the FAT/NTFS branch
  happens (see "FAT support" below).
- `NTFSDrive.{h,cpp}` / `MFTRecord.{h,cpp}` — the actual NTFS recovery engine: parses the MFT (`CNTFSDrive`/
  `CMFTRecord`), builds a `FileInfoHash` of recoverable file records, copies a given MFT record's data to disk.
  This is the real, finished, working feature.
- `MyThreadClass.{h,cpp}` — background-thread scanning/copying so the GUI stays responsive; posts custom
  events (`wxEVT_SHOW_NOTIFICATION`, `wxEVT_ENABLE_GUI_FIELDS`) back to the main thread, guarded by the
  wizard's mutex/critical section when touching shared state.
- `languages/` — plugin-style i18n: `ILanguage` interface (`getWXText(page, text)`), concrete packs
  (`English`, `Srpski`, `Srpski_Cyr`, `Deutsch`) via `LanguageFactory`/`LanguageDialog`.
- `FAT/` — vendored TestDisk/PhotoRec source (~357 files), kept in full but **not linked into the build**.
  `build.ps1` compiles a stub `photorecz()` instead. See README.md's "FAT support" section for exactly why and
  what finishing it would take — short version: the real `photorecz()` delegates into testdisk's own
  interactive ncurses UI, which needs PDCurses (not vendored here) and was never wired up to this wx GUI.
- Comments/identifiers mix English and Serbian (Latin and occasionally Cyrillic) — normal for this codebase.

### Things that look like source but aren't

`Page_SelectFilesPage.cpp.{BACKUP,BASE,LOCAL,REMOTE}.7468.cpp` would exist here if this were copied from an
unresolved merge in the original tree — they were deliberately excluded when this repo was assembled. If you
ever see similarly-named stray files reappear (e.g. from a careless merge), don't let them get compiled; they
aren't real source and will produce duplicate-symbol link errors if picked up by a glob.
