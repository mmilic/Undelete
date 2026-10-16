// Volumes.cpp: which volume a path is on, and where recovered files can go

#include <windows.h>
#include <shlobj.h>
#include <wx/wx.h>
#include <wx/filename.h>
#include "Volumes.h"

// The volume a path is on, as "\\?\Volume{GUID}\". The path doesn't have to exist yet - its deepest
// existing folder decides. Empty if Windows can't tell.

static wxString VolumeOf(const wxString &sPath) {
    wxFileName fn = wxFileName::DirName(sPath);
    fn.MakeAbsolute();
    while (fn.GetDirCount() && !fn.DirExists())
        fn.RemoveLastDir();

    wchar_t wszMountPoint[MAX_PATH + 1], wszVolume[64];
    if (!GetVolumePathNameW(fn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR).wc_str(), wszMountPoint, MAX_PATH + 1))
        return wxEmptyString;
    if (GetVolumeNameForVolumeMountPointW(wszMountPoint, wszVolume, 64))
        return wxString(wszVolume).Upper();

    // a SUBST drive letter is a folder of another drive: "\??\C:\Photos" - that drive decides
    wxString sMountPoint(wszMountPoint);
    wchar_t wszTarget[MAX_PATH + 1];
    if (sMountPoint.length() == 3 && sMountPoint[1] == ':'
            && QueryDosDeviceW(sMountPoint.Left(2).wc_str(), wszTarget, MAX_PATH + 1)
            && wxString(wszTarget).StartsWith("\\??\\") && wxString(wszTarget).length() > 6)
        return VolumeOf(wxString(wszTarget).Mid(4));
    return sMountPoint.Upper(); // a network share has no volume name - its path will do
}

bool IsOnSameVolume(const wxString &sPath, const wxString &sSource) {
    // A disk image is a file in use: writing beside it can't overwrite what is in it. Only the free space of
    // a drive that is scanned itself is at risk.
    if (wxFileName::FileExists(sSource))
        return false;
    wxString sVolume1 = VolumeOf(sPath), sVolume2 = VolumeOf(sSource);
    if (sVolume1.IsEmpty() || sVolume2.IsEmpty()) // can't tell - compare the drive letters
        return !sPath.IsEmpty() && !sSource.IsEmpty() && wxToupper(sPath[0]) == wxToupper(sSource[0]);
    return sVolume1 == sVolume2;
}

std::vector<wxString> SuggestDestinations(const wxString &sSource, size_t iMax) {
    std::vector<wxString> vSuggestions;
    wchar_t wszDesktop[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, wszDesktop))) {
        wxString sDestination = wxString(wszDesktop) + "\\RetrievedFiles";
        if (!IsOnSameVolume(sDestination, sSource))
            vSuggestions.push_back(sDestination);
    }

    // an empty card reader must not bring up a "no disk in the drive" message box
    DWORD dwOldErrorMode;
    SetThreadErrorMode(SEM_FAILCRITICALERRORS, &dwOldErrorMode);
    wchar_t wszDrives[512];
    DWORD dwLen = GetLogicalDriveStringsW(511, wszDrives);
    for (wchar_t *pDrive = wszDrives; dwLen && dwLen < 512 && *pDrive && vSuggestions.size() < iMax; pDrive += wcslen(pDrive) + 1) {
        UINT uType = GetDriveTypeW(pDrive);
        DWORD dwFlags = 0;
        if ((uType == DRIVE_FIXED || uType == DRIVE_REMOVABLE)
                && GetVolumeInformationW(pDrive, NULL, 0, NULL, NULL, &dwFlags, NULL, 0) // a disk is in it
                && !(dwFlags & FILE_READ_ONLY_VOLUME)
                && !IsOnSameVolume(pDrive, sSource))
            vSuggestions.push_back(wxString(pDrive) + "RetrievedFiles");
    }
    SetThreadErrorMode(dwOldErrorMode, NULL);
    if (vSuggestions.size() > iMax)
        vSuggestions.resize(iMax);
    return vSuggestions;
}
