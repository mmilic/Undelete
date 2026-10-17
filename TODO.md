# To do

## Recovering more files

1. **NTFS: a fragmented MFT isn't scanned to its end.** When the MFT's own run list continues in another
   record (an attribute list), only its first part is scanned (`NTFSDrive.cpp`, `LoadMFT`) - deleted files
   further on silently never show up. Hits big, old, much-used drives.
2. **Heavily fragmented NTFS files**: the same attribute list for a file's data runs isn't followed either -
   such files are refused.
3. **NTFS-compressed files**: LZNT1 decompression. Today they are refused.
4. **"Save a disk image first"**: copy a failing card or the system drive to an image file, then scan the
   image. The app can already read images.
5. **Recreate the folder structure** as an option when recovering. Today all files land in one folder.

## Product and release

- **The name**: the product is BitBack, the app still says GetMyFilesBack - window title, exe name, version
  info, the language dialog, the README.
- **Payment**: a provider that handles EU VAT, and whether the paid version needs a licence check (the app
  has none). The 2014 site sold through ClickBank.
- **The website** (`website/`) before it goes online: the price and the payment link, an imprint and a privacy
  policy (required by law in Germany), the licence terms, Lato's licence text (`OFL.txt`), the 2014 social
  accounts, a screenshot without the developer's drives - see `website/README.md`.
- **Code signing**: without it, the admin prompt says "Unknown publisher" and SmartScreen may warn. It needs
  a paid certificate.

## Testing

- **Real hardware**: an elevated scan of a real NTFS drive, a FAT32 USB stick and an exFAT SD card (written and
  deleted from by Windows, a camera and a phone). Also check the 4K-sector and big-cluster fixes on real disks,
  and the free-space warning.
- **FAT32 on real cards**: for files whose type can't be checked, the first cluster's high word that Windows
  cleared is guessed from the neighbouring files (README.md, "FAT"). Check how often that is right on a
  well-used card.
- **A short manual checklist per release** (`TESTING.md`): the GUI from a USB stick, the same-drive and
  free-space warnings, 100% and 150% display scaling, German.

## Build

- **Visual Studio 2026 only**: `CMakePresets.json` uses the "Visual Studio 17 2022" generator and `build-env.ps1`
  prefers VS 2022 (vcpkg must build with the same compiler). A machine with only VS 2026 needs a second preset
  with the "Visual Studio 18 2026" generator, chosen by `build-env.ps1`.

## More languages (30-40)

- Switch `Tr()` to gettext `.po`/`.mo` files through `wxTranslations`, embedded as resources so it stays one
  exe. Move German over. Add a plural-aware call: Slavic languages have 3 plural forms and Arabic has 6. Add a
  CI check for untranslated texts.
- Start in the Windows language automatically, with a language choice in the window that is remembered,
  instead of a start dialog with 40 entries.
- About 35 left-to-right languages first, then check each script's look: the CJK, Thai and Hindi font fallback
  in the hand-drawn controls.
- Right-to-left languages (Arabic, Hebrew, Persian, Urdu) as a separate step: the hand-drawn controls need
  mirroring.
- Embed wxWidgets' own translations (`wxstd`).
- Still to decide: which languages, and whether machine translations may ship before a native speaker has
  checked them.
