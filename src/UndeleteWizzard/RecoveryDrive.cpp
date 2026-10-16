// RecoveryDrive.cpp: what the NTFS and FAT engines share - the scan results, the volume handle, the content
// check and the Recycle Bin names.

#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include <map>
#include <string>
#include "RecoveryDrive.h"
#include "FileContents.h"

void DeleteFileInfoHash(FileInfoHash *finfohash) {
    for (FileInfoHash::iterator it = finfohash->begin(); it != finfohash->end(); ++it)
        delete it->second;
    finfohash->clear();
}

std::string FileStatusText(const t_FileInfo &stFileInfo) {
    std::string sStatus;
    if (stFileInfo.dwAttributes & FILE_ATTRIBUTE_READONLY)
        sStatus += "Read Only, ";
    if (stFileInfo.dwAttributes & FILE_ATTRIBUTE_ENCRYPTED)
        sStatus += "Encrypted, ";
    if (stFileInfo.dwAttributes & FILE_ATTRIBUTE_COMPRESSED)
        sStatus += "Compressed, ";
    if (stFileInfo.dwAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
        sStatus += "Sparse, ";
    if (stFileInfo.dwAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        sStatus += "Reparse Point, ";
    if (stFileInfo.dwAttributes & FILE_ATTRIBUTE_OFFLINE)
        sStatus += "Offline, ";
    if (stFileInfo.bInRecycleBin)
        sStatus += "Deleted in Recycle Bin";
    else if (stFileInfo.bDeleted)
        sStatus += "Deleted";
    return sStatus;
}

CRecoveryDrive::CRecoveryDrive() {
    m_hDrive = 0;
    m_bOwnsHandle = false;
    m_pbCancel = 0;
    m_plProgressKB = 0;
}

CRecoveryDrive::~CRecoveryDrive() {
    CloseVolume();
}

void CRecoveryDrive::CloseVolume() {
    if (m_bOwnsHandle && m_hDrive && m_hDrive != INVALID_HANDLE_VALUE)
        CloseHandle(m_hDrive);
    m_bOwnsHandle = false;
}

int CRecoveryDrive::OpenVolume(const wxString &wxsSelectedDrive) {
    wxString sVolume = wxString("\\\\.\\") + wxsSelectedDrive.Left(2); // "\\.\E:"

    CloseVolume(); // the previous scan's drive

    m_hDrive = CreateFileW(sVolume.wc_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (m_hDrive == INVALID_HANDLE_VALUE) {
        DWORD dwErrorCode = GetLastError();
        wxLogMessage("Cannot open %s (error %u).", sVolume, (unsigned) dwErrorCode);
        m_hDrive = 0;
        return dwErrorCode ? (int) dwErrorCode : ERROR_ACCESS_DENIED;
    }
    m_bOwnsHandle = true;
    return ERROR_SUCCESS;
}

void CRecoveryDrive::CheckFileContents(t_FileInfo &stFileInfo) {
    if (!CanCheckFileContents(stFileInfo.sFilename.c_str())) {
        stFileInfo.iContent = CONTENT_NO_SIGNATURE; // nothing to compare with - don't even read it
        return;
    }
    if (stFileInfo.iRecoverability == RECOVERABILITY_OVERWRITTEN || stFileInfo.bStartReused) {
        // another file's data is there now - it could even be a file of the same type, which would "look valid"
        stFileInfo.iContent = CONTENT_START_OVERWRITTEN;
        return;
    }

    std::vector<BYTE> vStart;
    if (ReadFileStart(stFileInfo.iRecordId, vStart, CONTENT_CHECK_BYTES)) {
        stFileInfo.iContent = CONTENT_UNREADABLE;
        return;
    }
    stFileInfo.iContent = ClassifyFileContents(stFileInfo.sFilename.c_str(), vStart.empty() ? NULL : &vStart[0], (DWORD) vStart.size());
}

// Recycle Bin (Vista and later): a deleted file is renamed to $R<6 random chars><original extension>
// and a small companion file $I<same 6 chars><extension> holds its original full path:
//   version 1 (Vista - 8.1): 8 bytes version, 8 file size, 8 deletion time, path as 260 WCHARs
//   version 2 (Windows 10+): 8 bytes version, 8 file size, 8 deletion time, 4 path length in WCHARs, path

static bool GetOriginalPathFromRecycleBinInfo(const BYTE *puchData, DWORD dwLen, std::wstring &wsPath) {
    if (dwLen < 24)
        return false;

    LONGLONG llVersion;
    memcpy(&llVersion, puchData, sizeof (llVersion));

    const wchar_t *pwPath;
    DWORD dwMaxChars;
    if (llVersion == 1) {
        pwPath = (const wchar_t *) (puchData + 24);
        dwMaxChars = (dwLen - 24) / sizeof (wchar_t);
        if (dwMaxChars > MAX_PATH)
            dwMaxChars = MAX_PATH;
    } else if (llVersion == 2) {
        if (dwLen < 28)
            return false;
        memcpy(&dwMaxChars, puchData + 24, sizeof (DWORD));
        if (dwMaxChars > (dwLen - 28) / sizeof (wchar_t))
            return false;
        pwPath = (const wchar_t *) (puchData + 28);
    } else
        return false;

    DWORD dwChars = 0;
    while (dwChars < dwMaxChars && pwPath[dwChars])
        dwChars++;
    if (!dwChars)
        return false;

    wsPath.assign(pwPath, dwChars);
    return true;
}

void CRecoveryDrive::ResolveRecycleBinNames(FileInfoHash *finfohash) {
    FileInfoHash::iterator it;

    std::map<std::wstring, int> mapInfoFiles; // "$I...." name -> file id
    std::vector<int> vUsedInfoFiles;
    for (it = finfohash->begin(); it != finfohash->end(); ++it)
        if (!it->second->sFilename.compare(0, 2, L"$I"))
            mapInfoFiles[it->second->sFilename] = it->second->iRecordId;

    for (it = finfohash->begin(); it != finfohash->end(); ++it) {
        t_FileInfo *pFileInfo = it->second;
        if (pFileInfo->sFilename.compare(0, 2, L"$R") || pFileInfo->sFilename.length() < 8)
            continue;

        wxLogMessage("A file deleted from Recycle Bin is found: %s", wxString(pFileInfo->sFilename.c_str()));

        std::wstring wsInfoName = L"$I" + pFileInfo->sFilename.substr(2);
        std::map<std::wstring, int>::iterator itInfo = mapInfoFiles.find(wsInfoName);
        if (itInfo == mapInfoFiles.end()) {
            wxLogMessage("No %s found - the original name is lost.", wxString(wsInfoName.c_str()));
            continue;
        }

        BYTE *puchData = 0;
        DWORD dwLen = 0;
        if (Read_File(itInfo->second, puchData, dwLen)) {
            wxLogMessage("Error when trying to retrieve contents of %s", wxString(wsInfoName.c_str()));
            continue;
        }

        std::wstring wsPath;
        bool bFound = GetOriginalPathFromRecycleBinInfo(puchData, dwLen, wsPath);
        delete[] puchData;
        if (!bFound) {
            wxLogMessage("%s has an unknown format.", wxString(wsInfoName.c_str()));
            continue;
        }

        size_t nSlash = wsPath.find_last_of(L'\\');
        std::wstring wsName = (nSlash == std::wstring::npos) ? wsPath : wsPath.substr(nSlash + 1);
        if (wsName.empty())
            continue;

        pFileInfo->sFilename = wsName;
        pFileInfo->bInRecycleBin = true;

        // its real folder is the one it was deleted from, not $Recycle.Bin - without the drive letter, like the others
        std::wstring wsFolder = (nSlash == std::wstring::npos) ? L"" : wsPath.substr(0, nSlash);
        if (wsFolder.length() >= 2 && wsFolder[1] == L':')
            wsFolder.erase(0, 2);
        pFileInfo->sFolder = wsFolder.empty() ? L"\\" : wsFolder;
        vUsedInfoFiles.push_back(itInfo->second);
        wxLogMessage("A file name retrieved from Recycle Bin: %s", wxString(pFileInfo->sFilename.c_str()));
    }

    // a $I file that gave its file the name back is only bookkeeping - e.g. "$IA1B2C3.jpg" is no JPEG, don't list it
    for (size_t i = 0; i < vUsedInfoFiles.size(); i++) {
        it = finfohash->find(vUsedInfoFiles[i]);
        if (it != finfohash->end()) {
            delete it->second;
            finfohash->erase(it);
        }
    }
}
