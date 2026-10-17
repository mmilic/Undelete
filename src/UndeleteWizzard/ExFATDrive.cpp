// ExFATDrive.cpp: the exFAT recovery engine - see ExFATDrive.h for what deleting leaves behind and how it is used

#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include <new>
#include <set>
#include <string>
#include "ExFATDrive.h"

// directory entry types - the "in use" bit (0x80) is cleared when an entry is deleted
#define ENTRY_IN_USE 0x80
#define ENTRY_END_OF_DIRECTORY 0x00
#define ENTRY_BITMAP 0x81
#define ENTRY_FILE 0x85
#define ENTRY_STREAM 0xC0
#define ENTRY_NAME 0xC1

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

#define STREAM_ALLOCATION_POSSIBLE 0x01
#define STREAM_NO_FAT_CHAIN 0x02

#define FAT_END_OF_CHAIN 0xFFFFFFF8 // and up: 0xFFFFFFFF ends a chain, 0xFFFFFFF7 is a bad cluster
#define MAX_DIRECTORY_SIZE (256 * 1024 * 1024) // the exFAT limit
#define MAX_FAT_SIZE (512 * 1024 * 1024)
#define COPY_CHUNK_SIZE (1024 * 1024)
// how far the cluster guess may go past clusters in use before it gives up (like FATDrive.cpp)
#define MIN_CLUSTERS_TO_SKIP 4096

#pragma pack(push, 1)

struct EXFAT_BOOT_SECTOR {
    BYTE uchJump[3];
    char chFileSystemName[8]; // "EXFAT   "
    BYTE uchMustBeZero[53];
    ULONGLONG ullPartitionOffset;
    ULONGLONG ullVolumeLength; // sectors
    DWORD dwFATOffset; // sectors
    DWORD dwFATLength; // sectors
    DWORD dwClusterHeapOffset; // sectors
    DWORD dwClusterCount;
    DWORD dwRootCluster;
    DWORD dwSerialNumber;
    WORD wRevision;
    WORD wVolumeFlags; // bit 0: the active FAT and bitmap
    BYTE uchBytesPerSectorShift; // 9..12
    BYTE uchSectorsPerClusterShift; // cluster at most 32 MB
    BYTE uchNumberOfFATs; // 1, or 2 (TexFAT)
};

struct EXFAT_FILE_ENTRY { // 0x85
    BYTE uchType;
    BYTE uchSecondaryCount; // the entries that follow: one Stream Extension, the File Name entries, maybe more
    WORD wSetChecksum; // over all entries of the set, these two bytes left out
    WORD wAttributes;
    WORD wReserved1;
    DWORD dwCreateTime; // DOS date and time, local time
    DWORD dwModifyTime;
    DWORD dwAccessTime;
    BYTE uchCreate10ms; // 0..199
    BYTE uchModify10ms;
    BYTE uchCreateUtcOffset; // bit 7: valid, bits 0-6: signed, in 15 minutes
    BYTE uchModifyUtcOffset;
    BYTE uchAccessUtcOffset;
    BYTE uchReserved2[7];
};

struct EXFAT_STREAM_ENTRY { // 0xC0
    BYTE uchType;
    BYTE uchFlags; // STREAM_*
    BYTE uchReserved1;
    BYTE uchNameLength; // in UTF-16 characters
    WORD wNameHash;
    WORD wReserved2;
    ULONGLONG ullValidDataLength;
    DWORD dwReserved3;
    DWORD dwFirstCluster;
    ULONGLONG ullDataLength;
};

struct EXFAT_BITMAP_ENTRY { // 0x81
    BYTE uchType;
    BYTE uchFlags; // bit 0: the bitmap of the second FAT
    BYTE uchReserved[18];
    DWORD dwFirstCluster;
    ULONGLONG ullDataLength;
};

#pragma pack(pop)

CExFATDrive::CExFATDrive() {
    m_bInitialized = false;
    m_dwBytesPerSector = 0;
    m_dwBytesPerCluster = 0;
    m_llFATOffset = 0;
    m_llHeapOffset = 0;
    m_dwMaxCluster = 0;
    m_dwRootCluster = 0;
    m_dwActiveFAT = 0;
    m_llProgressPending = 0;
}

CExFATDrive::~CExFATDrive() {
}

void CExFATDrive::SetDriveHandle(HANDLE hDrive) {
    CloseVolume();
    m_hDrive = hDrive;
    m_bInitialized = false;
}

int CExFATDrive::ReadVolume(wxString wxsSelectedDrive) {
    m_bInitialized = false;
    int nRet = OpenVolume(wxsSelectedDrive);
    if (nRet)
        return nRet;

    nRet = Initialize();
    if (nRet)
        wxLogMessage("%s: no exFAT volume, or unreadable (error %d).", wxsSelectedDrive, nRet);
    return nRet;
}

// dwLen bytes from byte llOffset of the volume - both a whole number of sectors

int CExFATDrive::ReadBytes(LONGLONG llOffset, BYTE *puchOut, DWORD dwLen) {
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

int CExFATDrive::ReadClusters(DWORD dwCluster, DWORD dwCount, BYTE *puchOut) {
    if (dwCluster < 2 || dwCluster > m_dwMaxCluster || dwCount > m_dwMaxCluster - dwCluster + 1)
        return ERROR_SECTOR_NOT_FOUND;
    return ReadBytes(m_llHeapOffset + (LONGLONG) (dwCluster - 2) * m_dwBytesPerCluster, puchOut, dwCount * m_dwBytesPerCluster);
}

int CExFATDrive::Initialize() {
    m_bInitialized = false;
    std::vector<ST_EXFATFILE>().swap(m_vFiles);
    std::vector<DWORD>().swap(m_vFAT);
    std::vector<BYTE>().swap(m_vBitmap);

    // 4 KB: a whole number of sectors for every sector size
    std::vector<BYTE> vBoot(4096);
    int nRet = ReadBytes(0, &vBoot[0], (DWORD) vBoot.size());
    if (nRet) {
        wxLogMessage("Cannot read the boot sector (error %d).", nRet);
        return nRet;
    }

    EXFAT_BOOT_SECTOR bs;
    memcpy(&bs, &vBoot[0], sizeof (bs));
    if (memcmp(bs.chFileSystemName, "EXFAT   ", 8))
        return ERROR_INVALID_DRIVE;
    for (int i = 0; i < (int) sizeof (bs.uchMustBeZero); i++)
        if (bs.uchMustBeZero[i])
            return ERROR_INVALID_DRIVE; // where FAT's BIOS parameter block would be - zero on exFAT

    if (bs.uchBytesPerSectorShift < 9 || bs.uchBytesPerSectorShift > 12
            || bs.uchBytesPerSectorShift + bs.uchSectorsPerClusterShift > 25 // a cluster is at most 32 MB
            || bs.uchNumberOfFATs < 1 || bs.uchNumberOfFATs > 2 || !bs.dwClusterCount || bs.dwClusterCount > 0xFFFFFFF5
            || bs.dwFATOffset < 24 || !bs.dwFATLength
            || (ULONGLONG) bs.dwClusterHeapOffset < (ULONGLONG) bs.dwFATOffset + (ULONGLONG) bs.dwFATLength * bs.uchNumberOfFATs) {
        wxLogMessage("The exFAT boot sector is damaged.");
        return ERROR_INVALID_DRIVE;
    }

    m_dwBytesPerSector = 1 << bs.uchBytesPerSectorShift;
    m_dwBytesPerCluster = m_dwBytesPerSector << bs.uchSectorsPerClusterShift;
    m_dwMaxCluster = bs.dwClusterCount + 1;
    // the FAT must have an entry for every cluster
    ULONGLONG ullFATBytes = (ULONGLONG) bs.dwFATLength * m_dwBytesPerSector;
    if (ullFATBytes < ((ULONGLONG) m_dwMaxCluster + 1) * 4 || bs.dwRootCluster < 2 || bs.dwRootCluster > m_dwMaxCluster) {
        wxLogMessage("The exFAT boot sector is damaged.");
        return ERROR_INVALID_DRIVE;
    }
    m_dwActiveFAT = (bs.uchNumberOfFATs == 2 && (bs.wVolumeFlags & 1)) ? 1 : 0;
    m_llFATOffset = ((LONGLONG) bs.dwFATOffset + (LONGLONG) m_dwActiveFAT * bs.dwFATLength) * m_dwBytesPerSector;
    m_llHeapOffset = (LONGLONG) bs.dwClusterHeapOffset * m_dwBytesPerSector;
    m_dwRootCluster = bs.dwRootCluster;

    wxLogMessage("exFAT volume: %u clusters of %u bytes, %u-byte sectors.", (unsigned) bs.dwClusterCount,
            (unsigned) m_dwBytesPerCluster, (unsigned) m_dwBytesPerSector);
    m_bInitialized = true;
    return ERROR_SUCCESS;
}

// The FAT and the allocation bitmap are read once, at the start of a scan - recovery uses the same snapshot, so
// the clusters a file is recovered from are the ones its recoverability was judged by.

int CExFATDrive::LoadFAT() {
    std::vector<DWORD>().swap(m_vFAT);
    ULONGLONG ullBytes = ((ULONGLONG) m_dwMaxCluster + 1) * 4;
    ullBytes = (ullBytes + m_dwBytesPerSector - 1) / m_dwBytesPerSector * m_dwBytesPerSector; // whole sectors
    if (ullBytes > MAX_FAT_SIZE) {
        wxLogMessage("The FAT is too large (%llu bytes).", ullBytes);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    try {
        m_vFAT.resize((size_t) (ullBytes / 4));
    } catch (std::bad_alloc &) {
        wxLogMessage("Not enough memory for the FAT (%llu bytes).", ullBytes);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    int nRet = ReadBytes(m_llFATOffset, (BYTE *) &m_vFAT[0], (DWORD) ullBytes);
    if (nRet) {
        wxLogMessage("Cannot read the FAT (error %d).", nRet);
        std::vector<DWORD>().swap(m_vFAT);
    }
    return nRet;
}

// the allocation bitmap is a file of the root directory, named by a 0x81 entry (two of them with two FATs)

int CExFATDrive::LoadBitmap() {
    std::vector<BYTE>().swap(m_vBitmap);
    ST_DIR_TO_SCAN root = {m_dwRootCluster, false, 0, false, L"\\"};
    std::vector<BYTE> vRoot;
    int nRet = ReadDirectory(root, vRoot);
    if (nRet)
        return nRet;

    for (size_t i = 0; i + 32 <= vRoot.size() && vRoot[i] != ENTRY_END_OF_DIRECTORY; i += 32) {
        const EXFAT_BITMAP_ENTRY &e = *(const EXFAT_BITMAP_ENTRY *) &vRoot[i];
        if (e.uchType != ENTRY_BITMAP || (DWORD) (e.uchFlags & 1) != m_dwActiveFAT)
            continue;
        ULONGLONG ullNeeded = ((ULONGLONG) m_dwMaxCluster - 1 + 7) / 8;
        if (e.ullDataLength < ullNeeded || ullNeeded > MAX_FAT_SIZE)
            break;
        // the bitmap's clusters have a chain in the FAT; contiguous if it doesn't make sense
        DWORD dwClusters = (DWORD) ((ullNeeded + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster);
        std::vector<EXFAT_RUN> vRuns;
        if (!FollowChain(e.dwFirstCluster, dwClusters, vRuns)) {
            vRuns.clear();
            EXFAT_RUN run = {e.dwFirstCluster, dwClusters};
            vRuns.push_back(run);
        }
        try {
            m_vBitmap.resize((size_t) ullNeeded);
        } catch (std::bad_alloc &) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        nRet = ReadRuns(vRuns, ullNeeded, ullNeeded, &m_vBitmap[0], NULL);
        if (nRet)
            std::vector<BYTE>().swap(m_vBitmap);
        return nRet;
    }
    wxLogMessage("The allocation bitmap isn't in the root directory.");
    return ERROR_FILE_CORRUPT;
}

static void AddCluster(std::vector<CExFATDrive::EXFAT_RUN> &vRuns, DWORD dwCluster) {
    if (!vRuns.empty() && vRuns.back().dwCluster + vRuns.back().dwCount == dwCluster) {
        vRuns.back().dwCount++;
    } else {
        CExFATDrive::EXFAT_RUN run = {dwCluster, 1};
        vRuns.push_back(run);
    }
}

// dwCount clusters of the chain from dwFirstCluster on - false if the chain ends or leaves the volume before

bool CExFATDrive::FollowChain(DWORD dwFirstCluster, DWORD dwCount, std::vector<EXFAT_RUN> &vRuns) const {
    vRuns.clear();
    if (m_vFAT.empty())
        return false;
    DWORD c = dwFirstCluster;
    for (DWORD n = 0; n < dwCount; n++) {
        if (c < 2 || c > m_dwMaxCluster)
            return false;
        AddCluster(vRuns, c);
        c = m_vFAT[c];
    }
    return true;
}

bool CExFATDrive::LiveClusters(DWORD dwFirstCluster, bool bNoFatChain, ULONGLONG ullSize, std::vector<EXFAT_RUN> &vRuns) const {
    vRuns.clear();
    if (dwFirstCluster < 2 || dwFirstCluster > m_dwMaxCluster)
        return false;
    DWORD dwMaxClusters = wxMax((DWORD) 1, (DWORD) (MAX_DIRECTORY_SIZE / m_dwBytesPerCluster));

    if (!ullSize) { // the root directory: its chain, to its end
        for (DWORD c = dwFirstCluster, n = 0; n < dwMaxClusters; n++) {
            AddCluster(vRuns, c);
            if (m_vFAT.empty())
                break;
            DWORD dwNext = m_vFAT[c];
            if (dwNext >= FAT_END_OF_CHAIN || dwNext < 2 || dwNext > m_dwMaxCluster)
                break;
            c = dwNext;
        }
        return true;
    }

    ULONGLONG ullClusters = (ullSize + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster;
    DWORD dwClusters = (DWORD) wxMin(ullClusters, (ULONGLONG) dwMaxClusters);
    if (bNoFatChain) {
        if (dwClusters > m_dwMaxCluster - dwFirstCluster + 1)
            return false;
        EXFAT_RUN run = {dwFirstCluster, dwClusters};
        vRuns.push_back(run);
        return true;
    }
    if (FollowChain(dwFirstCluster, dwClusters, vRuns))
        return true;
    return !vRuns.empty(); // as much of the chain as there is
}

bool CExFATDrive::DeletedClusters(DWORD dwFirstCluster, bool bNoFatChain, ULONGLONG ullSize, std::vector<EXFAT_RUN> &vRuns,
        t_FileInfo *pFileInfo) const {
    vRuns.clear();
    if (pFileInfo) {
        pFileInfo->iRecoverability = RECOVERABILITY_UNKNOWN;
        pFileInfo->iPercentIntact = 0;
        pFileInfo->bStartReused = false;
    }

    if (!ullSize) { // an empty file has no clusters - there is nothing to lose
        if (pFileInfo) {
            pFileInfo->iRecoverability = RECOVERABILITY_GOOD;
            pFileInfo->iPercentIntact = 100;
        }
        return true;
    }
    if (dwFirstCluster < 2 || dwFirstCluster > m_dwMaxCluster || m_vBitmap.empty())
        return false;
    ULONGLONG ullNeeded = (ullSize + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster;
    if (ullNeeded > m_dwMaxCluster - 1)
        return false; // bigger than the volume - the entry can't be right
    DWORD dwNeeded = (DWORD) ullNeeded;

    // where the clusters are: known - contiguous, or the chain the FAT still holds - or else guessed
    bool bKnown = false;
    DWORD dwSkipped = 0;
    if (bNoFatChain) {
        if (dwNeeded > m_dwMaxCluster - dwFirstCluster + 1)
            return false;
        EXFAT_RUN run = {dwFirstCluster, dwNeeded};
        vRuns.push_back(run);
        bKnown = true;
    } else {
        bKnown = FollowChain(dwFirstCluster, dwNeeded, vRuns);
    }

    bool bStartReused = IsClusterInUse(dwFirstCluster);
    if (!bKnown) {
        // The chain is broken: another file has taken a cluster of it and changed its FAT entry. Guess like on
        // FAT - the free clusters from the first one on, skipping the ones in use; unless the first one is taken
        // too, then the file is judged as a contiguous one.
        vRuns.clear();
        DWORD dwTaken = 0;
        DWORD dwMaxSkipped = wxMax(dwNeeded, (DWORD) MIN_CLUSTERS_TO_SKIP);
        for (DWORD c = dwFirstCluster; !bStartReused && dwTaken < dwNeeded && c <= m_dwMaxCluster && dwSkipped <= dwMaxSkipped; c++) {
            if (IsClusterInUse(c)) {
                dwSkipped++;
            } else {
                AddCluster(vRuns, c);
                dwTaken++;
            }
        }
        if (dwTaken < dwNeeded) {
            vRuns.clear();
            dwSkipped = 0;
            if (dwNeeded > m_dwMaxCluster - dwFirstCluster + 1)
                return false;
            EXFAT_RUN run = {dwFirstCluster, dwNeeded};
            vRuns.push_back(run);
        }
    }

    if (pFileInfo) {
        DWORD dwFree = 0;
        for (size_t i = 0; i < vRuns.size(); i++)
            for (DWORD c = vRuns[i].dwCluster; c < vRuns[i].dwCluster + vRuns[i].dwCount; c++)
                if (!IsClusterInUse(c))
                    dwFree++;
        pFileInfo->bStartReused = bStartReused;
        int iPercent;
        if (dwSkipped) // free, but other files' clusters were skipped: fragmented around them, or partly taken - can't tell
            iPercent = (int) ((ULONGLONG) dwNeeded * 100 / ((ULONGLONG) dwNeeded + dwSkipped));
        else
            iPercent = (int) ((ULONGLONG) dwFree * 100 / dwNeeded);
        if (dwFree && (dwFree < dwNeeded || dwSkipped))
            iPercent = (iPercent < 1) ? 1 : (iPercent > 99 ? 99 : iPercent);
        pFileInfo->iPercentIntact = iPercent;
        pFileInfo->iRecoverability = (dwFree == dwNeeded && !dwSkipped) ? RECOVERABILITY_GOOD
                : (dwFree ? RECOVERABILITY_PARTIAL : RECOVERABILITY_OVERWRITTEN);
    }
    return true;
}

// the GUI thread shows how much of the selection has been written - counted in KB, like the other engines

void CExFATDrive::AddProgress(DWORD dwBytes) {
    if (!m_plProgressKB)
        return;
    m_llProgressPending += dwBytes;
    LONG lKB = (LONG) (m_llProgressPending >> 10);
    if (lKB) {
        InterlockedExchangeAdd((LONG *) m_plProgressKB, lKB);
        m_llProgressPending -= (LONGLONG) lKB << 10;
    }
}

int CExFATDrive::ReadRuns(const std::vector<EXFAT_RUN> &vRuns, ULONGLONG ullLen, ULONGLONG ullValid, BYTE *puchOut, HANDLE hFile) {
    DWORD dwChunkClusters = wxMax((DWORD) 1, (DWORD) (COPY_CHUNK_SIZE / m_dwBytesPerCluster));
    std::vector<BYTE> vBuffer;
    try {
        vBuffer.resize((size_t) dwChunkClusters * m_dwBytesPerCluster);
    } catch (std::bad_alloc &) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ULONGLONG ullPos = 0; // bytes of the file done
    for (size_t i = 0; i < vRuns.size() && ullPos < ullLen; i++) {
        for (DWORD dwDone = 0; dwDone < vRuns[i].dwCount && ullPos < ullLen;) {
            if (IsCancelled())
                return ERROR_CANCELLED;

            DWORD dwCount = wxMin(vRuns[i].dwCount - dwDone, dwChunkClusters);
            // the last chunk of the file only needs part of its clusters
            ULONGLONG ullChunkBytes = (ULONGLONG) dwCount * m_dwBytesPerCluster;
            DWORD dwUseful = (DWORD) wxMin(ullLen - ullPos, ullChunkBytes);
            dwCount = (dwUseful + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster;

            if (ullPos >= ullValid) {
                memset(&vBuffer[0], 0, dwUseful); // past the valid data - never written, reads as zeros
            } else {
                int nRet = ReadClusters(vRuns[i].dwCluster + dwDone, dwCount, &vBuffer[0]);
                if (nRet)
                    return nRet;
                if (ullPos + dwUseful > ullValid)
                    memset(&vBuffer[(size_t) (ullValid - ullPos)], 0, (size_t) (ullPos + dwUseful - ullValid));
            }

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
            ullPos += dwUseful;
            dwDone += dwCount;
        }
    }
    return ullPos < ullLen ? ERROR_HANDLE_EOF : ERROR_SUCCESS;
}

int CExFATDrive::Read_File(DWORD dwFileId, BYTE *&puchFileData, DWORD &dwFileDataLen) {
    if (!m_bInitialized || dwFileId >= m_vFiles.size())
        return ERROR_INVALID_ACCESS;

    const ST_EXFATFILE &file = m_vFiles[dwFileId];
    if (file.ullSize > 0x7FFFFFFF)
        return ERROR_NOT_ENOUGH_MEMORY;
    std::vector<EXFAT_RUN> vRuns;
    if (!DeletedClusters(file.dwFirstCluster, file.bNoFatChain, file.ullSize, vRuns, NULL))
        return ERROR_FILE_CORRUPT;

    DWORD dwSize = (DWORD) file.ullSize;
    BYTE *puchData = new (std::nothrow) BYTE[dwSize ? dwSize : 1];
    if (!puchData)
        return ERROR_NOT_ENOUGH_MEMORY;
    int nRet = ReadRuns(vRuns, dwSize, file.ullValid, puchData, NULL);
    if (nRet) {
        delete[] puchData;
        return nRet;
    }
    puchFileData = puchData;
    dwFileDataLen = dwSize;
    return ERROR_SUCCESS;
}

int CExFATDrive::ReadFileStart(DWORD dwFileId, std::vector<BYTE> &vData, DWORD dwMax) {
    vData.clear();
    if (!m_bInitialized || dwFileId >= m_vFiles.size())
        return ERROR_INVALID_ACCESS;

    const ST_EXFATFILE &file = m_vFiles[dwFileId];
    std::vector<EXFAT_RUN> vRuns;
    if (!DeletedClusters(file.dwFirstCluster, file.bNoFatChain, file.ullSize, vRuns, NULL))
        return ERROR_FILE_CORRUPT;

    DWORD dwWant = (DWORD) wxMin(file.ullSize, (ULONGLONG) dwMax);
    if (!dwWant)
        return ERROR_SUCCESS;
    vData.resize(dwWant);
    int nRet = ReadRuns(vRuns, dwWant, file.ullValid, &vData[0], NULL);
    if (nRet)
        vData.clear();
    return nRet;
}

int CExFATDrive::RetreiveAndWriteFileToDestination(int iFileId, const wchar_t *wszDestinationFileName) {
    wxString sDestinationFileName(wszDestinationFileName); // for logging

    if (!m_bInitialized || iFileId < 0 || (size_t) iFileId >= m_vFiles.size()) {
        wxLogMessage("Invalid Access during retrieving \'%s\'", sDestinationFileName);
        return ERROR_INVALID_ACCESS;
    }

    const ST_EXFATFILE &file = m_vFiles[iFileId];
    std::vector<EXFAT_RUN> vRuns;
    if (!DeletedClusters(file.dwFirstCluster, file.bNoFatChain, file.ullSize, vRuns, NULL)) {
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

    wxLogMessage("Extracting file \'%s\' (%llu bytes in %u fragments).", sDestinationFileName, file.ullSize, (unsigned) vRuns.size());
    m_llProgressPending = 0;
    int nRet = ReadRuns(vRuns, file.ullSize, file.ullValid, NULL, hFile);
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

int CExFATDrive::ReadDirectory(const ST_DIR_TO_SCAN &dir, std::vector<BYTE> &vData) {
    vData.clear();
    std::vector<EXFAT_RUN> vRuns;
    if (!dir.bDeleted) {
        if (!LiveClusters(dir.dwCluster, dir.bNoFatChain, dir.ullSize, vRuns))
            return ERROR_FILE_CORRUPT;
    } else {
        // A deleted folder: only its clusters that are still free can still hold its entries - one in use holds
        // something else by now (maybe another folder, whose files must not show up as this one's)
        std::vector<EXFAT_RUN> vAll;
        ULONGLONG ullSize = wxMin(dir.ullSize, (ULONGLONG) MAX_DIRECTORY_SIZE);
        if (!DeletedClusters(dir.dwCluster, dir.bNoFatChain, ullSize, vAll, NULL))
            return ERROR_FILE_CORRUPT;
        for (size_t i = 0; i < vAll.size(); i++)
            for (DWORD c = vAll[i].dwCluster; c < vAll[i].dwCluster + vAll[i].dwCount; c++)
                if (!IsClusterInUse(c))
                    AddCluster(vRuns, c);
    }

    for (size_t i = 0; i < vRuns.size(); i++) {
        size_t nOld = vData.size();
        vData.resize(nOld + (size_t) vRuns[i].dwCount * m_dwBytesPerCluster);
        int nRet = ReadClusters(vRuns[i].dwCluster, vRuns[i].dwCount, &vData[nOld]);
        if (nRet) {
            vData.resize(nOld);
            return vData.empty() ? nRet : ERROR_SUCCESS; // what could be read
        }
    }
    return ERROR_SUCCESS;
}

static WORD EntrySetChecksum(const BYTE *puchSet, size_t nBytes) {
    WORD wSum = 0;
    for (size_t i = 0; i < nBytes; i++) {
        if (i == 2 || i == 3)
            continue; // the checksum itself
        wSum = (WORD) (((wSum & 1) ? 0x8000 : 0) + (wSum >> 1) + puchSet[i]);
    }
    return wSum;
}

// exFAT dates are DOS dates and times, local time - with the offset from UTC when it is known

static LONGLONG ExFATTimeToFileTime(DWORD dwStamp, BYTE uch10ms, BYTE uchUtcOffset) {
    if (!dwStamp)
        return 0;
    SYSTEMTIME st = {0};
    st.wYear = (WORD) (1980 + (dwStamp >> 25));
    st.wMonth = (WORD) ((dwStamp >> 21) & 0x0F);
    st.wDay = (WORD) ((dwStamp >> 16) & 0x1F);
    st.wHour = (WORD) ((dwStamp >> 11) & 0x1F);
    st.wMinute = (WORD) ((dwStamp >> 5) & 0x3F);
    st.wSecond = (WORD) ((dwStamp & 0x1F) * 2);
    if (uch10ms < 200) {
        st.wSecond = (WORD) (st.wSecond + uch10ms / 100);
        st.wMilliseconds = (WORD) ((uch10ms % 100) * 10);
    }

    FILETIME ft;
    if (uchUtcOffset & 0x80) {
        // the local time of a known time zone: UTC = local time - offset (a 7-bit signed number of 15 minutes)
        int iOffset = uchUtcOffset & 0x7F;
        if (iOffset & 0x40)
            iOffset -= 0x80;
        if (!SystemTimeToFileTime(&st, &ft))
            return 0;
        LONGLONG ll = ((LONGLONG) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        return ll - (LONGLONG) iOffset * 15 * 60 * 10000000LL;
    }
    // no offset: like FAT, the local time of whoever wrote it - taken as this computer's
    SYSTEMTIME stUtc;
    if (!TzSpecificLocalTimeToSystemTime(NULL, &st, &stUtc) || !SystemTimeToFileTime(&stUtc, &ft))
        return 0;
    return ((LONGLONG) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

static bool IsNameChar(wchar_t wch) {
    return wch >= 0x20 && !wcschr(L"\"*/:<>?\\|", wch);
}

static std::wstring ChildPath(const std::wstring &sParent, const std::wstring &sName) {
    return (sParent == L"\\") ? sParent + sName : sParent + L"\\" + sName;
}

void CExFATDrive::ScanDirectory(const ST_DIR_TO_SCAN &dir, std::vector<ST_DIR_TO_SCAN> &vToScan, std::set<DWORD> &setSeen,
        FileInfoHash *finfohash) {
    std::vector<BYTE> vData;
    int nRet = ReadDirectory(dir, vData);
    if (nRet) {
        if (!dir.bDeleted)
            wxLogMessage("Cannot read the folder %s (error %d).", wxString(dir.sPath.c_str()), nRet);
        return;
    }

    size_t nEntries = vData.size() / 32;
    for (size_t i = 0; i < nEntries; i++) {
        const BYTE *puchEntry = &vData[i * 32];
        if (puchEntry[0] == ENTRY_END_OF_DIRECTORY)
            break;
        if ((puchEntry[0] | ENTRY_IN_USE) != ENTRY_FILE)
            continue; // the bitmap, the label... - or a secondary entry of a set that didn't check out

        bool bSetDeleted = !(puchEntry[0] & ENTRY_IN_USE);
        size_t nSecondary = puchEntry[1];
        if (nSecondary < 2 || nSecondary > 18 || i + nSecondary >= nEntries)
            continue;

        // The set with its "in use" bits set again, as it was when its checksum was made. A deleted set whose
        // entries have partly been taken by a newer set has entries in use among them - or a wrong checksum.
        BYTE uchSet[19 * 32];
        size_t nSetBytes = (nSecondary + 1) * 32;
        memcpy(uchSet, puchEntry, nSetBytes);
        bool bWhole = true;
        for (size_t k = 1; k <= nSecondary && bWhole; k++) {
            BYTE uchType = uchSet[k * 32];
            if (!(uchType & 0x40) || (bSetDeleted && (uchType & ENTRY_IN_USE)) || (!bSetDeleted && !(uchType & ENTRY_IN_USE)))
                bWhole = false; // not a secondary entry, or not of this set
            uchSet[k * 32] |= ENTRY_IN_USE;
        }
        uchSet[0] |= ENTRY_IN_USE;
        const EXFAT_FILE_ENTRY &file = *(const EXFAT_FILE_ENTRY *) uchSet;
        const EXFAT_STREAM_ENTRY &stream = *(const EXFAT_STREAM_ENTRY *) (uchSet + 32);
        size_t nNameEntries = ((size_t) stream.uchNameLength + 14) / 15;
        if (!bWhole || stream.uchType != ENTRY_STREAM || !stream.uchNameLength || 1 + nNameEntries > nSecondary
                || EntrySetChecksum(uchSet, nSetBytes) != file.wSetChecksum)
            continue;

        std::wstring sName;
        bool bValid = true;
        for (size_t k = 0; k < nNameEntries && bValid; k++) {
            const BYTE *puchName = uchSet + (2 + k) * 32;
            bValid = puchName[0] == ENTRY_NAME;
            for (int c = 0; c < 15 && bValid && sName.length() < stream.uchNameLength; c++)
                sName += (wchar_t) (puchName[2 + c * 2] | (puchName[3 + c * 2] << 8));
        }
        for (size_t k = 0; k < sName.length() && bValid; k++)
            bValid = IsNameChar(sName[k]);
        if (!bValid || sName.empty())
            continue;
        i += nSecondary; // the set checked out - its entries are done

        // in a deleted folder, everything is deleted - whether its entries were marked or not
        bool bDeleted = bSetDeleted || dir.bDeleted;
        bool bNoFatChain = (stream.uchFlags & STREAM_NO_FAT_CHAIN) != 0;
        DWORD dwCluster = (stream.uchFlags & STREAM_ALLOCATION_POSSIBLE) ? stream.dwFirstCluster : 0;

        if (file.wAttributes & ATTR_DIRECTORY) {
            // a deleted folder can only be read while its first cluster is still free
            if (dwCluster >= 2 && dwCluster <= m_dwMaxCluster && stream.ullDataLength && !setSeen.count(dwCluster)
                    && (!bDeleted || !IsClusterInUse(dwCluster))) {
                setSeen.insert(dwCluster);
                ST_DIR_TO_SCAN sub = {dwCluster, bNoFatChain, stream.ullDataLength, bDeleted, ChildPath(dir.sPath, sName)};
                vToScan.push_back(sub);
            }
            continue;
        }

        if (!bDeleted || (file.wAttributes & (ATTR_SYSTEM | ATTR_HIDDEN)))
            continue; // like on NTFS: no system or hidden files

        t_FileInfo *pFileInfo = new t_FileInfo;
        pFileInfo->sFilename = sName;
        pFileInfo->sFolder = dir.sPath;
        pFileInfo->bDeleted = true;
        pFileInfo->dwAttributes = file.wAttributes & (ATTR_READ_ONLY | ATTR_ARCHIVE);
        pFileInfo->n64Create = ExFATTimeToFileTime(file.dwCreateTime, file.uchCreate10ms, file.uchCreateUtcOffset);
        pFileInfo->n64Modify = ExFATTimeToFileTime(file.dwModifyTime, file.uchModify10ms, file.uchModifyUtcOffset);
        pFileInfo->n64Modfil = pFileInfo->n64Modify;
        pFileInfo->n64Access = ExFATTimeToFileTime(file.dwAccessTime, 0, file.uchAccessUtcOffset);
        pFileInfo->n64RealSize = (LONGLONG) stream.ullDataLength;
        pFileInfo->n64SizeOnDisk = (LONGLONG) ((stream.ullDataLength + m_dwBytesPerCluster - 1) / m_dwBytesPerCluster * m_dwBytesPerCluster);

        std::vector<EXFAT_RUN> vRuns;
        DeletedClusters(dwCluster, bNoFatChain, stream.ullDataLength, vRuns, pFileInfo);

        ST_EXFATFILE found = {dwCluster, bNoFatChain, stream.ullDataLength, wxMin(stream.ullValidDataLength, stream.ullDataLength)};
        pFileInfo->iRecordId = (int) m_vFiles.size();
        m_vFiles.push_back(found);
        AddFoundFile(finfohash, pFileInfo);
    }
}

int CExFATDrive::ScanFiles(FileInfoHash *finfohash) {
    if (!m_bInitialized)
        return ERROR_INVALID_ACCESS;

    std::vector<ST_EXFATFILE>().swap(m_vFiles);
    ReportScanProgress(-1); // how many folders there are only shows at the end
    int nRet = LoadFAT();
    if (!nRet)
        nRet = LoadBitmap();
    if (nRet) {
        wxLogMessage("The FAT or the allocation bitmap can't be read (error %d).", nRet);
        return nRet;
    }

    // the folders still to scan - a stack: the order doesn't matter, every folder carries its own path
    std::vector<ST_DIR_TO_SCAN> vToScan;
    std::set<DWORD> setSeen; // folder clusters already taken - a damaged or reused folder must not make a loop
    ST_DIR_TO_SCAN root = {m_dwRootCluster, false, 0, false, L"\\"};
    vToScan.push_back(root);
    setSeen.insert(m_dwRootCluster);

    unsigned uFolders = 0;
    while (!vToScan.empty()) {
        if (IsCancelled()) {
            wxLogMessage("Scan cancelled after %u folders.", uFolders);
            return ERROR_CANCELLED;
        }
        ST_DIR_TO_SCAN dir = vToScan.back();
        vToScan.pop_back();
        ScanDirectory(dir, vToScan, setSeen, finfohash);
        uFolders++;
    }

    wxLogMessage("No more files to add to the list (%u folders scanned).", uFolders);
    return ERROR_SUCCESS; // the Recycle Bin names come in FinishScan
}
