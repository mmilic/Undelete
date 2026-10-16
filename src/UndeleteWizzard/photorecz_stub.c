/* Build-only stub: the real photorecz() lives in FAT/phmain.c and delegates into
 * testdisk's interactive ncurses UI (do_curses_photorec in FAT/pdisksel.c), which
 * isn't wired up to this wx GUI and needs PDCurses, not vendored in this repo.
 * This lets the NTFS recovery path (the actual finished feature) build and link.
 * See README.md, "FAT support". */
int photorecz(char drive_letter, const char *dest_path) {
    (void) drive_letter;
    (void) dest_path;
    return -1;
}
