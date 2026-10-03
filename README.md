# GetMyFilesBack (UndeleteWizzard)

A Windows GUI file-undelete/recovery tool (wxWidgets wizard) that scans a drive's NTFS MFT for
deleted-but-recoverable files and copies them to a destination you choose.

This repo intentionally vendors its entire build toolchain (`tools/`) alongside the source
(`src/`), so a plain `git clone` + `build.ps1` is enough to produce a working `.exe` with nothing
else installed. This trades repo size for zero-setup builds — a deliberate, temporary choice, not
a recommended long-term layout.

## Build

```powershell
cd Undelete
powershell -ExecutionPolicy Bypass -File build.ps1
```

`-ExecutionPolicy Bypass` is needed because Windows' default policy blocks running unsigned local
`.ps1` scripts; this only applies to this one invocation and doesn't change any system setting.

The build is out-of-source: it produces `out\GetMyFilesBack.exe` (plus intermediate `.o`/`.res`
files in `out\`), leaving `src\` untouched. No MinGW/wxWidgets installation needed — the script
uses the compiler and static libraries vendored in `tools/`.

### Running it

The built exe's manifest requests `requestedExecutionLevel="highestAvailable"` (it needs raw NTFS
volume access), so:
- Launching it from a non-elevated shell fails outright (`CreateProcess` error 740) rather than
  prompting for UAC.
- Launch it from Explorer or an elevated PowerShell/cmd.

### A note on the clock

`src/UndeleteWizzard/main.cpp` has a hardcoded expiry check (`DateUsageLimit`, originally
2016-10-01) left over from when this was sold commercially — on any later system clock it just
`exit(0)`s immediately with no window. That check is currently commented out in this repo's copy
of `main.cpp` so the wizard actually runs; if you restore it, update the dates first.

## Repo layout

- `src/UndeleteWizzard/` — the application source (wxWidgets wizard: NTFS MFT scan/recovery,
  multi-language UI, wizard pages). See `CLAUDE.md` for the internal architecture.
- `src/UndeleteWizzard/FAT/` — vendored TestDisk/PhotoRec source, kept in full for reference.
  **Not linked into the current build** — see "FAT support" below.
- `tools/MinGW/` — GCC 4.6.2 MinGW toolchain (g++, gcc, windres, ar, ld, binutils, runtime headers
  and import libs). Trimmed: the original install's `msys/`, `var/`, and `share/` subtrees were
  dropped since they're only needed to run `configure`/`make` to rebuild wxWidgets itself from
  source — not to compile/link this app against the already-built static wx libs below.
- `tools/wxWidgets-3.0.1_static/` — the subset of a static wxWidgets 3.0.1 MSW build actually
  needed to link against it: `include/` (headers), `lib/*.a` (the static archives), and
  `lib/wx/include/msw-unicode-static-3.0/` (the generated `wx/setup.h` for this build
  configuration). The original build directory (not vendored here — it's ~1.3GB of intermediate
  `.o` files and build logs) also contains the actual wxWidgets source if you ever need to rebuild
  it from scratch.

**Toolchain/library version matters.** The static wxWidgets libs here were built with GCC 4.6.2.
Linking this app with a *different* GCC (e.g. 4.8.x) compiles and links without any error, but the
resulting exe segfaults immediately inside wxWidgets' own pre-`main` static initializers — a
libstdc++ ABI mismatch between GCC versions, not a bug in the app. `build.ps1` always uses
`tools\MinGW` for this reason; don't repoint it at a system-installed MinGW/MSYS2 toolchain.

## FAT support (not implemented)

`Page_SelectDestinationDirPage.cpp` calls `photorecz()` (declared in `FAT/phmain.h`) only when the
selected source drive is FAT, not NTFS. The real `photorecz()` (in `FAT/phmain.c`) delegates into
testdisk's own **interactive ncurses disk/partition-selection UI** (`do_curses_photorec` in
`FAT/pdisksel.c`) — this was never actually wired up to the wx wizard, and would need a vendored
PDCurses-for-Windows build (not included here) plus a console, which a windowed (`-mwindows`) app
doesn't have. `build.ps1` links a stub `photorecz()` instead, so the build succeeds and the real,
finished feature (NTFS recovery) works; selecting a FAT drive will just do nothing useful.

If you want to pick this up: ~335 of the ~357 files under `FAT/` compile standalone with just
`-DHAVE_CONFIG_H` (that macro is what makes `config.h` get included, which is what defines
`DWORD`/`HANDLE`/etc. via `windef.h`/`winbase.h`/`winioctl.h`). The other ~22 are testdisk's
curses-only menu screens (`intrfn.c`, `phnc.c`, `pdisksel.c`, `phrecn.c`, the `part*n.c` files) plus
the now-dead `addpart.c` (only those curses files call it) — real FAT support means either vendoring
PDCurses and linking those in, or rewriting `do_curses_photorec`'s UI calls to report progress into
the wizard instead of a curses window.
