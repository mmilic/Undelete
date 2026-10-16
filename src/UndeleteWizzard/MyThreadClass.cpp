
#include <wx/wx.h>
#include <wx/filename.h>

#include "NTFSDrive.h"
#include "FATDrive.h"
#include "MyThreadClass.h"
#include "IsValidFileName.h"
#include "languages/Translate.h"

CRecoveryDrive *OpenRecoveryDrive(const wxString &sSource, int &nError) {
    CNTFSDrive *pNTFS = new CNTFSDrive;
    nError = pNTFS->ReadVolume(sSource);
    if (!nError)
        return pNTFS;
    delete pNTFS;
    if (nError != ERROR_INVALID_DRIVE && nError != ERROR_BAD_DEVICE)
        return NULL; // it couldn't even be read (access denied...) - no point trying FAT

    CFATDrive *pFAT = new CFATDrive;
    nError = pFAT->ReadVolume(sSource);
    if (!nError)
        return pFAT;
    delete pFAT;
    return NULL;
}

/////////////////////////////////// scan ///////////////////////////////////

CScanThread::CScanThread(wxEvtHandler *pOwner, const wxString &sSource, FileInfoHash *pHash)
: wxThread(wxTHREAD_JOINABLE) {
    m_pOwner = pOwner;
    m_sSource = sSource;
    m_pHash = pHash;
    m_bCancel = false;
    m_lPercent = 0;
    m_iResult = ERROR_SUCCESS;
    m_pDrive = NULL;
}

// The scan only fills the hash map - the window shows it after the thread is done.

void *CScanThread::Entry() {
    wxLogMessage("Scan of %s started.", m_sSource);
    DeleteFileInfoHash(m_pHash); // the files of a previous scan - the window has already emptied the list

    m_pDrive = OpenRecoveryDrive(m_sSource, m_iResult);
    if (m_pDrive && !m_bCancel) {
        wxLogMessage("%s is %s.", m_sSource, m_pDrive->FileSystemName());
        m_pDrive->SetCancelFlag(&m_bCancel);
        m_pDrive->SetScanProgress(&m_lPercent);
        m_iResult = m_pDrive->GetFileList(m_pHash);
        m_pDrive->SetCancelFlag(NULL);
        m_pDrive->SetScanProgress(NULL);
        wxLogMessage("Scan finished: %u deleted files found (result %d).", (unsigned) m_pHash->size(), m_iResult);
    } else if (m_pDrive) {
        m_iResult = ERROR_CANCELLED;
    } else {
        wxLogMessage("%s can't be opened (error %d).", m_sSource, m_iResult);
    }

    wxQueueEvent(m_pOwner, new wxThreadEvent(wxEVT_THREAD, THREAD_SCAN_DONE));
    return NULL;
}

/////////////////////////////////// recovery ///////////////////////////////////

CCopyThread::CCopyThread(wxEvtHandler *pOwner, CRecoveryDrive *pDrive, const std::vector<t_FileInfo *> &vFiles, const wxString &sDestinationDir)
: wxThread(wxTHREAD_JOINABLE) {
    m_pOwner = pOwner;
    m_pDrive = pDrive;
    m_vFiles = vFiles;
    m_sDestinationDir = sDestinationDir;
    m_bCancel = false;
    m_lKBWritten = 0;
    m_lCurrent = 0;
    m_iCopied = 0;
    m_bCancelled = false;
}

// why a file could not be recovered, in words for the end page

static wxString RecoveryErrorText(int iError, const t_FileInfo *pFile) {
    wxString sReason;
    switch (iError) {
        case ERROR_NOT_SUPPORTED:
            sReason = Tr(L"compressed or very fragmented file - not supported");
            break;
        case ERROR_FILE_CORRUPT:
        case ERROR_FILE_INVALID:
            sReason = Tr(L"its record on the drive is damaged");
            break;
        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:
            sReason = Tr(L"the destination drive is full");
            break;
        case ERROR_ACCESS_DENIED:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_FILE_EXISTS:
        case ERROR_CANNOT_MAKE:
            sReason.Printf(Tr(L"the file could not be created in the destination folder (error %d)"), iError);
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            sReason = Tr(L"not enough memory");
            break;
        default:
            sReason.Printf(Tr(L"the drive could not be read (error %d)"), iError);
    }
    if (pFile->iRecoverability == RECOVERABILITY_OVERWRITTEN)
        sReason += Tr(L"; it was marked as overwritten");
    return sReason;
}

// "photo.jpg", 2 -> "photo (2).jpg"; the number goes before the extension, so the file still opens
// with the right program. Names without an extension (or only a leading dot, ".config") get it at the end.

wxString NumberedFileName(const wxString &sName, int iNumber) {
    int iDot = sName.Find('.', true); // the last dot
    if (iDot <= 0)
        return wxString::Format("%s (%d)", sName, iNumber);
    return wxString::Format("%s (%d)%s", sName.Left(iDot), iNumber, sName.Mid(iDot));
}

void *CCopyThread::Entry() {
    wxLogMessage("Recovery of %u files to %s started.", (unsigned) m_vFiles.size(), m_sDestinationDir);
    // cancel stops a long file in the middle (checked per 1 MB), not only between files
    m_pDrive->SetCancelFlag(&m_bCancel);
    m_pDrive->SetProgressCounter(&m_lKBWritten);

    wxFileName destination;
    destination.AssignDir(m_sDestinationDir);

    for (size_t iFile = 0; iFile < m_vFiles.size(); iFile++) {
        if (m_bCancel)
            break;
        m_lCurrent = (LONG) iFile;
        t_FileInfo *pFile = m_vFiles[iFile];
        wxLogMessage("Recovering %s (%s).", wxString(pFile->sFilename.c_str()), RecoverabilityLabel(pFile->iRecoverability));

        // a name Windows accepts - names stay UTF-16 (wchar_t) all the way
        wchar_t wszName[_MAX_PATH];
        wcsncpy(wszName, pFile->sFilename.c_str(), _MAX_PATH - 1);
        wszName[_MAX_PATH - 1] = 0;
        IsValidFileName(wszName, true);
        destination.SetFullName(wxString(wszName));

        // a file of that name is already there (recovered before, or two deleted files with the same name):
        // number it the way Windows does - "photo (2).jpg", "photo (3).jpg"
        wxString sOriginalName(wszName);
        for (int iNumber = 2; destination.FileExists(destination.GetFullPath()); iNumber++)
            destination.SetFullName(NumberedFileName(sOriginalName, iNumber));

        // only that: the default flags also expand %VARIABLES% in the file name
        destination.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
        wxString sPath = destination.GetFullPath();

        int iRet = m_pDrive->RetreiveAndWriteFileToDestination(pFile->iRecordId, sPath.wc_str());
        if (iRet == ERROR_CANCELLED)
            break;
        if (iRet) {
            wxLogMessage("Recovering %s FAILED.", sPath);
            m_vFailures.push_back(wxString(pFile->sFilename.c_str()) + " - " + RecoveryErrorText(iRet, pFile));
        } else {
            m_iCopied++;
            wxLogMessage("Recovered to %s.", sPath);
        }
    }

    m_bCancelled = m_bCancel;
    m_pDrive->SetCancelFlag(NULL);
    m_pDrive->SetProgressCounter(NULL);
    wxLogMessage("Recovery %s: %d of %u files.", m_bCancelled ? "cancelled" : "finished", m_iCopied, (unsigned) m_vFiles.size());

    wxQueueEvent(m_pOwner, new wxThreadEvent(wxEVT_THREAD, THREAD_COPY_DONE));
    return NULL;
}
