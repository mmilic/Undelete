# To do

## FAT32: deleted files past cluster 65535 (likely top priority)

Found by the Windows-written image test (`tests/windows-image`). On a FAT32 volume that Windows formatted
and wrote, most deleted files come back with the wrong data or flagged "Overwritten", and a file in a deleted
folder is missing.

The suspected cause, which fits every symptom but isn't confirmed yet: when Windows deletes a file on FAT32,
it clears the high word of the first cluster number in the directory entry. On a volume with more than 65,535
clusters, which is every real USB stick and memory card, files past that cluster then point to the wrong
clusters. The FAT test can't show it: its images are small, and pyfatfs keeps the high word.

- Confirm it: upload the Windows-made FAT32 image from CI and read the directory entries.
- Fix idea: try `low16 + k * 65536` for every possible `k`. A folder can be identified exactly by its `.`
  entry, which holds its own full cluster number. For a file, take the candidate whose clusters are free and
  fit its size.
- Then add `@("fat32", "FAT32", 512, 64)` back to `$Images` in `tests/windows-image/run.ps1`.

## Recovering more files

1. **exFAT support**: the format of nearly every SD card and USB stick over 32 GB. These are shown
   greyed out today.
2. **"Save a disk image first"**: copy a failing card or the system drive to an image file, then scan the
   image. The app can already read images.
3. **Heavily fragmented NTFS files**: an attribute list, with the data runs in a second MFT record, isn't
   followed yet.
4. **NTFS-compressed files**: LZNT1 decompression. Today they are refused.
5. **Recreate the folder structure** as an option when recovering. Today all files land in one folder.

## Release

- **Code signing**: without it, the admin prompt says "Unknown publisher" and SmartScreen may warn. It needs
  a paid certificate.

## Testing

- **Real hardware**: an elevated scan of a real NTFS drive, a USB stick and an SD card. Also check the 4K-sector
  and big-cluster fixes on real disks, and the free-space warning.
- **A short manual checklist per release** (`TESTING.md`): the GUI from a USB stick, the same-drive and
  free-space warnings, 100% and 150% display scaling, German.
- **WSL with Ubuntu locally**, so the NTFS test runs before pushing (`wsl --install -d Ubuntu`).

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
