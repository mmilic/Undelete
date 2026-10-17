/*
 * File:   FATDrive.h
 *
 * Recovery engine for FAT12, FAT16 and FAT32 volumes (USB sticks, memory cards, old disks).
 *
 * Deleting a file on FAT sets the first byte of its 32-byte directory entry to 0xE5 (its long file name
 * entries too) and frees its cluster chain in the FAT. The rest of the entry stays: the name without its
 * first character, the size, the dates and the first cluster. Where the other clusters were is lost with
 * the chain, so it is guessed the way DOS UNDELETE did: from the first cluster on, take as many clusters as
 * the size needs, skipping the ones in use now. A contiguous file comes back whole, and so does a file
 * fragmented around files that still exist - but a file whose clusters had to be skipped is never called
 * "Good", because a file that was partly overwritten looks the same. A file fragmented around space that is
 * free now comes back wrong, and nothing on the volume shows it: its start is right, the rest is whatever
 * was in the free clusters after it. If the first cluster is in use, the file is judged like a contiguous
 * one - no skipping.
 *
 * The scan walks the directory tree from the root, so each file's folder is known as it is found. Deleted
 * folders are walked too, as long as their first cluster is still free and still holds a directory.
 */

#ifndef FATDRIVE_H
#define	FATDRIVE_H

#include <set>
#include "RecoveryDrive.h"

#pragma pack(push, 1)

// boot sector (BIOS parameter block) - the common part, then the FAT32 extension
struct FAT_BOOT_SECTOR {
    BYTE uchJump[3];
    char chOemName[8];
    WORD wBytesPerSector;
    BYTE uchSectorsPerCluster;
    WORD wReservedSectors;
    BYTE uchNumberOfFATs;
    WORD wRootEntries; // FAT12/16: entries of the fixed root directory, FAT32: 0
    WORD wTotalSectors16;
    BYTE uchMedia;
    WORD wSectorsPerFAT16; // FAT32: 0
    WORD wSectorsPerTrack;
    WORD wHeads;
    DWORD dwHiddenSectors;
    DWORD dwTotalSectors32;
    // FAT32 only
    DWORD dwSectorsPerFAT32;
    WORD wExtFlags;
    WORD wVersion;
    DWORD dwRootCluster;
    WORD wFSInfoSector;
    WORD wBackupBootSector;
};

struct FAT_DIR_ENTRY {
    BYTE uchName[11]; // 8.3, space padded, no dot. uchName[0]: 0xE5 = deleted, 0 = end of the directory
    BYTE uchAttributes;
    BYTE uchNTCase; // 0x08: the base name is lower case, 0x10: the extension is
    BYTE uchCreateTimeTenth; // 10 ms units, 0..199
    WORD wCreateTime;
    WORD wCreateDate;
    WORD wAccessDate;
    WORD wFirstClusterHigh; // FAT32 only
    WORD wWriteTime;
    WORD wWriteDate;
    WORD wFirstClusterLow;
    DWORD dwFileSize;
};

// long file name: up to 20 of these before the 8.3 entry, the last part of the name first, 13 UTF-16 chars each
struct FAT_LFN_ENTRY {
    BYTE uchOrder; // 1, 2... from the 8.3 entry back; 0x40 marks the last one. 0xE5 once deleted
    WORD wName1[5];
    BYTE uchAttributes; // always 0x0F
    BYTE uchType;
    BYTE uchChecksum; // of the 11 bytes of the 8.3 name
    WORD wName2[6];
    WORD wFirstCluster; // always 0
    WORD wName3[2];
};

#pragma pack(pop)

enum {
    FAT_TYPE_NONE = 0,
    FAT_TYPE_12 = 12,
    FAT_TYPE_16 = 16,
    FAT_TYPE_32 = 32
};

class CFATDrive : public CRecoveryDrive {
public:
    CFATDrive();
    virtual ~CFATDrive();

    const char *FileSystemName() const override;

    // a volume opened by the caller (the test harness opens a disk image), starting at byte 0
    void SetDriveHandle(HANDLE hDrive);
    // read the boot sector and the FAT. ERROR_INVALID_DRIVE: not a FAT volume
    int Initialize();
    int GetFATType() const { return m_iFATType; }

    int ReadVolume(wxString wxsSelectedDrive) override;
    int ScanFiles(FileInfoHash *finfohash) override;
    int Read_File(DWORD dwFileId, BYTE *&puchFileData, DWORD &dwFileDataLen) override;
    int ReadFileStart(DWORD dwFileId, std::vector<BYTE> &vData, DWORD dwMax) override;
    int RetreiveAndWriteFileToDestination(int iFileId, const wchar_t *wszDestinationFileName) override;

    // a run of consecutive clusters
    struct FAT_RUN {
        DWORD dwCluster;
        DWORD dwCount;
    };

private:
    bool m_bInitialized;
    int m_iFATType;
    DWORD m_dwBytesPerSector;
    DWORD m_dwBytesPerCluster;
    LONGLONG m_llFATOffset; // bytes from the start of the volume
    DWORD m_dwFATBytes; // size of one FAT
    DWORD m_dwNumberOfFATs;
    DWORD m_dwActiveFAT; // FAT32 with mirroring off: the FAT in use, otherwise 0
    LONGLONG m_llRootDirOffset; // FAT12/16: the fixed root directory
    DWORD m_dwRootDirBytes;
    DWORD m_dwRootCluster; // FAT32: the root directory is an ordinary cluster chain
    LONGLONG m_llDataOffset; // where cluster 2 starts
    DWORD m_dwMaxCluster; // the highest valid cluster number (the clusters are 2..m_dwMaxCluster)

    std::vector<DWORD> m_vFAT; // the FAT, one entry per cluster, as it was when the scan started
    std::vector<BYTE> m_vInUse; // one bit per cluster: in use (or bad), from m_vFAT

    // what the scan found about each deleted file - ST_FILEINFO::iRecordId is the index here
    struct ST_FATFILE {
        DWORD dwFirstCluster;
        DWORD dwSize;
    };
    std::vector<ST_FATFILE> m_vFiles;

    int LoadFAT();
    bool IsClusterInUse(DWORD dwCluster) const {
        return (m_vInUse[dwCluster >> 3] & (1 << (dwCluster & 7))) != 0;
    }
    DWORD NextCluster(DWORD dwCluster) const; // the next cluster of a chain, 0 at its end (or if it's broken)
    bool IsEndOfChain(DWORD dwValue) const;

    // the clusters a deleted file most likely had, see the top of FATDrive.h. Also judges its recoverability
    // when pFileInfo is given. False if the first cluster can't be right.
    bool GuessClusters(DWORD dwFirstCluster, DWORD dwSize, std::vector<FAT_RUN> &vRuns, t_FileInfo *pFileInfo) const;

    // read whole clusters, cluster dwCluster on
    int ReadClusters(DWORD dwCluster, DWORD dwCount, BYTE *puchOut);
    int ReadBytes(LONGLONG llOffset, BYTE *puchOut, DWORD dwLen);
    // the data of runs, at most dwLen bytes: into puchOut, or written to hFile when it's given
    int ReadRuns(const std::vector<FAT_RUN> &vRuns, DWORD dwLen, BYTE *puchOut, HANDLE hFile);

    // the clusters of a directory: the chain of one that exists, the guessed clusters of a deleted one -
    // never one in setClaimed (another deleted folder's), and the ones it takes go there
    int ReadDirectory(DWORD dwCluster, bool bDeleted, std::vector<BYTE> &vData, std::set<DWORD> &setClaimed);

    // FAT32: Windows clears the high word of a deleted entry's first cluster, so past cluster 65535 only the low
    // word is left - the cluster is one of low, low + 65536, low + 2 * 65536... (ScanDirectory, in two passes):
    // First the ones that can be told for sure - a folder by its "." entry, which holds its own full cluster
    // number, a file by the content check of its start (0: none). The others go where they are expected from
    // their nearest neighbours in the folder that are known for sure - the place nearest to dwExpected.
    DWORD ResolveClearedHighWordExactly(WORD wLow, bool bDirectory, DWORD dwSize, const std::wstring &sName);
    DWORD ResolveClearedHighWordNear(WORD wLow, DWORD dwExpected) const;

    struct ST_DIR_TO_SCAN {
        DWORD dwCluster; // 0: the fixed root directory of FAT12/16
        bool bDeleted;
        std::wstring sPath;
    };
    void ScanDirectory(const ST_DIR_TO_SCAN &dir, std::vector<ST_DIR_TO_SCAN> &vToScan, std::set<DWORD> &setSeen,
            FileInfoHash *finfohash);

    LONGLONG m_llProgressPending; // bytes written but not yet added to *m_plProgressKB (less than 1 KB)
    void AddProgress(DWORD dwBytes);
};

#endif	/* FATDRIVE_H */
