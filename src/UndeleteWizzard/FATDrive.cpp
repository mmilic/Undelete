// FATDrive.cpp: the FAT12/16/32 recovery engine - see FATDrive.h for how deleted files are found and guessed

#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include <new>
#include <algorithm>
#include <set>
#include <string>
#include "FATDrive.h"
#include "FileContents.h"

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_LONG_NAME 0x0F // read only + hidden + system + volume id: a long file name entry

#define DELETED_ENTRY 0xE5
#define MAX_LFN_ENTRIES 20 // 255 characters, 13 per entry
#define MAX_DIRECTORY_SIZE (65536 * 32) // a FAT directory has at most 65536 entries
#define MAX_FAT_SIZE (512 * 1024 * 1024) // FAT32 of a 2 TB volume with 32 KB clusters: 256 MB
#define COPY_CHUNK_SIZE (1024 * 1024)
#define MAX_DELETED_DIRECTORY_SEARCH (64 * 1024 * 1024) // how far past its first cluster a deleted folder's next ones are looked for

// how far the cluster guess may go past clusters in use before it gives up: as many as the file has, at least
// this many - a file fragmented around a big file that still exists is out of reach anyway
#define MIN_CLUSTERS_TO_SKIP 4096

CFATDrive::CFATDrive() {
    m_bInitialized = false;
    m_iFATType = FAT_TYPE_NONE;
    m_dwBytesPerSector = 0;
    m_dwBytesPerCluster = 0;
    m_llFATOffset = 0;
    m_dwFATBytes = 0;
    m_dwNumberOfFATs = 0;
    m_dwActiveFAT = 0;
    m_llRootDirOffset = 0;
    m_dwRootDirBytes = 0;
    m_dwRootCluster = 0;
    m_llDataOffset = 0;
    m_dwMaxCluster = 0;
    m_llProgressPending = 0;
}

CFATDrive::~CFATDrive() {
}

const char *CFATDrive::FileSystemName() const {
    switch (m_iFATType) {
        case FAT_TYPE_12: return "FAT12";
        case FAT_TYPE_16: return "FAT16";
        case FAT_TYPE_32: return "FAT32";
        default: return "FAT";
    }
}

void CFATDrive::SetDriveHandle(HANDLE hDrive) {
    CloseVolume();
    m_hDrive = hDrive;
    m_bInitialized = false;
}

int CFATDrive::ReadVolume(wxString wxsSelectedDrive) {
    m_bInitialized = false;
    int nRet = OpenVolume(wxsSelectedDrive);
    if (nRet)
        return nRet;

    nRet = Initialize();
    if (nRet)
        wxLogMessage("%s: no FAT volume, or unreadable (error %d).", wxsSelectedDrive, nRet);
    return nRet;
}

// dwLen bytes from byte llOffset of the volume - both a whole number of sectors

int CFATDrive::ReadBytes(LONGLONG llOffset, BYTE *puchOut, DWORD dwLen) {
    LARGE_INTEGER pos;
    pos.QuadPart = llOffset;
    if (!SetFilePointerEx(m_hDrive, pos, NULL, FILE_BEGIN))
        return GetLastError();

    DWORD dwDone = 0;
    while (dwDone < dwLen) {
        DWORD dwBytes = 0;
        if (!ReadFile(m_hDrive, puchOut + dwDone, dwLen - dwDone, &dwBytes, NULL))
            return GetLastError();
        if (!dwBytes)
            return ERROR_HANDLE_EOF;
        dwDone += dwBytes;
    }
    return ERROR_SUCCESS;
}

int CFATDrive::ReadClusters(DWORD dwCluster, DWORD dwCount, BYTE *puchOut) {
    if (dwCluster < 2 || dwCluster > m_dwMaxCluster || dwCount > m_dwMaxCluster - dwCluster + 1)
        return ERROR_SECTOR_NOT_FOUND;
    return ReadBytes(m_llDataOffset + (LONGLONG) (dwCluster - 2) * m_dwBytesPerCluster, puchOut, dwCount * m_dwBytesPerCluster);
}

// the boot sector tells where everything is. Which FAT it is depends only on the number of clusters
// (Microsoft's FAT specification) - the "FAT32" label in the boot sector is only a label.

int CFATDrive::Initialize() {
    m_bInitialized = false;
    m_iFATType = FAT_TYPE_NONE;
    std::vector<ST_FATFILE>().swap(m_vFiles);
    std::vector<BYTE>().swap(m_vInUse);

    // 4 KB: a whole number of sectors for every sector size
    std::vector<BYTE> vBoot(4096);
    int nRet = ReadBytes(0, &vBoot[0], (DWORD) vBoot.size());
    if (nRet) {
        wxLogMessage("Cannot read the boot sector (error %d).", nRet);
        return nRet;
    }

    FAT_BOOT_SECTOR bs;
    memcpy(&bs, &vBoot[0], sizeof (bs));

    DWORD dwBytesPerSector = bs.wBytesPerSector;
    DWORD dwSectorsPerCluster = bs.uchSectorsPerCluster;
    if ((dwBytesPerSector != 512 && dwBytesPerSector != 1024 && dwBytesPerSector != 2048 && dwBytesPerSector != 4096)
            || !dwSectorsPerCluster || (dwSectorsPerCluster & (dwSectorsPerCluster - 1)) || dwSectorsPerCluster > 128
            || !bs.wReservedSectors || !bs.uchNumberOfFATs || bs.uchNumberOfFATs > 4)
        return ERROR_INVALID_DRIVE; // NTFS and exFAT have zeros here

    DWORD dwSectorsPerFAT = bs.wSectorsPerFAT16 ? bs.wSectorsPerFAT16 : bs.dwSectorsPerFAT32;
    DWORD dwTotalSectors = bs.wTotalSectors16 ? bs.wTotalSectors16 : bs.dwTotalSectors32;
    DWORD dwRootDirSectors = ((DWORD) bs.wRootEntries * 32 + dwBytesPerSector - 1) / dwBytesPerSector;
    ULONGLONG ullMetaSectors = bs.wReservedSectors + (ULONGLONG) bs.uchNumberOfFATs * dwSectorsPerFAT + dwRootDirSectors;
    if (!dwSectorsPerFAT || !dwTotalSectors || ullMetaSectors >= dwTotalSectors)
        return ERROR_INVALID_DRIVE;

    DWORD dwClusters = (DWORD) ((dwTotalSectors - ullMetaSectors) / dwSectorsPerCluster);
    if (dwClusters < 4085)
        m_iFATType = FAT_TYPE_12;
    else if (dwClusters < 65525)
        m_iFATType = FAT_TYPE_16;
    else
        m_iFATType = FAT_TYPE_32;

    if ((m_iFATType == FAT_TYPE_32) != (bs.wSectorsPerFAT16 == 0 && bs.wRootEntries == 0)) {
        wxLogMessage("The boot sector doesn't match a FAT volume of %u clusters.", (unsigned) dwClusters);
        m_iFATType = FAT_TYPE_NONE;
        return ERROR_INVALID_DRIVE;
    }

    m_dwBytesPerSector = dwBytesPerSector;
    m_dwBytesPerCluster = dwBytesPerSector * dwSectorsPerCluster;
    m_dwNumberOfFATs = bs.uchNumberOfFATs;
    m_llFATOffset = (LONGLONG) bs.wReservedSectors * dwBytesPerSector;
    if ((ULONGLONG) dwSectorsPerFAT * dwBytesPerSector > MAX_FAT_SIZE) {
        wxLogMessage("The FAT is too large (%u sectors).", (unsigned) dwSectorsPerFAT);
        m_iFATType = FAT_TYPE_NONE;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    m_dwFATBytes = dwSectorsPerFAT * dwBytesPerSector;
    m_llRootDirOffset = m_llFATOffset + (LONGLONG) m_dwNumberOfFATs * m_dwFATBytes;
    m_dwRootDirBytes = dwRootDirSectors * dwBytesPerSector;
    m_llDataOffset = m_llRootDirOffset + m_dwRootDirBytes;
    m_dwRootCluster = (m_iFATType == FAT_TYPE_32) ? bs.dwRootCluster : 0;
    m_dwActiveFAT = (m_iFATType == FAT_TYPE_32 && (bs.wExtFlags & 0x80)) ? (bs.wExtFlags & 0x0F) : 0;
    if (m_dwActiveFAT >= m_dwNumberOfFATs)
        m_dwActiveFAT = 0;

    // the clusters are 2..dwClusters+1 - as far as the FAT has entries for them
    ULONGLONG ullEntries = (m_iFATType == FAT_TYPE_12) ? (ULONGLONG) m_dwFATBytes * 2 / 3
            : (ULONGLONG) m_dwFATBytes / (m_iFATType == FAT_TYPE_16 ? 2 : 4);
    m_dwMaxCluster = dwClusters + 1;
    if (m_dwMaxCluster > ullEntries - 1)
        m_dwMaxCluster = (DWORD) (ullEntries - 1);
    if (m_iFATType == FAT_TYPE_32 && m_dwMaxCluster > 0x0FFFFFF6)
        m_dwMaxCluster = 0x0FFFFFF6;

    wxLogMessage("%s volume: %u clusters of %u bytes, %u FATs of %u bytes.", FileSystemName(), (unsigned) (m_dwMaxCluster - 1),
            (unsigned) m_dwBytesPerCluster, (unsigned) m_dwNumberOfFATs, (unsigned) m_dwFATBytes);
    m_bInitialized = true;
    return ERROR_SUCCESS;
}

// The FAT, decoded to one entry per cluster, and from it which clusters are in use. Read once at the start
// of a scan - recovery uses the same snapshot, so the clusters a file is recovered from are the ones its
// recoverability was judged by.

int CFATDrive::LoadFAT() {
    std::vector<DWORD>().swap(m_vFAT);
    std::vector<BYTE>().swap(m_vInUse);

    std::vector<BYTE> vRaw;
    try {
        vRaw.resize(m_dwFATBytes);
        m_vFAT.resize((size_t) m_dwMaxCluster + 1);
        m_vInUse.resize((size_t) m_dwMaxCluster / 8 + 1);
    } catch (std::bad_alloc &) {
        wxLogMessage("Not enough memory for the FAT (%u bytes).", (unsigned) m_dwFATBytes);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // the FAT in use: the first one, unless FAT32 mirroring is off and another one is active. If that one
    // can't be read, try the copies.
    int nRet = ERROR_READ_FAULT;
    for (DWORD i = 0; i < m_dwNumberOfFATs && nRet; i++) {
        DWORD dwFAT = (m_dwActiveFAT + i) % m_dwNumberOfFATs;
        nRet = ReadBytes(m_llFATOffset + (LONGLONG) dwFAT * m_dwFATBytes, &vRaw[0], m_dwFATBytes);
        if (nRet)
            wxLogMessage("Cannot read FAT #%u (error %d).", (unsigned) dwFAT + 1, nRet);
    }
    if (nRet)
        return nRet;

    const BYTE *p = &vRaw[0];
    for (DWORD c = 0; c <= m_dwMaxCluster; c++) {
        DWORD dwValue;
        if (m_iFATType == FAT_TYPE_12) {
            WORD w = (WORD) (p[c + c / 2] | (p[c + c / 2 + 1] << 8));
            dwValue = (c & 1) ? (w >> 4) : (w & 0x0FFF);
        } else if (m_iFATType == FAT_TYPE_16) {
            dwValue = p[c * 2] | (p[c * 2 + 1] << 8);
        } else {
            dwValue = (p[c * 4] | (p[c * 4 + 1] << 8) | (p[c * 4 + 2] << 16) | ((DWORD) p[c * 4 + 3] << 24)) & 0x0FFFFFFF;
        }
        m_vFAT[c] = dwValue;
        if (dwValue || c < 2) // clusters 0 and 1 don't exist - count them as used
            m_vInUse[c >> 3] |= (BYTE) (1 << (c & 7));
    }
    return ERROR_SUCCESS;
}

bool CFATDrive::IsEndOfChain(DWORD dwValue) const {
    switch (m_iFATType) {
        case FAT_TYPE_12: return dwValue >= 0xFF8;
        case FAT_TYPE_16: return dwValue >= 0xFFF8;
        default: return dwValue >= 0x0FFFFFF8;
    }
}

DWORD CFATDrive::NextCluster(DWORD dwCluster) const {
    if (dwCluster < 2 || dwCluster > m_dwMaxCluster || m_vFAT.empty())
        return 0;
    DWORD dwNext = m_vFAT[dwCluster];
    if (IsEndOfChain(dwNext) || dwNext < 2 || dwNext > m_dwMaxCluster)
        return 0; // end of the chain - or a free, bad or impossible entry, which ends it too
    return dwNext;
}

static void AddCluster(std::vector<CFATDrive::FAT_RUN> &vRuns, DWORD dwCluster) {
    if (!vRuns.empty() && vRuns.back().dwCluster + vRuns.back().dwCount == dwCluster) {
        vRuns.back().dwCount++;
    } else {
        CFATDrive::FAT_RUN run = {dwCluster, 1};
        vRuns.push_back(run);
    }
}

bool CFATDrive::GuessClusters(DWORD dwFirstCluster, DWORD dwSize, std::vector<FAT_RUN> &vRuns, t_FileInfo *pFileInfo) const {
    vRuns.clear();
    if (pFileInfo) {
        pFileInfo->iRecoverability = RECOVERABILITY_UNKNOWN;
        pFileInfo->iPercentIntact = 0;
        pFileInfo->bStartReused = false;
    }

    if (!dwSize) { // an empty file has no clusters - there is nothing to lose
        if (pFileInfo) {
            pFileInfo->iRecoverability = RECOVERABILITY_GOOD;
            pFileInfo->iPercentIntact = 100;
        }
        return true;
    }
    if (dwFirstCluster < 2 || dwFirstCluster > m_dwMaxCluster || m_vInUse.empty())
        return false;

    DWORD dwNeeded = (DWORD) (((ULONGLONG) dwSize + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster);
    DWORD dwMaxSkipped = (dwNeeded > MIN_CLUSTERS_TO_SKIP) ? dwNeeded : MIN_CLUSTERS_TO_SKIP;
    DWORD dwTaken = 0, dwSkipped = 0;
    // the first cluster is the only one known for sure. Once another file has it, the clusters after it
    // were most likely taken the same way - skipping to free ones further on would only collect other data.
    bool bStartReused = IsClusterInUse(dwFirstCluster);

    // take the free clusters from the first one on, skipping the ones in use
    for (DWORD c = dwFirstCluster; !bStartReused && dwTaken < dwNeeded && c <= m_dwMaxCluster && dwSkipped <= dwMaxSkipped;) {
        if (!(c & 7) && m_vInUse[c >> 3] == 0xFF && c + 7 <= m_dwMaxCluster) { // 8 clusters in use at once
            dwSkipped += 8;
            c += 8;
        } else if (IsClusterInUse(c)) {
            dwSkipped++;
            c++;
        } else {
            AddCluster(vRuns, c);
            dwTaken++;
            c++;
        }
    }

    if (dwTaken == dwNeeded) {
        if (pFileInfo) {
            if (!dwSkipped) {
                pFileInfo->iRecoverability = RECOVERABILITY_GOOD;
                pFileInfo->iPercentIntact = 100;
            } else {
                // clusters in use between them: either the file was fragmented around another file (then it is
                // intact), or another file has taken part of it (then a part is gone) - there is no telling which
                int iPercent = (int) ((ULONGLONG) dwNeeded * 100 / ((ULONGLONG) dwNeeded + dwSkipped));
                pFileInfo->iRecoverability = RECOVERABILITY_PARTIAL;
                pFileInfo->iPercentIntact = (iPercent < 1) ? 1 : (iPercent > 99 ? 99 : iPercent);
            }
        }
        return true;
    }

    // the first cluster is reused, or there aren't enough free clusters within reach: the file's clusters have
    // mostly been taken by other files. Recover the clusters from the first one on, like a contiguous file,
    // and say how many are in use.
    vRuns.clear();
    if (dwNeeded > m_dwMaxCluster - dwFirstCluster + 1)
        return false; // the file would go past the end of the volume - the entry can't be right
    FAT_RUN run = {dwFirstCluster, dwNeeded};
    vRuns.push_back(run);

    if (pFileInfo) {
        DWORD dwFree = 0;
        for (DWORD c = dwFirstCluster; c < dwFirstCluster + dwNeeded; c++)
            if (!IsClusterInUse(c))
                dwFree++;
        pFileInfo->bStartReused = bStartReused;
        pFileInfo->iPercentIntact = (int) ((ULONGLONG) dwFree * 100 / dwNeeded);
        if (dwFree && dwFree < dwNeeded)
            pFileInfo->iPercentIntact = (pFileInfo->iPercentIntact < 1) ? 1 : (pFileInfo->iPercentIntact > 99 ? 99 : pFileInfo->iPercentIntact);
        pFileInfo->iRecoverability = (dwFree == dwNeeded) ? RECOVERABILITY_GOOD
                : (dwFree ? RECOVERABILITY_PARTIAL : RECOVERABILITY_OVERWRITTEN);
    }
    return true;
}

// the GUI thread shows how much of the selection has been written - counted in KB, like CMFTRecord does

void CFATDrive::AddProgress(DWORD dwBytes) {
    if (!m_plProgressKB)
        return;
    m_llProgressPending += dwBytes;
    LONG lKB = (LONG) (m_llProgressPending >> 10);
    if (lKB) {
        InterlockedExchangeAdd((LONG *) m_plProgressKB, lKB);
        m_llProgressPending -= (LONGLONG) lKB << 10;
    }
}

int CFATDrive::ReadRuns(const std::vector<FAT_RUN> &vRuns, DWORD dwLen, BYTE *puchOut, HANDLE hFile) {
    DWORD dwChunkClusters = COPY_CHUNK_SIZE / m_dwBytesPerCluster;
    if (!dwChunkClusters)
        dwChunkClusters = 1;

    std::vector<BYTE> vBuffer;
    try {
        vBuffer.resize((size_t) dwChunkClusters * m_dwBytesPerCluster);
    } catch (std::bad_alloc &) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for (size_t i = 0; i < vRuns.size() && dwLen; i++) {
        for (DWORD dwDone = 0; dwDone < vRuns[i].dwCount && dwLen;) {
            if (IsCancelled())
                return ERROR_CANCELLED;

            DWORD dwCount = vRuns[i].dwCount - dwDone;
            if (dwCount > dwChunkClusters)
                dwCount = dwChunkClusters;
            // the last chunk of the file only needs part of its clusters
            ULONGLONG ullChunkBytes = (ULONGLONG) dwCount * m_dwBytesPerCluster;
            DWORD dwUseful = (dwLen < ullChunkBytes) ? dwLen : (DWORD) ullChunkBytes;
            dwCount = (dwUseful + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster;

            int nRet = ReadClusters(vRuns[i].dwCluster + dwDone, dwCount, &vBuffer[0]);
            if (nRet)
                return nRet;

            if (hFile) {
                DWORD dwWritten = 0;
                if (!WriteFile(hFile, &vBuffer[0], dwUseful, &dwWritten, NULL) || dwWritten != dwUseful) {
                    DWORD dwErrorCode = GetLastError();
                    wxLogMessage("WriteFile returned error code %u . Possibly not enough drive space", (unsigned) dwErrorCode);
                    return dwErrorCode ? (int) dwErrorCode : ERROR_WRITE_FAULT;
                }
                AddProgress(dwUseful);
            } else {
                memcpy(puchOut, &vBuffer[0], dwUseful);
                puchOut += dwUseful;
            }
            dwLen -= dwUseful;
            dwDone += dwCount;
        }
    }
    return dwLen ? ERROR_HANDLE_EOF : ERROR_SUCCESS;
}

int CFATDrive::Read_File(DWORD dwFileId, BYTE *&puchFileData, DWORD &dwFileDataLen) {
    if (!m_bInitialized || dwFileId >= m_vFiles.size())
        return ERROR_INVALID_ACCESS;

    const ST_FATFILE &file = m_vFiles[dwFileId];
    std::vector<FAT_RUN> vRuns;
    if (!GuessClusters(file.dwFirstCluster, file.dwSize, vRuns, NULL))
        return ERROR_FILE_CORRUPT;

    if (file.dwSize > 0x7FFFFFFF)
        return ERROR_NOT_ENOUGH_MEMORY;
    BYTE *puchData = new (std::nothrow) BYTE[file.dwSize ? file.dwSize : 1];
    if (!puchData)
        return ERROR_NOT_ENOUGH_MEMORY;

    int nRet = ReadRuns(vRuns, file.dwSize, puchData, NULL);
    if (nRet) {
        delete[] puchData;
        return nRet;
    }
    puchFileData = puchData;
    dwFileDataLen = file.dwSize;
    return ERROR_SUCCESS;
}

int CFATDrive::ReadFileStart(DWORD dwFileId, std::vector<BYTE> &vData, DWORD dwMax) {
    vData.clear();
    if (!m_bInitialized || dwFileId >= m_vFiles.size())
        return ERROR_INVALID_ACCESS;

    const ST_FATFILE &file = m_vFiles[dwFileId];
    std::vector<FAT_RUN> vRuns;
    if (!GuessClusters(file.dwFirstCluster, file.dwSize, vRuns, NULL))
        return ERROR_FILE_CORRUPT;

    DWORD dwWant = (file.dwSize < dwMax) ? file.dwSize : dwMax;
    if (!dwWant)
        return ERROR_SUCCESS;
    vData.resize(dwWant);
    int nRet = ReadRuns(vRuns, dwWant, &vData[0], NULL);
    if (nRet)
        vData.clear();
    return nRet;
}

int CFATDrive::RetreiveAndWriteFileToDestination(int iFileId, const wchar_t *wszDestinationFileName) {
    wxString sDestinationFileName(wszDestinationFileName); // for logging

    if (!m_bInitialized || iFileId < 0 || (size_t) iFileId >= m_vFiles.size()) {
        wxLogMessage("Invalid Access during retrieving \'%s\'", sDestinationFileName);
        return ERROR_INVALID_ACCESS;
    }

    const ST_FATFILE &file = m_vFiles[iFileId];
    std::vector<FAT_RUN> vRuns;
    if (!GuessClusters(file.dwFirstCluster, file.dwSize, vRuns, NULL)) {
        wxLogMessage("\'%s\': its first cluster (%u) can't be right.", sDestinationFileName, (unsigned) file.dwFirstCluster);
        return ERROR_FILE_CORRUPT;
    }

    HANDLE hFile = CreateFileW(LongPathForWindows(sDestinationFileName).wc_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        // one file that can't be created must not stop the others - the copy thread logs it and goes on
        DWORD dwErrorCode = GetLastError();
        wxLogMessage("Unable to create file \"%s\" (error %u).", sDestinationFileName, (unsigned) dwErrorCode);
        return dwErrorCode ? (int) dwErrorCode : ERROR_CANNOT_MAKE;
    }

    wxLogMessage("Extracting file \'%s\' (%u bytes in %u fragments).", sDestinationFileName, (unsigned) file.dwSize, (unsigned) vRuns.size());
    m_llProgressPending = 0;
    int nRet = ReadRuns(vRuns, file.dwSize, NULL, hFile);
    CloseHandle(hFile);
    if (nRet) {
        wxLogMessage("Extracting file \'%s\' FAILED (error %d)!!!", sDestinationFileName, nRet);
        if (nRet == ERROR_CANCELLED)
            DeleteFileW(LongPathForWindows(sDestinationFileName).wc_str()); // a cut-off file would look like a recovered one
        return nRet;
    }
    wxLogMessage("Extracted a file.");
    return ERROR_SUCCESS;
}

/////////////////////////////////// directories ///////////////////////////////////

// can this byte be in an 8.3 name? (lower case letters can't, but some tools write them anyway)

static bool IsShortNameChar(BYTE uch) {
    if (uch < 0x20)
        return false;
    return !strchr("\"*+,./:;<=>?[\\]|", uch);
}

static bool IsDotEntry(const FAT_DIR_ENTRY &e) {
    return !memcmp(e.uchName, ".          ", 11) || !memcmp(e.uchName, "..         ", 11);
}

// does this look like directory entries? Used for clusters of deleted folders, which may hold anything by now -
// strict, because a file's data must not pass: every entry valid, and after the end of the directory only zeros
// (the rest of a folder's last cluster is zeroed when the folder gets it)

static bool LooksLikeDirectory(const BYTE *puchData, DWORD dwLen) {
    for (DWORD i = 0; i + 32 <= dwLen; i += 32) {
        const FAT_DIR_ENTRY &e = *(const FAT_DIR_ENTRY *) (puchData + i);
        if (!e.uchName[0]) { // the end of the directory
            for (DWORD j = i; j < dwLen; j++)
                if (puchData[j])
                    return false;
            return true;
        }
        if (e.uchAttributes == ATTR_LONG_NAME) {
            const FAT_LFN_ENTRY &lfn = *(const FAT_LFN_ENTRY *) (puchData + i);
            if (lfn.uchType || lfn.wFirstCluster)
                return false;
            continue;
        }
        if (e.uchAttributes & 0xC0)
            return false; // bits no FAT entry has
        if (IsDotEntry(e))
            continue;
        if (e.uchName[0] != DELETED_ENTRY && e.uchName[0] != 0x05 && !IsShortNameChar(e.uchName[0]))
            return false;
        for (int j = 1; j < 11; j++)
            if (!IsShortNameChar(e.uchName[j]))
                return false;
    }
    return true;
}

static bool HasEndOfDirectory(const BYTE *puchData, DWORD dwLen) {
    for (DWORD i = 0; i + 32 <= dwLen; i += 32)
        if (!puchData[i])
            return true;
    return false;
}

int CFATDrive::ReadDirectory(DWORD dwCluster, bool bDeleted, std::vector<BYTE> &vData, std::set<DWORD> &setClaimed) {
    vData.clear();
    if (!dwCluster) { // the fixed root directory of FAT12/16
        vData.resize(m_dwRootDirBytes);
        return m_dwRootDirBytes ? ReadBytes(m_llRootDirOffset, &vData[0], m_dwRootDirBytes) : ERROR_SUCCESS;
    }

    DWORD dwMaxClusters = MAX_DIRECTORY_SIZE / m_dwBytesPerCluster;
    if (!dwMaxClusters)
        dwMaxClusters = 1;

    if (!bDeleted) {
        // follow the chain, reading consecutive clusters at once
        for (DWORD c = dwCluster, dwRead = 0; c && dwRead < dwMaxClusters;) {
            DWORD dwCount = 1, dwLast = c, dwNext;
            while ((dwNext = NextCluster(dwLast)) == dwLast + 1 && dwRead + dwCount < dwMaxClusters) {
                dwLast = dwNext;
                dwCount++;
            }
            size_t nOld = vData.size();
            vData.resize(nOld + (size_t) dwCount * m_dwBytesPerCluster);
            int nRet = ReadClusters(c, dwCount, &vData[nOld]);
            if (nRet) {
                vData.resize(nOld);
                return vData.empty() ? nRet : ERROR_SUCCESS; // what could be read
            }
            dwRead += dwCount;
            c = NextCluster(dwLast);
        }
        return ERROR_SUCCESS;
    }

    // A deleted folder: its chain is gone, only its first cluster is known. It must still be free and still
    // start with the "." entry - else it holds something else by now.
    if (IsClusterInUse(dwCluster))
        return ERROR_FILE_CORRUPT;
    vData.resize(m_dwBytesPerCluster);
    int nRet = ReadClusters(dwCluster, 1, &vData[0]);
    if (nRet || !IsDotEntry(*(const FAT_DIR_ENTRY *) &vData[0]) || !LooksLikeDirectory(&vData[0], m_dwBytesPerCluster)) {
        vData.clear();
        return nRet ? nRet : ERROR_FILE_CORRUPT;
    }
    if (HasEndOfDirectory(&vData[0], m_dwBytesPerCluster))
        return ERROR_SUCCESS;

    // It grew: a folder gets its next cluster when it fills up - from the free space of that moment, usually
    // after the data of the files it held by then. That data is free now too, so look further on for free
    // clusters that hold directory entries, until the one with the end of the folder. Not an empty one (a
    // folder's next cluster starts with an entry), not one starting another folder (its "." entry), not one
    // another folder already took. The data in between can be large - only so far, read in big pieces.
    DWORD dwSearch = MAX_DELETED_DIRECTORY_SEARCH / m_dwBytesPerCluster;
    DWORD dwChunk = wxMax((DWORD) 1, (DWORD) (COPY_CHUNK_SIZE / m_dwBytesPerCluster));
    std::vector<BYTE> vChunk((size_t) dwChunk * m_dwBytesPerCluster);
    DWORD dwRead = 1;
    for (DWORD c = dwCluster + 1; c <= m_dwMaxCluster && c - dwCluster <= dwSearch && dwRead < dwMaxClusters;) {
        if (IsClusterInUse(c) || setClaimed.count(c)) {
            c++;
            continue;
        }
        // the free clusters from here on, in one read
        DWORD dwCount = 1;
        while (dwCount < dwChunk && c + dwCount <= m_dwMaxCluster && !IsClusterInUse(c + dwCount) && !setClaimed.count(c + dwCount))
            dwCount++;
        if (ReadClusters(c, dwCount, &vChunk[0]))
            break; // what was found so far
        for (DWORD i = 0; i < dwCount; i++) {
            const BYTE *puchCluster = &vChunk[(size_t) i * m_dwBytesPerCluster];
            if (!puchCluster[0] || IsDotEntry(*(const FAT_DIR_ENTRY *) puchCluster) || !LooksLikeDirectory(puchCluster, m_dwBytesPerCluster))
                continue;
            setClaimed.insert(c + i);
            vData.insert(vData.end(), puchCluster, puchCluster + m_dwBytesPerCluster);
            if (HasEndOfDirectory(puchCluster, m_dwBytesPerCluster) || ++dwRead >= dwMaxClusters)
                return ERROR_SUCCESS;
        }
        c += dwCount;
    }
    return ERROR_SUCCESS;
}

static BYTE ShortNameChecksum(const BYTE *puchName) {
    BYTE uchSum = 0;
    for (int i = 0; i < 11; i++)
        uchSum = (BYTE) (((uchSum & 1) << 7) + (uchSum >> 1) + puchName[i]);
    return uchSum;
}

// "README  TXT" -> "README.TXT" (or "readme.txt" with the lower case flags Windows sets instead of a long name)

static std::wstring ShortNameText(const BYTE *puchName, BYTE uchNTCase) {
    char szBase[9], szExt[4];
    memcpy(szBase, puchName, 8);
    szBase[8] = 0;
    memcpy(szExt, puchName + 8, 3);
    szExt[3] = 0;
    if ((BYTE) szBase[0] == 0x05)
        szBase[0] = (char) 0xE5; // a name really starting with 0xE5 (a Japanese lead byte) is stored as 0x05
    for (int i = 7; i >= 0 && szBase[i] == ' '; i--)
        szBase[i] = 0;
    for (int i = 2; i >= 0 && szExt[i] == ' '; i--)
        szExt[i] = 0;
    if (uchNTCase & 0x08)
        CharLowerBuffA(szBase, (DWORD) strlen(szBase));
    if (uchNTCase & 0x10)
        CharLowerBuffA(szExt, (DWORD) strlen(szExt));

    std::string sName(szBase);
    if (szExt[0])
        sName += std::string(".") + szExt;

    // 8.3 names are in the OEM code page
    wchar_t wszName[16];
    int iLen = MultiByteToWideChar(CP_OEMCP, 0, sName.c_str(), (int) sName.length(), wszName, 15);
    return std::wstring(wszName, iLen > 0 ? iLen : 0);
}

static bool IsLongNameChar(wchar_t wch) {
    return wch >= 0x20 && !wcschr(L"\"*/:<>?\\|", wch);
}

// The long file name of the 8.3 entry at iEntry: the long name entries right before it, nearest first.
// Of a deleted entry, the first byte of the 8.3 name is gone, and the order numbers of the long name entries
// too. The entries still carry the checksum of the whole 8.3 name, though - and only one first byte gives
// that checksum, which brings the 8.3 name back as well. puchFirstByte gets it (0 if there is no long name).

static std::wstring LongNameOf(const FAT_DIR_ENTRY *pEntries, int iEntry, bool bDeleted, BYTE *puchFirstByte) {
    *puchFirstByte = 0;
    const FAT_DIR_ENTRY &e = pEntries[iEntry];

    std::wstring sName;
    int iFound = 0;
    BYTE uchChecksum = 0;
    bool bEnded = false;
    for (int j = iEntry - 1; j >= 0 && iFound < MAX_LFN_ENTRIES && !bEnded; j--) {
        const FAT_LFN_ENTRY &lfn = *(const FAT_LFN_ENTRY *) &pEntries[j];
        if (lfn.uchAttributes != ATTR_LONG_NAME)
            break;
        if (bDeleted ? (lfn.uchOrder != DELETED_ENTRY) : ((lfn.uchOrder & 0x3F) != iFound + 1))
            break;
        if (iFound && lfn.uchChecksum != uchChecksum)
            break;
        uchChecksum = lfn.uchChecksum;

        WORD wChars[13];
        memcpy(wChars, lfn.wName1, sizeof (lfn.wName1));
        memcpy(wChars + 5, lfn.wName2, sizeof (lfn.wName2));
        memcpy(wChars + 11, lfn.wName3, sizeof (lfn.wName3));
        for (int k = 0; k < 13; k++) {
            if (wChars[k] == 0x0000 || wChars[k] == 0xFFFF) {
                bEnded = true;
                break;
            }
            sName += (wchar_t) wChars[k];
        }
        iFound++;
        if (!bDeleted && (lfn.uchOrder & 0x40))
            break; // the last one
    }
    if (!iFound || sName.empty())
        return std::wstring();

    BYTE uchName[11];
    memcpy(uchName, e.uchName, 11);
    if (bDeleted) {
        int b;
        for (b = 0; b < 256; b++) {
            uchName[0] = (BYTE) b;
            if (ShortNameChecksum(uchName) == uchChecksum)
                break;
        }
        if (b == 256 || (b != 0x05 && !IsShortNameChar((BYTE) b)))
            return std::wstring(); // these long name entries belonged to something else
    } else if (ShortNameChecksum(uchName) != uchChecksum) {
        return std::wstring();
    }

    for (size_t k = 0; k < sName.length(); k++)
        if (!IsLongNameChar(sName[k]))
            return std::wstring();

    *puchFirstByte = uchName[0];
    return sName;
}

// FAT dates and times are local time, in 2 second steps

static LONGLONG FATTimeToFileTime(WORD wDate, WORD wTime, BYTE uchTenth) {
    if (!wDate)
        return 0;
    SYSTEMTIME st = {0}, stUtc;
    st.wYear = (WORD) (1980 + (wDate >> 9));
    st.wMonth = (WORD) ((wDate >> 5) & 0x0F);
    st.wDay = (WORD) (wDate & 0x1F);
    st.wHour = (WORD) (wTime >> 11);
    st.wMinute = (WORD) ((wTime >> 5) & 0x3F);
    st.wSecond = (WORD) ((wTime & 0x1F) * 2);
    if (uchTenth < 200) {
        st.wSecond = (WORD) (st.wSecond + uchTenth / 100);
        st.wMilliseconds = (WORD) ((uchTenth % 100) * 10);
    }
    FILETIME ft;
    if (!TzSpecificLocalTimeToSystemTime(NULL, &st, &stUtc) || !SystemTimeToFileTime(&stUtc, &ft))
        return 0;
    return ((LONGLONG) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

static std::wstring ChildPath(const std::wstring &sParent, const std::wstring &sName) {
    return (sParent == L"\\") ? sParent + sName : sParent + L"\\" + sName;
}

// how many candidate clusters are read for the exact check - on a 2 TB FAT32 volume there can be 1000 of them
#define MAX_CANDIDATES_READ 16

DWORD CFATDrive::ResolveClearedHighWordExactly(WORD wLow, bool bDirectory, DWORD dwSize, const std::wstring &sName) {
    // only a place that is still free can be read for it - in use, it holds another file's data by now
    std::vector<DWORD> vCandidates;
    for (DWORD c = wLow; c <= m_dwMaxCluster && vCandidates.size() < MAX_CANDIDATES_READ; c += 0x10000)
        if (c >= 2 && !IsClusterInUse(c))
            vCandidates.push_back(c);

    if (bDirectory) {
        // a folder's first cluster starts with its "." entry, which holds the folder's own cluster number
        std::vector<BYTE> vData(m_dwBytesPerCluster);
        for (size_t i = 0; i < vCandidates.size(); i++) {
            if (ReadClusters(vCandidates[i], 1, &vData[0]))
                continue;
            const FAT_DIR_ENTRY &dot = *(const FAT_DIR_ENTRY *) &vData[0];
            if (!memcmp(dot.uchName, ".          ", 11)
                    && ((DWORD) dot.wFirstClusterHigh << 16 | dot.wFirstClusterLow) == vCandidates[i])
                return vCandidates[i];
        }
        return 0;
    }

    // a file whose start shows its type: the place where it passes the content check - if only one does
    if (!dwSize || !CanCheckFileContents(sName.c_str()))
        return 0;
    DWORD dwStart = wxMin(dwSize, (DWORD) CONTENT_CHECK_BYTES);
    std::vector<BYTE> vStart(dwStart);
    DWORD dwFound = 0;
    for (size_t i = 0; i < vCandidates.size(); i++) {
        std::vector<FAT_RUN> vRuns;
        if (!GuessClusters(vCandidates[i], dwSize, vRuns, NULL) || ReadRuns(vRuns, dwStart, &vStart[0], NULL))
            continue;
        if (ClassifyFileContents(sName.c_str(), &vStart[0], dwStart) == CONTENT_OK) {
            if (dwFound)
                return 0; // two places look like it (two copies of a photo?) - leave it to the neighbours
            dwFound = vCandidates[i];
        }
    }
    return dwFound;
}

DWORD CFATDrive::ResolveClearedHighWordNear(WORD wLow, DWORD dwExpected) const {
    // the place nearest to where the file is expected - in use or not: a file whose place is in use now was
    // overwritten, and a free place further away must not be taken for it just because it is free
    DWORD dwBest = 0, dBest = 0xFFFFFFFF;
    for (DWORD c = wLow; c <= m_dwMaxCluster; c += 0x10000) {
        DWORD d = c > dwExpected ? c - dwExpected : dwExpected - c;
        if (c >= 2 && d < dBest) {
            dBest = d;
            dwBest = c;
        }
    }
    return dwBest;
}

void CFATDrive::ScanDirectory(const ST_DIR_TO_SCAN &dir, std::vector<ST_DIR_TO_SCAN> &vToScan, std::set<DWORD> &setSeen,
        FileInfoHash *finfohash) {
    std::vector<BYTE> vData;
    int nRet = ReadDirectory(dir.dwCluster, dir.bDeleted, vData, setSeen);
    if (nRet) {
        if (!dir.bDeleted)
            wxLogMessage("Cannot read the folder %s (error %d).", wxString(dir.sPath.c_str()), nRet);
        return;
    }

    const FAT_DIR_ENTRY *pEntries = (const FAT_DIR_ENTRY *) (vData.empty() ? NULL : &vData[0]);
    int iEntries = (int) (vData.size() / sizeof (FAT_DIR_ENTRY));
    // the Recycle Bin's files are all named $R... and $I... - a lost first character there is a "$"
    std::wstring sUpperPath(dir.sPath);
    CharUpperBuffW(&sUpperPath[0], (DWORD) sUpperPath.length());
    bool bInRecycleBin = sUpperPath.find(L"\\$RECYCLE.BIN") != std::wstring::npos;

    // the folder's entries, in their order - their clusters are settled before any is used (see
    // ResolveClearedHighWordExactly in FATDrive.h)
    struct ST_ENTRY {
        int iEntry;
        std::wstring sName;
        DWORD dwCluster;
        bool bDeleted;
        bool bKnown; // the cluster is known for sure - the others lie between their nearest known neighbours
    };
    std::vector<ST_ENTRY> vEntries;

    for (int i = 0; i < iEntries; i++) {
        const FAT_DIR_ENTRY &e = pEntries[i];
        if (!e.uchName[0])
            break; // the end of the directory
        if (e.uchAttributes == ATTR_LONG_NAME || (e.uchAttributes & ATTR_VOLUME_ID) || IsDotEntry(e))
            continue;

        // in a deleted folder, everything is deleted - whether its entries were marked or not
        bool bEntryDeleted = e.uchName[0] == DELETED_ENTRY;
        if ((e.uchAttributes & 0xC0) || (!bEntryDeleted && e.uchName[0] != 0x05 && !IsShortNameChar(e.uchName[0])))
            continue; // not a valid entry
        bool bValid = true;
        for (int j = 1; j < 11 && bValid; j++)
            bValid = IsShortNameChar(e.uchName[j]);
        if (!bValid)
            continue;

        BYTE uchFirstByte;
        std::wstring sName = LongNameOf(pEntries, i, bEntryDeleted, &uchFirstByte);
        if (sName.empty()) {
            BYTE uchName[11];
            memcpy(uchName, e.uchName, 11);
            if (bEntryDeleted)
                uchName[0] = bInRecycleBin ? '$' : '_'; // the first character is lost
            sName = ShortNameText(uchName, e.uchNTCase);
        }
        if (sName.empty())
            continue;

        ST_ENTRY entry = {i, sName, e.wFirstClusterLow, bEntryDeleted || dir.bDeleted, true};
        if (m_iFATType == FAT_TYPE_32)
            entry.dwCluster |= (DWORD) e.wFirstClusterHigh << 16;
        if (m_iFATType == FAT_TYPE_32 && bEntryDeleted && !e.wFirstClusterHigh && m_dwMaxCluster > 0xFFFF && entry.dwCluster) {
            // Windows cleared the high word when it deleted the entry - only the low word is left
            DWORD dwExact = ResolveClearedHighWordExactly(e.wFirstClusterLow, (e.uchAttributes & ATTR_DIRECTORY) != 0, e.dwFileSize, sName);
            if (dwExact)
                entry.dwCluster = dwExact;
            else if (e.uchAttributes & ATTR_DIRECTORY)
                entry.dwCluster = 0; // no free cluster holds it any more
            else
                entry.bKnown = false;
        }
        vEntries.push_back(entry);
    }

    // The others: files written one after another lie one after another, so a file starts about where the files
    // between it and its next known neighbour, counted back from that neighbour's start, begin - this also holds
    // where the volume was full and the allocation went on from its beginning. Without a known next neighbour,
    // counted on from where the previous known one ends. Off by less than 32768 clusters, the nearest place is right.
    std::vector<DWORD> vClusters(vEntries.size()); // the clusters each entry takes
    for (size_t k = 0; k < vEntries.size(); k++) {
        const FAT_DIR_ENTRY &e = pEntries[vEntries[k].iEntry];
        vClusters[k] = (e.uchAttributes & ATTR_DIRECTORY) ? 1 : (DWORD) (((ULONGLONG) e.dwFileSize + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster);
    }
    for (size_t k = 0; k < vEntries.size(); k++) {
        if (vEntries[k].bKnown)
            continue;
        DWORD dwExpected = 0;
        ULONGLONG ullBetween = 0;
        for (size_t n = k; n < vEntries.size(); n++) {
            if (vEntries[n].bKnown && vEntries[n].dwCluster >= 2 && vEntries[n].dwCluster <= m_dwMaxCluster) {
                if (ullBetween < vEntries[n].dwCluster)
                    dwExpected = (DWORD) (vEntries[n].dwCluster - ullBetween);
                break;
            }
            if (!vEntries[n].bKnown)
                ullBetween += vClusters[n];
        }
        if (!dwExpected) {
            ullBetween = 0;
            DWORD dwPrevEnd = dir.dwCluster ? dir.dwCluster + 1 : 2; // nothing before it is known: right after the folder
            for (size_t p = k; p-- > 0;) {
                if (vEntries[p].bKnown && vEntries[p].dwCluster >= 2 && vEntries[p].dwCluster <= m_dwMaxCluster) {
                    dwPrevEnd = vEntries[p].dwCluster + vClusters[p];
                    break;
                }
                if (!vEntries[p].bKnown)
                    ullBetween += vClusters[p];
            }
            dwExpected = (DWORD) wxMin((ULONGLONG) dwPrevEnd + ullBetween, (ULONGLONG) m_dwMaxCluster);
        }
        vEntries[k].dwCluster = ResolveClearedHighWordNear(pEntries[vEntries[k].iEntry].wFirstClusterLow, dwExpected);
        // a guess - the log says which place was taken (for a support case)
        wxLogMessage("%s: first cluster guessed as %u (expected near %u).", wxString(vEntries[k].sName.c_str()),
                (unsigned) vEntries[k].dwCluster, (unsigned) dwExpected);
    }

    for (size_t k = 0; k < vEntries.size(); k++) {
        const ST_ENTRY &entry = vEntries[k];
        const FAT_DIR_ENTRY &e = pEntries[entry.iEntry];
        DWORD dwCluster = entry.dwCluster;

        if (e.uchAttributes & ATTR_DIRECTORY) {
            // a deleted folder can only be read while its first cluster is still free
            if (dwCluster >= 2 && dwCluster <= m_dwMaxCluster && !setSeen.count(dwCluster)
                    && (!entry.bDeleted || !IsClusterInUse(dwCluster))) {
                setSeen.insert(dwCluster);
                ST_DIR_TO_SCAN sub = {dwCluster, entry.bDeleted, ChildPath(dir.sPath, entry.sName)};
                vToScan.push_back(sub);
            }
            continue;
        }

        if (!entry.bDeleted || (e.uchAttributes & (ATTR_SYSTEM | ATTR_HIDDEN)))
            continue; // like on NTFS: no system or hidden files

        t_FileInfo *pFileInfo = new t_FileInfo;
        pFileInfo->sFilename = entry.sName;
        pFileInfo->sFolder = dir.sPath;
        pFileInfo->bDeleted = true;
        pFileInfo->dwAttributes = e.uchAttributes & (ATTR_READ_ONLY | FILE_ATTRIBUTE_ARCHIVE);
        pFileInfo->n64Create = FATTimeToFileTime(e.wCreateDate, e.wCreateTime, e.uchCreateTimeTenth);
        pFileInfo->n64Modify = FATTimeToFileTime(e.wWriteDate, e.wWriteTime, 0);
        pFileInfo->n64Modfil = pFileInfo->n64Modify;
        pFileInfo->n64Access = FATTimeToFileTime(e.wAccessDate, 0, 0);
        pFileInfo->n64RealSize = e.dwFileSize;
        pFileInfo->n64SizeOnDisk = ((LONGLONG) e.dwFileSize + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster * m_dwBytesPerCluster;

        std::vector<FAT_RUN> vRuns;
        GuessClusters(dwCluster, e.dwFileSize, vRuns, pFileInfo);

        ST_FATFILE file = {dwCluster, e.dwFileSize};
        pFileInfo->iRecordId = (int) m_vFiles.size();
        m_vFiles.push_back(file);
        AddFoundFile(finfohash, pFileInfo);
    }
}

int CFATDrive::ScanFiles(FileInfoHash *finfohash) {
    if (!m_bInitialized)
        return ERROR_INVALID_ACCESS;

    std::vector<ST_FATFILE>().swap(m_vFiles);
    ReportScanProgress(-1); // how many folders there are only shows at the end
    int nRet = LoadFAT();
    if (nRet) {
        wxLogMessage("The FAT can't be read (error %d).", nRet);
        return nRet;
    }

    // the folders still to scan - a stack: the order doesn't matter, every folder carries its own path
    std::vector<ST_DIR_TO_SCAN> vToScan;
    std::set<DWORD> setSeen; // folder clusters already taken - a damaged or reused folder must not make a loop
    ST_DIR_TO_SCAN root = {m_dwRootCluster, false, L"\\"};
    vToScan.push_back(root);
    if (m_dwRootCluster)
        setSeen.insert(m_dwRootCluster);

    unsigned uFolders = 0;
    while (!vToScan.empty()) {
        if (IsCancelled()) {
            wxLogMessage("Scan cancelled after %u folders.", uFolders);
            std::vector<DWORD>().swap(m_vFAT);
            return ERROR_CANCELLED;
        }
        ST_DIR_TO_SCAN dir = vToScan.back();
        vToScan.pop_back();
        ScanDirectory(dir, vToScan, setSeen, finfohash);
        uFolders++;
    }

    wxLogMessage("No more files to add to the list (%u folders scanned).", uFolders);
    std::vector<DWORD>().swap(m_vFAT); // the chains are only needed to walk the folders - which clusters are in use stays
    return ERROR_SUCCESS; // the Recycle Bin names come in FinishScan
}
