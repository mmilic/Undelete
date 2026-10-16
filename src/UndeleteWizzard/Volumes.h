/*
 * File:   Volumes.h
 *
 * Which drive a folder is on - recovered files must never be written to the drive they come from, because
 * every file written there can overwrite the next deleted file.
 */

#ifndef VOLUMES_H
#define	VOLUMES_H

#include <wx/string.h>
#include <vector>

// true if sPath is on the volume sSource ("E:\") - writing there could overwrite deleted files. A folder where
// another drive is mounted (C:\Mount\USB) belongs to that drive, a SUBST drive letter to the drive it points
// into. A path that doesn't exist yet: its deepest existing folder decides. A disk image as the source: never -
// writing beside the image file can't change what is in it.
bool IsOnSameVolume(const wxString &sPath, const wxString &sSource);

// folders to save recovered files in, best first: RetrievedFiles on the Desktop, then on the other hard
// disks and USB drives - none on the source drive. At most iMax.
std::vector<wxString> SuggestDestinations(const wxString &sSource, size_t iMax);

#endif	/* VOLUMES_H */
