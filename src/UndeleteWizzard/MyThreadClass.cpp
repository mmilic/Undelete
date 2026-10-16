
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/filename.h>
#include <wx/listctrl.h>

#include "RecoveryDrive.h"
#include "MyThreadClass.h"
#include "IsValidFileName.h"

//First define the thread classes. In some other versions, this should probably go into a different file so that it is more readoble, but for now, it stays here.

void MyThreadClass::Init() {
    m_pDrive = NULL;
    m_finfohash = NULL;
    m_wxpDestinationDir = NULL;
    m_wxpSourceDrive = NULL;
    m_parent = NULL; //treba mi da bi ovaj thread mogao da baca evente
    m_iptrNumberOfCopiedFiles = NULL;
    m_CancelProgresControl = false;
    m_iResult = ERROR_SUCCESS;
}

void MyThreadClass::SetCancelProgresControl(bool CancelProgresControl) {
    this->m_CancelProgresControl = CancelProgresControl;
}

bool MyThreadClass::IsCancelProgresControl() const {
    return m_CancelProgresControl;
}

void MyThreadClass::SetIptrNumberOfCopiedFiles(int* iptrNumberOfCopiedFiles) {
    this->m_iptrNumberOfCopiedFiles = iptrNumberOfCopiedFiles;
}

int* MyThreadClass::GetIptrNumberOfCopiedFiles() const {
    return m_iptrNumberOfCopiedFiles;
}

// joinable: the page that starts a thread waits for it (Wait()) and deletes it, so a cancelled scan or copy
// has really stopped before another one can start on the same drive engine and hash map

MyThreadClass::MyThreadClass()
: wxThread(wxTHREAD_JOINABLE) {
    Init();
}

MyThreadClass::MyThreadClass(wxWindow *parent) : wxThread(wxTHREAD_JOINABLE) {
    Init();
    m_parent = parent; //treba mi da bi ovaj thread mogao da baca evente
}

void MyThreadClass::SetDrivePtr(CRecoveryDrive *ptr) {
    m_pDrive = ptr;
}

void MyThreadClass::SetFinfoHashPtr(FileInfoHash *ptr) {
    m_finfohash = ptr;
}

void MyThreadClass::SetDestinationDirPtr(wxString *ptr) {
    m_wxpDestinationDir = ptr;
}

void MyThreadClass::SetSourceDrive(wxString *ptr) {
    m_wxpSourceDrive = ptr;
}

MyThreadClass::~MyThreadClass() {
}

MyThreadScanVolume::MyThreadScanVolume(wxWindow *parent)
: MyThreadClass(parent) {
}

// The events below are sent with ProcessEvent, so their handlers run in this (worker) thread: the handlers only
// store values, behind a lock where needed (see the pages' SetDialogString).

void MyThreadClass::ThrowSelectDestinationDrivePulseEvent(wxString wxstrMessage) {
    wxCommandEvent event(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_PULSE_DIALOG, m_parent->GetId());

    // Give it some contents
    event.SetString(wxstrMessage);

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogMessage("Unable to send event: wxEVT_SELECT_DESTINATION_DRIVE_PAGE_PULSE_DIALOG");
}

void MyThreadClass::ThrowSelectDestinationDriveStopEvent() {
    wxCommandEvent event(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG, m_parent->GetId());

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogMessage("Unable to send event: wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG");
}

void MyThreadClass::ThrowSelectFilesDialogPulseEvent(wxString wxstrMessage, int iCount) {
    wxCommandEvent event(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG, m_parent->GetId());

    // Give it some contents
    event.SetString(wxstrMessage);
    event.SetInt(iCount);

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogMessage("Unable to send event: wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG");
}

void MyThreadClass::ThrowSelectFilesDialogStopEvent() {
    wxCommandEvent event(wxEVT_SELECT_FILES_PAGE_STOP_DIALOG, m_parent->GetId());

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogMessage("Unable to send event: wxEVT_SELECT_FILES_PAGE_STOP_DIALOG");
}

// The scan only fills the hash map. The list control belongs to the GUI thread - wxWidgets controls must
// not be touched from worker threads - so the page fills it (wxSelectDestinationDirPage::OnWizardPageChanging)
// after this thread signals the end with ThrowSelectDestinationDriveStopEvent.

// The result is in m_iResult (ERROR_SUCCESS, ERROR_CANCELLED or the error that stopped the scan); the page
// reads it after Wait() and shows the message - no message boxes from this thread.

void *MyThreadClass::ScanVolume() {
    wxLogMessage("LoadFiles Thread started executing.");
    m_pDrive->SetCancelFlag(&m_CancelProgresControl);

    // the files of a previous scan - the page has already emptied the list that pointed to them
    DeleteFileInfoHash(m_finfohash);

    ThrowSelectDestinationDrivePulseEvent("Reading Volume.\nThis can take several minutes..."); // update progress bar message ;)
    wxLogMessage("Started reading volume %s (%s)", *m_wxpSourceDrive, m_pDrive->FileSystemName());
    m_iResult = m_pDrive->ReadVolume(*m_wxpSourceDrive);
    if (m_iResult) {
        wxLogMessage("ReadVolume failed with error %d.", m_iResult);
    } else if (!IsCancelProgresControl()) { //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel
        wxLogMessage("Completed reading volume %s", *m_wxpSourceDrive);
        ThrowSelectDestinationDrivePulseEvent("Creating deleted files list.\nThis can take several minutes...");

        wxLogMessage("Creating deleted files list...");
        m_iResult = m_pDrive->GetFileList(m_finfohash);
        wxLogMessage("Completed creating deleted files list: %u deleted files found (result %d).", (unsigned) m_finfohash->size(), m_iResult);
    }

    m_pDrive->SetCancelFlag(NULL);
    ThrowSelectDestinationDriveStopEvent();
    return (0);
}

void *MyThreadScanVolume::Entry() { //ovde pocinje izvrsavanje threada
    return ScanVolume();
}

void *MyThreadClass::Entry() {
    return (NULL);
}

MyThreadCopyFiles::MyThreadCopyFiles() : MyThreadClass() {
}

MyThreadCopyFiles::MyThreadCopyFiles(wxWindow * parent) : MyThreadClass(parent) {
}

// why a file could not be recovered, in words for the end page

static wxString RecoveryErrorText(int iError, const t_FileInfo *pFile) {
    wxString sReason;
    switch (iError) {
        case ERROR_NOT_SUPPORTED:
            sReason = "compressed or very fragmented file - not supported";
            break;
        case ERROR_FILE_CORRUPT:
        case ERROR_FILE_INVALID:
            sReason = "its record on the drive is damaged";
            break;
        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:
            sReason = "the destination drive is full";
            break;
        case ERROR_ACCESS_DENIED:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_FILE_EXISTS:
        case ERROR_CANNOT_MAKE:
            sReason.Printf("the file could not be created in the destination folder (error %d)", iError);
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            sReason = "not enough memory";
            break;
        default:
            sReason.Printf("the drive could not be read (error %d)", iError);
    }
    if (pFile->iRecoverability == RECOVERABILITY_OVERWRITTEN)
        sReason += "; it was marked as overwritten";
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

// the files to copy are collected by the GUI thread before this thread starts - the thread must not read the list control

void MyThreadCopyFiles::SetFilesToCopy(const std::vector<t_FileInfo *> &vFiles) {
    m_vFilesToCopy = vFiles;
}

void *MyThreadCopyFiles::Entry() {
    wxLogMessage("Started execution of ThreadCopyFiles");
    // cancel stops a long file in the middle (checked per 1 MB), not only between files
    m_pDrive->SetCancelFlag(&m_CancelProgresControl);
    m_lKBWritten = 0;
    m_vFailures.clear();
    m_pDrive->SetProgressCounter(&m_lKBWritten);

    wxFileName wxDestinationFileName;
    wxDestinationFileName.AssignDir(*m_wxpDestinationDir);

    t_FileInfo *pTempFInfo;
    int iFileNo = 0;

    *m_iptrNumberOfCopiedFiles = 0;

    for (size_t iFile = 0; iFile < m_vFilesToCopy.size(); iFile++) {
        pTempFInfo = m_vFilesToCopy[iFile];

        // this item is selected - do whatever is needed with it
        wxLogMessage("Item %s is selected (%s).", wxString(pTempFInfo->sFilename.c_str()), RecoverabilityLabel(pTempFInfo->iRecoverability));

        // Prepare destination and file name

        //ovde proveravamo da li je dobro ime i odmah ga menjamo - names stay UTF-16 (wchar_t) all the way
        wchar_t pFullName[_MAX_PATH];
        wcsncpy(pFullName, pTempFInfo->sFilename.c_str(), _MAX_PATH - 1);
        pFullName[_MAX_PATH - 1] = 0;
        //ovde se bas vrsi provera i menjanje imena fajla (i extenzije - path se ne menja)
        IsValidFileName(pFullName, true);
        wxDestinationFileName.SetFullName(wxString(pFullName));

        // a file of that name is already there (recovered before, or two deleted files with the same name):
        // number it the way Windows does - "photo (2).jpg", "photo (3).jpg"
        wxString sOriginalName(pFullName);
        for (int iNumber = 2; wxDestinationFileName.FileExists(wxDestinationFileName.GetFullPath()); iNumber++)
            wxDestinationFileName.SetFullName(NumberedFileName(sOriginalName, iNumber));

        // skidamo duple // simbole i slicno - only that: the default flags also expand %VARIABLES% in the file name
        wxDestinationFileName.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);

        wxString wxsDestinationPath = wxDestinationFileName.GetFullPath();

        ThrowSelectFilesDialogPulseEvent(wxDestinationFileName.GetFullName(), ++iFileNo);
        wxLogMessage("Started Retrieving of %s .", wxString(pTempFInfo->sFilename.c_str()));
        wxLogMessage("Will be moved to: %s", wxsDestinationPath);
        if (IsCancelProgresControl()) //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel
            break;

        int iRet = m_pDrive->RetreiveAndWriteFileToDestination(pTempFInfo->iRecordId, wxsDestinationPath.wc_str());
        if (iRet == ERROR_CANCELLED)
            break;
        if (iRet) {
            wxLogMessage("Undeleting of %s FAILED.", wxsDestinationPath);
            m_vFailures.push_back(wxString(pTempFInfo->sFilename.c_str()) + " - " + RecoveryErrorText(iRet, pTempFInfo));
        } else {
            (*m_iptrNumberOfCopiedFiles)++;
            wxLogMessage("Undeleting of %s SUCCESSFULL.", wxsDestinationPath);
        }
    }

    m_pDrive->SetCancelFlag(NULL);
    m_pDrive->SetProgressCounter(NULL);
    ThrowSelectFilesDialogStopEvent();
    wxLogMessage("%s\nCheck %s", IsCancelProgresControl() ? "CANCELLED!" : "FINISHED!", wxDestinationFileName.GetPath());

    return (NULL);
}


//TODO popravi mesta gde pise sranje
//TODO razvoji thread akcije u razlicite fajlove
