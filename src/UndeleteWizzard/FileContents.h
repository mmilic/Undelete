/*
 * File:   FileContents.h
 *
 * Does a deleted file's data still look like what its name says it is? "Good" recoverability only means
 * the file's clusters are free now - another file may have used them in between and been deleted too.
 * The start of the data is compared with the signature of the file type (a JPEG starts with FF D8 FF,
 * a PDF with "%PDF", Office files with "PK"...), and text files are checked for binary garbage.
 */

#ifndef FILECONTENTS_H
#define	FILECONTENTS_H

#include <windows.h>

enum {
    CONTENT_NOT_CHECKED = 0,
    CONTENT_OK, // the start matches the file type
    CONTENT_DAMAGED, // the start doesn't match - other data is there now
    CONTENT_ZEROS, // the start is all zeros - the data was wiped or never written
    CONTENT_START_OVERWRITTEN, // the first cluster is in use by another file now - not worth reading
    CONTENT_NO_SIGNATURE, // a file type that can't be checked this way (or an empty file)
    CONTENT_UNREADABLE // the data can't be read (compressed, read error...)
};

// how much of the start of a file the check needs (a TAR header is at 257, a PDF may start a bit late)
#define CONTENT_CHECK_BYTES 4096

const char *ContentLabel(int iContent); // "Looks valid", "Damaged"...

// false for types without a known signature - those don't need to be read at all
bool CanCheckFileContents(const wchar_t *szFilename);

// puchStart: the first dwLen bytes of the file (dwLen may be less than CONTENT_CHECK_BYTES for a small file)
int ClassifyFileContents(const wchar_t *szFilename, const BYTE *puchStart, DWORD dwLen);

#endif	/* FILECONTENTS_H */
