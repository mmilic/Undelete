
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/filename.h>
#include <wx/listctrl.h>

#include "NTFSDrive.h"
#include "MyThreadClass.h"
#include "IsValidFileName.h"




#define MY_SHOW_ERROR_MESSAGE(Str)      {                                                               \
                                        wxString myErrorMessage;                                        \
                                        myErrorMessage.Printf(Str);\
                                                                                                        \
                                        wxMessageDialog *dial = new wxMessageDialog(NULL,               \
                                        myErrorMessage, wxT("Error"), wxOK | wxICON_ERROR);             \
                                        dial->ShowModal();                                              \
                                        }                                                               \
                                        
#define MY_SHOW_INFO_MESSAGE(Str)      {                                                                \
                                        wxString myErrorMessage;                                        \
                                        myErrorMessage.Printf(Str);\
                                                                                                        \
                                        wxMessageDialog *dial = new wxMessageDialog(NULL,               \
                                        myErrorMessage, wxT("Info"), wxOK | wxICON_INFORMATION);        \
                                        dial->ShowModal();                                              \
                                        }    

#define LOG(args...) if(log_file)    log_file << args << std::endl;

DEFINE_EVENT_TYPE(wxEVT_SHOW_NOTIFICATION)
DEFINE_EVENT_TYPE(wxEVT_ENABLE_GUI_FIELDS)

//First define the thread classes. In some other versions, this should probably go into a different file so that it is more readoble, but for now, it stays here.

void MyThreadClass::Init() {
    m_count = 0;
    m_pUsedNTFSDrive = NULL;
    m_finfohash = NULL;
    m_parent = NULL; //treba mi da bi ovaj thread mogao da baca evente
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
// has really stopped before another one can start on the same CNTFSDrive and hash map

MyThreadClass::MyThreadClass()
: wxThread(wxTHREAD_JOINABLE) {
    Init();
}

//MyThreadClass::MyThreadClass(wxFrame *parent) : wxThread() {
//    Init();
//    m_parent = parent; //treba mi da bi ovaj thread mogao da baca evente
//}

MyThreadClass::MyThreadClass(wxWindow *parent) : wxThread(wxTHREAD_JOINABLE) {
    Init();
    m_parent = parent; //treba mi da bi ovaj thread mogao da baca evente
}

void MyThreadClass::SetNTFSDrivePtr(CNTFSDrive *ptr) {
    m_pUsedNTFSDrive = ptr;
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

void MyThreadClass::SetMutexLockPtr(wxMutex *ptr) {
    m_LockPtr = ptr;
}

void MyThreadClass::SetLogHndl(HANDLE hFile) {
    hFileLog = hFile;
}

MyThreadClass::~MyThreadClass() {
}

MyThreadLoadFilesFromDiskNTFS::MyThreadLoadFilesFromDiskNTFS(wxWindow *parent)
: MyThreadClass(parent) {
}

//MyThreadSearchAction::MyThreadSearchAction(wxWindow *parent)
//: MyThreadClass(parent) {
//}

// ovaj ne sluzi nicemu, moze i da se obrise

void MyThreadClass::ThrowInfoEvent(wxString wxstrMessage) {
    wxCommandEvent event(wxEVT_SHOW_NOTIFICATION, m_parent->GetId());

    //Give it some contents
    event.SetString(wxstrMessage);
    // Send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event)) {
        wxLogMessage("Unable to send event: wxEVT_SHOW_NOTIFICATION");
    }
}

// ovaj ne sluzi nicemu, moze i da se obrise

void MyThreadClass::ThrowEnableGuiEvent() {
    wxCommandEvent event(wxEVT_ENABLE_GUI_FIELDS, m_parent->GetId());

    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        printf("sranje");
}

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

void MyThreadClass::ThrowSelectFilesDialogPulseEvent(wxString wxstrMessage) {
    wxCommandEvent event(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG, m_parent->GetId());

    // Give it some contents
    event.SetString(wxstrMessage);

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogMessage("Unable to send event: wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG");
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

void MyThreadClass::ThrowSearchActionEvent() {
    wxCommandEvent event(wxEVT_SELECT_FILES_SEARCH_ACTION, m_parent->GetId());

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
    m_pUsedNTFSDrive->SetCancelFlag(&m_CancelProgresControl);

    // the files of a previous scan - the page has already emptied the list that pointed to them
    DeleteFileInfoHash(m_finfohash);

    ThrowSelectDestinationDrivePulseEvent("Reading Volume.\nThis can take several minutes..."); // update progress bar message ;)
    wxLogMessage("Started reading volume %s", *m_wxpSourceDrive);
    m_iResult = m_pUsedNTFSDrive->ReadVolume(*m_wxpSourceDrive);
    if (m_iResult) {
        wxLogMessage("ReadVolume failed with error %d.", m_iResult);
    } else if (!IsCancelProgresControl()) { //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel
        wxLogMessage("Completed reading volume %s", *m_wxpSourceDrive);
        ThrowSelectDestinationDrivePulseEvent("Creating deleted files list.\nThis can take several minutes...");

        wxLogMessage("Creating deleted files list...");
        m_iResult = m_pUsedNTFSDrive->GetFileList(m_finfohash);
        wxLogMessage("Completed creating deleted files list: %u deleted files found (result %d).", (unsigned) m_finfohash->size(), m_iResult);
    }

    m_pUsedNTFSDrive->SetCancelFlag(NULL);
    ThrowSelectDestinationDriveStopEvent();
    return (0);
}

void *MyThreadLoadFilesFromDiskNTFS::Entry() { //ovde pocinje izvrsavanje threada
    return ScanVolume();
}

void *MyThreadClass::Entry() {
    return (NULL);
}

MyThreadCopyFiles::MyThreadCopyFiles() : MyThreadClass() {
}

MyThreadCopyFiles::MyThreadCopyFiles(wxWindow * parent) : MyThreadClass(parent) {
}

void MyThreadCopyFiles::SetDestTextBox(wxTextCtrl * ptr) {
    m_DestTextBox = ptr;
}

void MyThreadCopyFiles::SetMwPtr_strSelectedDir(wxString * mwPtr_strSelectedDir) {
    this->mwPtr_strSelectedDir = mwPtr_strSelectedDir;
}

wxString * MyThreadCopyFiles::GetMwPtr_strSelectedDir() const {
    return mwPtr_strSelectedDir;
}

// the files to copy are collected by the GUI thread before this thread starts - the thread must not read the list control

void MyThreadCopyFiles::SetFilesToCopy(const std::vector<t_FileInfo *> &vFiles) {
    m_vFilesToCopy = vFiles;
}

void *MyThreadCopyFiles::Entry() {
    wxLogMessage("Started execution of ThreadCopyFiles");
    // cancel stops a long file in the middle (checked per 1 MB), not only between files
    m_pUsedNTFSDrive->SetCancelFlag(&m_CancelProgresControl);

    wxFileName wxDestinationFileName;
    wxDestinationFileName.AssignDir(*m_wxpDestinationDir);

    t_FileInfo *pTempFInfo;
    int iCount = 0;
    int iFileNo = 0;

    *m_iptrNumberOfCopiedFiles = 0;

    for (size_t iFile = 0; iFile < m_vFilesToCopy.size(); iFile++) {
        pTempFInfo = m_vFilesToCopy[iFile];

        // this item is selected - do whatever is needed with it
        wxLogMessage("Item %s is selected (%s).", wxString(pTempFInfo->sFilename.c_str()), RecoverabilityLabel(pTempFInfo->iRecoverability));

        // Prepare destination and file name

        //Ovde se odredjuje kompletno ime fajla, tu treba da se proba da li postoji ili ne :)
        //Ovo bi moglo pedantnije da se uradi, da da novi broj samo ako postoji fajl sa tim imenom
        //Zbog toga u ovom prvom postavljanju imena fajla nema iCount variable - prvo ime je originalno a uvek posle se vrsi dodavanje broja.

        //ovde proveravamo da li je dobro ime i odmah ga menjamo - names stay UTF-16 (wchar_t) all the way
        wchar_t pFullName[_MAX_PATH];
        wcsncpy(pFullName, pTempFInfo->sFilename.c_str(), _MAX_PATH - 1);
        pFullName[_MAX_PATH - 1] = 0;
        //ovde se bas vrsi provera i menjanje imena fajla (i extenzije - path se ne menja)
        IsValidFileName(pFullName, true);
        wxDestinationFileName.SetFullName(wxString(pFullName));

        //sta ce mi ovaj wxsCompleteFileName - nemam pojma - al reko lakse je 'vako
        wxString wxsCompleteFileName;
        while (wxDestinationFileName.FileExists(wxDestinationFileName.GetFullPath())) {
            iCount++;
            wxsCompleteFileName.Printf("%d_%s", iCount, wxDestinationFileName.GetFullName());
            wxDestinationFileName.SetFullName(wxsCompleteFileName);
        }

        wxDestinationFileName.Normalize(); //skidamo duple // simbole i slicno

        wxString wxsDestinationPath = wxDestinationFileName.GetFullPath();

        ThrowSelectFilesDialogPulseEvent(wxDestinationFileName.GetFullName(), ++iFileNo);
        wxLogMessage("Started Retrieving of %s .", wxString(pTempFInfo->sFilename.c_str()));
        wxLogMessage("Will be moved to: %s", wxsDestinationPath);
        if (IsCancelProgresControl()) //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel
            break;

        int iRet = m_pUsedNTFSDrive->RetreiveAndWriteFileToDestination(pTempFInfo->iRecordId, wxsDestinationPath.wc_str());
        if (iRet)
            wxLogMessage("Undeleting of %s FAILED.", wxsDestinationPath);
        else {
            (*m_iptrNumberOfCopiedFiles)++;
            wxLogMessage("Undeleting of %s SUCCESSFULL.", wxsDestinationPath);
        }
        iCount++;
    }

    m_pUsedNTFSDrive->SetCancelFlag(NULL);
    ThrowSelectFilesDialogStopEvent();
    wxLogMessage("%s\nCheck %s", IsCancelProgresControl() ? "CANCELLED!" : "FINISHED!", wxDestinationFileName.GetPath());

    return (NULL);
}

MyThreadLoadFilesFromDiskFAT::MyThreadLoadFilesFromDiskFAT(wxWindow *parent)
: MyThreadClass(parent) {
}

void *MyThreadLoadFilesFromDiskFAT::Entry() { //ovde pocinje izvrsavanje threada
    return ScanVolume();
}

//TODO popravi mesta gde pise sranje
//TODO razvoji thread akcije u razlicite fajlove
