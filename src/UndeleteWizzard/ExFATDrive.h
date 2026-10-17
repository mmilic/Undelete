/*
 * File:   ExFATDrive.h
 *
 * Recovery engine for exFAT volumes - the format of nearly every SD card and USB stick over 32 GB.
 *
 * A file is a set of 32-byte directory entries: a File entry (attributes, dates), a Stream Extension entry (first
 * cluster, size, whether the clusters are contiguous) and File Name entries (the whole name, 15 UTF-16 characters
 * each). Deleting a file only clears the "in use" bit of each entry (0x85 0xC0 0xC1 become 0x05 0x40 0x41) and
 * its clusters in the allocation bitmap. So unlike on FAT, everything stays known:
 * - a contiguous file ("NoFatChain") is its first cluster and its size
 * - a fragmented one keeps its chain in the FAT - the FAT isn't touched when a file is deleted
 * Whether the data is still there is judged by the allocation bitmap, the way NTFS does it. Should a chain be
 * broken (another file reused part of it), the clusters are guessed like on FAT: from the first one on,
 * skipping the clusters in use - and such a file is never called "Good".
 *
 * Each entry set carries a checksum over all its entries, which tells a deleted set that is still whole from
 * one that new entries have partly overwritten. The scan walks the directory tree from the root, deleted folders
 * included while their first cluster is still free.
 */

#ifndef EXFATDRIVE_H
#define	EXFATDRIVE_H

#include <set>
#include "RecoveryDrive.h"

class CExFATDrive : public CRecoveryDrive {
public:
    CExFATDrive();
    virtual ~CExFATDrive();

    const char *FileSystemName() const override { return "exFAT"; }

    // a volume opened by the caller (the test harness opens a disk image), starting at byte 0
    void SetDriveHandle(HANDLE hDrive);
    // read the boot sector. ERROR_INVALID_DRIVE: not an exFAT volume
    int Initialize();
    bool IsInitialized() const { return m_bInitialized; }

    int ReadVolume(wxString wxsSelectedDrive) override;
    int ScanFiles(FileInfoHash *finfohash) override;
    int Read_File(DWORD dwFileId, BYTE *&puchFileData, DWORD &dwFileDataLen) override;
    int ReadFileStart(DWORD dwFileId, std::vector<BYTE> &vData, DWORD dwMax) override;
    int RetreiveAndWriteFileToDestination(int iFileId, const wchar_t *wszDestinationFileName) override;

    // a run of consecutive clusters
    struct EXFAT_RUN {
        DWORD dwCluster;
        DWORD dwCount;
    };

private:
    bool m_bInitialized;
    DWORD m_dwBytesPerSector;
    DWORD m_dwBytesPerCluster;
    LONGLONG m_llFATOffset; // bytes from the start of the volume - of the FAT in use
    LONGLONG m_llHeapOffset; // where cluster 2 starts
    DWORD m_dwMaxCluster; // the highest valid cluster number (the clusters are 2..m_dwMaxCluster)
    DWORD m_dwRootCluster;
    DWORD m_dwActiveFAT; // 0, or 1 on a volume with two FATs (TexFAT) whose second one is active

    std::vector<DWORD> m_vFAT; // the FAT as it was when the scan started - the chains of fragmented files
    std::vector<BYTE> m_vBitmap; // the allocation bitmap: bit n is cluster n + 2, set = in use

    // what the scan found about each deleted file - ST_FILEINFO::iRecordId is the index here
    struct ST_EXFATFILE {
        DWORD dwFirstCluster;
        bool bNoFatChain; // contiguous: the FAT has no chain for it
        ULONGLONG ullSize; // DataLength
        ULONGLONG ullValid; // ValidDataLength - what lies past it reads as zeros
    };
    std::vector<ST_EXFATFILE> m_vFiles;

    int LoadFAT();
    int LoadBitmap();
    bool IsClusterInUse(DWORD dwCluster) const {
        if (dwCluster < 2 || dwCluster > m_dwMaxCluster || m_vBitmap.empty())
            return true;
        DWORD n = dwCluster - 2;
        return (m_vBitmap[n >> 3] & (1 << (n & 7))) != 0;
    }

    // the clusters of a deleted file or folder, see the top of ExFATDrive.h. Also judges its recoverability
    // when pFileInfo is given. False if the first cluster or the size can't be right.
    bool DeletedClusters(DWORD dwFirstCluster, bool bNoFatChain, ULONGLONG ullSize, std::vector<EXFAT_RUN> &vRuns,
            t_FileInfo *pFileInfo) const;
    // the clusters of a folder that exists: contiguous, or its chain (ullSize 0: the root - to the chain's end)
    bool LiveClusters(DWORD dwFirstCluster, bool bNoFatChain, ULONGLONG ullSize, std::vector<EXFAT_RUN> &vRuns) const;
    bool FollowChain(DWORD dwFirstCluster, DWORD dwCount, std::vector<EXFAT_RUN> &vRuns) const;

    int ReadBytes(LONGLONG llOffset, BYTE *puchOut, DWORD dwLen);
    int ReadClusters(DWORD dwCluster, DWORD dwCount, BYTE *puchOut);
    // the data of runs, at most ullLen bytes, zeros past ullValid: into puchOut, or written to hFile when it's given
    int ReadRuns(const std::vector<EXFAT_RUN> &vRuns, ULONGLONG ullLen, ULONGLONG ullValid, BYTE *puchOut, HANDLE hFile);

    struct ST_DIR_TO_SCAN {
        DWORD dwCluster;
        bool bNoFatChain;
        ULONGLONG ullSize; // 0: the root directory, whose size only its chain tells
        bool bDeleted;
        std::wstring sPath;
    };
    int ReadDirectory(const ST_DIR_TO_SCAN &dir, std::vector<BYTE> &vData);
    void ScanDirectory(const ST_DIR_TO_SCAN &dir, std::vector<ST_DIR_TO_SCAN> &vToScan, std::set<DWORD> &setSeen,
            FileInfoHash *finfohash);

    LONGLONG m_llProgressPending; // bytes written but not yet added to *m_plProgressKB (less than 1 KB)
    void AddProgress(DWORD dwBytes);
};

#endif	/* EXFATDRIVE_H */
