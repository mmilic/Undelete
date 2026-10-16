/*
 * File:   MyThreadClass.h
 *
 * The scan and the recovery run in threads, so the window stays responsive. A thread never touches a
 * control: the window polls its progress with a timer, and the thread posts one wxEVT_THREAD event to its
 * owner when it is done (id THREAD_SCAN_DONE / THREAD_COPY_DONE). The owner then Wait()s for it and reads
 * the outcome. Threads are joinable, and Cancel() stops them within a few thousand MFT records or 1 MB of
 * a file - the engine checks the flag.
 */

#ifndef MYTHREADCLASS_H
#define	MYTHREADCLASS_H

#include <wx/thread.h>
#include <wx/event.h>
#include <vector>
#include "RecoveryDrive.h"

enum {
    THREAD_SCAN_DONE = 1,
    THREAD_COPY_DONE
};

// open a drive ("E:\") or a disk image with the engine of its file system: NTFS, else FAT.
// NULL with nError set when it can't be opened; ERROR_INVALID_DRIVE: neither NTFS nor FAT.
CRecoveryDrive *OpenRecoveryDrive(const wxString &sSource, int &nError);

class CScanThread : public wxThread {
public:
    CScanThread(wxEvtHandler *pOwner, const wxString &sSource, FileInfoHash *pHash);

    void Cancel() { m_bCancel = true; }
    int GetPercent() const { return (int) m_lPercent; } // -1 while the engine can't tell

    // the files found since the last call - the window shows them while the scan goes on. They are complete
    // except for what the engine's FinishScan adds (folder paths on NTFS, Recycle Bin names).
    void TakeNewFiles(std::vector<t_FileInfo *> &vFiles) { m_found.Take(vFiles); }

    // after Wait(): ERROR_SUCCESS, ERROR_CANCELLED (the hash holds what was found until then) or an error,
    // and the drive's engine - the caller takes it over (NULL if the drive couldn't be opened) and calls its
    // FinishScan
    int m_iResult;
    CRecoveryDrive *m_pDrive;

protected:
    void *Entry() override;

private:
    wxEvtHandler *m_pOwner;
    wxString m_sSource;
    FileInfoHash *m_pHash;
    volatile bool m_bCancel;
    volatile LONG m_lPercent;
    CFoundFiles m_found;
};

class CCopyThread : public wxThread {
public:
    CCopyThread(wxEvtHandler *pOwner, CRecoveryDrive *pDrive, const std::vector<t_FileInfo *> &vFiles, const wxString &sDestinationDir);

    void Cancel() { m_bCancel = true; }
    LONG GetKBWritten() const { return m_lKBWritten; } // all files together
    int GetCurrentIndex() const { return (int) m_lCurrent; } // 0-based, the file being written
    const std::vector<t_FileInfo *> &GetFiles() const { return m_vFiles; }

    // after Wait()
    int m_iCopied;
    std::vector<wxString> m_vFailures; // "name - reason" for each file that could not be recovered
    bool m_bCancelled;

protected:
    void *Entry() override;

private:
    wxEvtHandler *m_pOwner;
    CRecoveryDrive *m_pDrive;
    std::vector<t_FileInfo *> m_vFiles;
    wxString m_sDestinationDir;
    volatile bool m_bCancel;
    volatile LONG m_lKBWritten;
    volatile LONG m_lCurrent;
};

// "photo.jpg", 2 -> "photo (2).jpg" - the name for a recovered file when the original name is taken
wxString NumberedFileName(const wxString &sName, int iNumber);

#endif	/* MYTHREADCLASS_H */
