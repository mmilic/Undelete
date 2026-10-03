
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

MyThreadClass::MyThreadClass()
: wxThread() {
    Init();
}

//MyThreadClass::MyThreadClass(wxFrame *parent) : wxThread() {
//    Init();
//    m_parent = parent; //treba mi da bi ovaj thread mogao da baca evente
//}

MyThreadClass::MyThreadClass(wxWindow *parent) : wxThread() {
    Init();
    m_parent = parent; //treba mi da bi ovaj thread mogao da baca evente
}

void MyThreadClass::SetNTFSDrivePtr(CNTFSDrive *ptr) {
    m_pUsedNTFSDrive = ptr;
}

void MyThreadClass::SetFinfoHashPtr(FileInfoHash *ptr) {
    m_finfohash = ptr;
}

void MyThreadClass::SetListControlPtr(wxListCtrl *ptr) {
    m_ListControl = ptr;
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
        wxLogFatalError("An error occurred during thread creation.\nThe program will exit now.");
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
        wxLogFatalError("Unable to send event: wxEVT_SELECT_DESTINATION_DRIVE_PAGE_PULSE_DIALOG");
}

void MyThreadClass::ThrowSelectDestinationDriveStopEvent() {
    wxCommandEvent event(wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG, m_parent->GetId());

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogFatalError("Unable to send event: wxEVT_SELECT_DESTINATION_DRIVE_PAGE_STOP_DIALOG");
}

void MyThreadClass::ThrowSelectFilesDialogPulseEvent(wxString wxstrMessage) {
    wxCommandEvent event(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG, m_parent->GetId());

    // Give it some contents
    event.SetString(wxstrMessage);

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogFatalError("Unable to send event: wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG");
}

void MyThreadClass::ThrowSelectFilesDialogPulseEvent(wxString wxstrMessage, int iCount) {
    wxCommandEvent event(wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG, m_parent->GetId());

    // Give it some contents
    event.SetString(wxstrMessage);
    event.SetInt(iCount);

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogFatalError("Unable to send event: wxEVT_SELECT_FILES_PAGE_PULSE_DIALOG");
}

void MyThreadClass::ThrowSelectFilesDialogStopEvent() {
    wxCommandEvent event(wxEVT_SELECT_FILES_PAGE_STOP_DIALOG, m_parent->GetId());

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogFatalError("Unable to send event: wxEVT_SELECT_FILES_PAGE_STOP_DIALOG");
}

void MyThreadClass::ThrowSearchActionEvent() {
    wxCommandEvent event(wxEVT_SELECT_FILES_SEARCH_ACTION, m_parent->GetId());

    // Do send it
    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        wxLogFatalError("Unable to send event: wxEVT_SELECT_FILES_PAGE_STOP_DIALOG");
}

void MyThreadLoadFilesFromDiskNTFS::FillListControlFromHashMap() {
    if (m_finfohash->empty()) {
        wxString myErrorMessage;
        myErrorMessage.Printf("There were no deleted files  found on this drive.");

        wxMessageDialog *dial = new wxMessageDialog(NULL,
                myErrorMessage, wxT("Info"), wxOK | wxICON_ERROR);
        dial->ShowModal();
    }
    wxLogMessage("Found %d deleted files.", m_finfohash->size());

    int listCounter = 30; // zato sto sam stavio da od toliko krene da skenira fajlove - sad ajd - pamti napamet dok neko ne stavi macro
    m_ListControl->ClearAll();

    //    m_ListControl->InsertColumn(0, wxT("MFT Record ID"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(0, wxT("File Name"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(1, wxT("File Size"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(2, wxT("File Status"), wxLIST_FORMAT_LEFT, -1);

    // to speed up inserting we hide the control temporarily
    m_ListControl->Hide();

    // ovde sad ide ona funkcija koja trpa u listu
    wxLogMessage("Populating file list.");
    wxString buf;
    FileInfoHash::iterator it;
    t_FileInfo *pTempFinfo = NULL;
    for (it = m_finfohash->begin(); it != m_finfohash->end(); ++it) {
        pTempFinfo = it->second;

        //        buf.Printf("%d", pTempFinfo->iRecordId);
        //        long lIndex = m_ListControl->InsertItem(listCounter, buf, 0);


        //nesto me jebalo sa regex-ima pa cu preci na glupi strncmp
        //obzirom da se sve operacije za regex nalaze u std::tr1 namespace-u - malo je ruzno ovako al jbg
        //        std::tr1::regex regPattern("[$][R][A-Z0-9](6)");
        //        
        //        if(std::tr1::regex_match(pTempFinfo->szFilename, regPattern))
        //            printf("radi");

        char tmp[2] = "."; //glupo resenje
        if (!strncmp(pTempFinfo->szFilename, "$R", 2) && (pTempFinfo->szFilename)[8] == tmp[0]) {
            wxLogMessage("A file deleted from Recycle Bin is found.");

            //Nadji da li postoji fajl koji se zove isto kao ovaj, ali da pocinje na $I umesto na $R
            char nameFile[12 + 1];
            memset(nameFile, '\0', 13 * sizeof (char));

            strncpy(nameFile, "$I", 2);
            strncat(nameFile, pTempFinfo->szFilename + 2, 10);

            // ime ovoga koji trazimo je u nameFile, sad ludujemo kroz hash tabelu

            FileInfoHash::iterator innerIterator;
            t_FileInfo *pInnerTempFinfo = NULL;
            BYTE *resultBuffer;
            DWORD dwStringSize;
            char tempCharBuffer[MAX_PATH];
            char *ptrCurrentToken;
            char *ptrPreviousToken;

            int nRet;
            wxLogMessage("Trying to find a file name for this file.");
            for (innerIterator = m_finfohash->begin(); innerIterator != m_finfohash->end(); ++innerIterator) {
                pInnerTempFinfo = innerIterator->second;
                if (!strncmp(pInnerTempFinfo->szFilename, nameFile, 12)) {
                    nRet = m_pUsedNTFSDrive->Read_File(pInnerTempFinfo->iRecordId, resultBuffer, dwStringSize);
                    if (nRet) {
                        wxLogMessage("Error when trying to retrieve contents of %s", nameFile);
                        break;
                    }
                    memset(pTempFinfo->szFilename, '\0', MAX_PATH * sizeof (char));
                    memset(tempCharBuffer, '\0', MAX_PATH * sizeof (char));
                    wcstombs(tempCharBuffer, (wchar_t*)(resultBuffer + 24), MAX_PATH);

                    ptrCurrentToken = strtok(tempCharBuffer, "\\");
                    ptrPreviousToken = ptrCurrentToken;
                    while (ptrCurrentToken != NULL) {
                        ptrPreviousToken = ptrCurrentToken;
                        ptrCurrentToken = strtok(NULL, "\\");
                    }

                    strncpy(pTempFinfo->szFilename, ptrPreviousToken, strlen(ptrPreviousToken));
                    strcpy(pTempFinfo->szBuffer, "Deleted in Recycle Bin");
                    wxLogMessage("A file name retrieved from Recycle Bin.");
                    break;
                }
                //                else
                //                    wxLogMessage("A file name was not retrieved.");
            }
        }
//ovde ubaci u neku listu - neku jednostavniju globalnu strukturu
        //i onda popunjavaj u zavisnosti od searcha 
        long lIndex = m_ListControl->InsertItem(listCounter, pTempFinfo->szFilename, 0);
        m_ListControl->SetItemPtrData(lIndex, (wxUIntPtr) pTempFinfo);
        double llSizeInMB = pTempFinfo->n64SizeOnDisk;
        llSizeInMB /= (1024 * 1024);  //in MB
        char sSize[64] = {};
        sprintf(sSize, "%.2f MB", llSizeInMB);        
        m_ListControl->SetItem(lIndex, 1, wxString(sSize));
        m_ListControl->SetItem(lIndex, 2, pTempFinfo->szBuffer);
        listCounter++;

    }
    wxLogMessage("Populating file list completed.");

    m_ListControl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_ListControl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    //    m_ListControl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER); // bilo je wxLIST_AUTOSIZE
    m_ListControl->RefreshItem(0);
    m_ListControl->Show();
}

void *MyThreadLoadFilesFromDiskNTFS::Entry() { //ovde pocinje izvrsavanje threada 

    // ovo nicemu ne sluzi - verovatno ce sve da se brise   
    // wxMutexLocker lock(*m_LockPtr); 
    int iRet;

    wxLogMessage("LoadFiles Thread started executing.");
    ThrowSelectDestinationDrivePulseEvent("Reading Volume.\nThis can take several minutes..."); // update progress bar message ;)

    wxLogMessage("Started reading volume %s", m_wxpSourceDrive->mb_str());
    iRet = m_pUsedNTFSDrive->ReadVolume(*m_wxpSourceDrive);
    if (iRet) {
        wxLogFatalError("ReadVolume failed. Please check access rights and restart the program");
    }
    wxLogMessage("Completed reading volume %s", m_wxpSourceDrive->mb_str());

    if (IsCancelProgresControl()) return (NULL); //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel
    ThrowSelectDestinationDrivePulseEvent("Creating deleted files list.\nThis can take several minutes...");

    wxLogMessage("Creating deleted files list...");
    m_finfohash->clear(); //ocisti hash za svaki slucaj :)    
    m_pUsedNTFSDrive->GetFileList(m_finfohash);
    wxLogMessage("Completed creating deleted files list.\nThis can take several minutes...");

    ThrowSelectDestinationDrivePulseEvent("Loading deleted files list.\nThis can take several minutes...");
    if (IsCancelProgresControl()) return (NULL); //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel

    wxLogMessage("Filling list control from Hash map");
    FillListControlFromHashMap();
    wxLogMessage("Completed filling list control from Hash map");

    ThrowSelectDestinationDriveStopEvent();

    return (0);
}
//
//MyThreadLoadFilesFromDiskNTFS::~MyThreadLoadFilesFromDiskNTFS() {
//
//}

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

void *MyThreadCopyFiles::Entry() {
    wxLogMessage("Started execution of ThreadCopyFiles");

    wxFileName wxDestinationFileName;
    wxDestinationFileName.AssignDir(*m_wxpDestinationDir);

    long item = -1;
    t_FileInfo *pTempFInfo;
    int iCount = 0;
    int iFileNo = 0;

    *m_iptrNumberOfCopiedFiles = 0;

    for (;;) {
        item = m_ListControl->GetNextItem(item,
                wxLIST_NEXT_ALL,
                wxLIST_STATE_SELECTED);
        if (item == -1) {
            wxLogMessage("No more selected items.");
            break;
        }

        pTempFInfo = (t_FileInfo *) m_ListControl->GetItemData(item);

        // this item is selected - do whatever is needed with it
        wxLogMessage("Item %s is selected.", pTempFInfo->szFilename);

        // Prepare destination and file name

        //Ovde se odredjuje kompletno ime fajla, tu treba da se proba da li postoji ili ne :)
        //Ovo bi moglo pedantnije da se uradi, da da novi broj samo ako postoji fajl sa tim imenom
        //Zbog toga u ovom prvom postavljanju imena fajla nema iCount variable - prvo ime je originalno a uvek posle se vrsi dodavanje broja.

        wxDestinationFileName.SetFullName(pTempFInfo->szFilename);

        //ovde proveravamo da li je dobro ime i odmah ga menjamo        
        char pFullName[1024];
        memset(pFullName, '\0', 1024 * sizeof (char));
        strncpy(pFullName, wxDestinationFileName.GetFullName().mb_str(), wxDestinationFileName.GetFullName().Len());
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

        //ovo uvodjenje promenljive sCompleteFileName je zato sto me mrzi da razmotavam kako da koristim nesto drugo umesto toga (consta char* -> char*)
        char sCompleteFileName[512];
        memset(sCompleteFileName, '\0', 512 * sizeof (char));
        strncpy(sCompleteFileName, wxDestinationFileName.GetFullPath().mb_str(), 512);

        ThrowSelectFilesDialogPulseEvent(wxsCompleteFileName, ++iFileNo);
        wxLogMessage("Started Retrieving of %s .", pTempFInfo->szFilename);
        wxLogMessage("Will be moved to: %s", sCompleteFileName);
        if (IsCancelProgresControl()) return (NULL); //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel

        int iRet = m_pUsedNTFSDrive->RetreiveAndWriteFileToDestination(pTempFInfo->iRecordId, sCompleteFileName);
        if (iRet)
            wxLogMessage("Undeleting of %s FAILED.", sCompleteFileName);
        else {
            (*m_iptrNumberOfCopiedFiles)++;
            wxLogMessage("Undeleting of %s SUCCESSFULL.", sCompleteFileName);
        }
        iCount++;
    }

    ThrowSelectFilesDialogStopEvent();
    wxLogMessage("FINISHED!\nCheck %s", wxDestinationFileName.GetPath().mb_str());

    return (NULL);
}

void MyThreadSearchAction::FillListControlFromHashMap() {
    if (m_finfohash->empty()) {
        wxString myErrorMessage;
        myErrorMessage.Printf("There were no deleted files  found on this drive.");

        wxMessageDialog *dial = new wxMessageDialog(NULL,
                myErrorMessage, wxT("Info"), wxOK | wxICON_ERROR);
        dial->ShowModal();
    }
    wxLogMessage("Started Searching with \'%s\'", m_finfohash->size());
    
    m_ListControl->ClearAll();   
    m_ListControl->InsertColumn(0, wxT("File name"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(1, wxT("Status"), wxLIST_FORMAT_LEFT, -1);
    // to speed up inserting we hide the control temporarily
    m_ListControl->Hide();

    // ovde sad ide ona funkcija koja trpa u listu
    wxLogMessage("Populating file list with search results.");
    int listCounter = 30;
    FileInfoHash::iterator it;
    t_FileInfo *pTempFinfo = NULL;
    for (it = m_finfohash->begin(); it != m_finfohash->end(); ++it) {
        pTempFinfo = it->second;
        long lIndex = m_ListControl->InsertItem(listCounter, pTempFinfo->szFilename, 0);
        m_ListControl->SetItemPtrData(lIndex, (wxUIntPtr) pTempFinfo);
        m_ListControl->SetItem(lIndex, 1, pTempFinfo->szBuffer);
        listCounter++;

    }
    wxLogMessage("Populating file list completed.");

    m_ListControl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_ListControl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    //    m_ListControl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER); // bilo je wxLIST_AUTOSIZE
    m_ListControl->RefreshItem(0);
    m_ListControl->Show();
}

MyThreadLoadFilesFromDiskFAT::MyThreadLoadFilesFromDiskFAT(wxWindow *parent)
: MyThreadClass(parent) {
}

void MyThreadLoadFilesFromDiskFAT::FillListControlFromHashMap() {
    if (m_finfohash->empty()) {
        wxString myErrorMessage;
        myErrorMessage.Printf("There were no deleted files  found on this drive.");

        wxMessageDialog *dial = new wxMessageDialog(NULL,
                myErrorMessage, wxT("Info"), wxOK | wxICON_ERROR);
        dial->ShowModal();
    }
    wxLogMessage("Found %d deleted files.", m_finfohash->size());

    int listCounter = 30; // zato sto sam stavio da od toliko krene da skenira fajlove - sad ajd - pamti napamet dok neko ne stavi macro
    m_ListControl->ClearAll();

    //    m_ListControl->InsertColumn(0, wxT("MFT Record ID"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(0, wxT("File Name"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(1, wxT("File Size"), wxLIST_FORMAT_LEFT, -1);
    m_ListControl->InsertColumn(2, wxT("File Status"), wxLIST_FORMAT_LEFT, -1);

    // to speed up inserting we hide the control temporarily
    m_ListControl->Hide();

    // ovde sad ide ona funkcija koja trpa u listu
    wxLogMessage("Populating file list.");
    wxString buf;
    FileInfoHash::iterator it;
    t_FileInfo *pTempFinfo = NULL;
    for (it = m_finfohash->begin(); it != m_finfohash->end(); ++it) {
        pTempFinfo = it->second;

        //        buf.Printf("%d", pTempFinfo->iRecordId);
        //        long lIndex = m_ListControl->InsertItem(listCounter, buf, 0);


        //nesto me jebalo sa regex-ima pa cu preci na glupi strncmp
        //obzirom da se sve operacije za regex nalaze u std::tr1 namespace-u - malo je ruzno ovako al jbg
        //        std::tr1::regex regPattern("[$][R][A-Z0-9](6)");
        //        
        //        if(std::tr1::regex_match(pTempFinfo->szFilename, regPattern))
        //            printf("radi");

        char tmp[2] = "."; //glupo resenje
        if (!strncmp(pTempFinfo->szFilename, "$R", 2) && (pTempFinfo->szFilename)[8] == tmp[0]) {
            wxLogMessage("A file deleted from Recycle Bin is found.");

            //Nadji da li postoji fajl koji se zove isto kao ovaj, ali da pocinje na $I umesto na $R
            char nameFile[12 + 1];
            memset(nameFile, '\0', 13 * sizeof (char));

            strncpy(nameFile, "$I", 2);
            strncat(nameFile, pTempFinfo->szFilename + 2, 10);

            // ime ovoga koji trazimo je u nameFile, sad ludujemo kroz hash tabelu

            FileInfoHash::iterator innerIterator;
            t_FileInfo *pInnerTempFinfo = NULL;
            BYTE *resultBuffer;
            DWORD dwStringSize;
            char tempCharBuffer[MAX_PATH];
            char *ptrCurrentToken;
            char *ptrPreviousToken;

            int nRet;
            wxLogMessage("Trying to find a file name for this file.");
            for (innerIterator = m_finfohash->begin(); innerIterator != m_finfohash->end(); ++innerIterator) {
                pInnerTempFinfo = innerIterator->second;
                if (!strncmp(pInnerTempFinfo->szFilename, nameFile, 12)) {
                    nRet = m_pUsedNTFSDrive->Read_File(pInnerTempFinfo->iRecordId, resultBuffer, dwStringSize);
                    if (nRet) {
                        wxLogMessage("Error when trying to retrieve contents of %s", nameFile);
                        break;
                    }
                    memset(pTempFinfo->szFilename, '\0', MAX_PATH * sizeof (char));
                    memset(tempCharBuffer, '\0', MAX_PATH * sizeof (char));
                    wcstombs(tempCharBuffer, (wchar_t*)(resultBuffer + 24), MAX_PATH);

                    ptrCurrentToken = strtok(tempCharBuffer, "\\");
                    ptrPreviousToken = ptrCurrentToken;
                    while (ptrCurrentToken != NULL) {
                        ptrPreviousToken = ptrCurrentToken;
                        ptrCurrentToken = strtok(NULL, "\\");
                    }

                    strncpy(pTempFinfo->szFilename, ptrPreviousToken, strlen(ptrPreviousToken));
                    strcpy(pTempFinfo->szBuffer, "Deleted in Recycle Bin");
                    wxLogMessage("A file name retrieved from Recycle Bin.");
                    break;
                }
                //                else
                //                    wxLogMessage("A file name was not retrieved.");
            }
        }
//ovde ubaci u neku listu - neku jednostavniju globalnu strukturu
        //i onda popunjavaj u zavisnosti od searcha 
        long lIndex = m_ListControl->InsertItem(listCounter, pTempFinfo->szFilename, 0);
        m_ListControl->SetItemPtrData(lIndex, (wxUIntPtr) pTempFinfo);
        double llSizeInMB = pTempFinfo->n64SizeOnDisk;
        llSizeInMB /= (1024 * 1024);  //in MB
        char sSize[64] = {};
        sprintf(sSize, "%.2f MB", llSizeInMB);        
        m_ListControl->SetItem(lIndex, 1, wxString(sSize));
        m_ListControl->SetItem(lIndex, 2, pTempFinfo->szBuffer);
        listCounter++;

    }
    wxLogMessage("Populating file list completed.");

    m_ListControl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_ListControl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    //    m_ListControl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER); // bilo je wxLIST_AUTOSIZE
    m_ListControl->RefreshItem(0);
    m_ListControl->Show();
}

void *MyThreadLoadFilesFromDiskFAT::Entry() { //ovde pocinje izvrsavanje threada 

    // ovo nicemu ne sluzi - verovatno ce sve da se brise   
    // wxMutexLocker lock(*m_LockPtr); 
    int iRet;

    wxLogMessage("LoadFiles Thread started executing.");
    ThrowSelectDestinationDrivePulseEvent("Reading Volume.\nThis can take several minutes..."); // update progress bar message ;)

    wxLogMessage("Started reading volume %s", m_wxpSourceDrive->mb_str());
    iRet = m_pUsedNTFSDrive->ReadVolume(*m_wxpSourceDrive);
    if (iRet) {
        wxLogFatalError("ReadVolume failed. Please check access rights and restart the program");
    }
    wxLogMessage("Completed reading volume %s", m_wxpSourceDrive->mb_str());

    if (IsCancelProgresControl()) return (NULL); //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel
    ThrowSelectDestinationDrivePulseEvent("Creating deleted files list.\nThis can take several minutes...");

    wxLogMessage("Creating deleted files list...");
    m_finfohash->clear(); //ocisti hash za svaki slucaj :)    
    m_pUsedNTFSDrive->GetFileList(m_finfohash);
    wxLogMessage("Completed creating deleted files list.\nThis can take several minutes...");

    ThrowSelectDestinationDrivePulseEvent("Loading deleted files list.\nThis can take several minutes...");
    if (IsCancelProgresControl()) return (NULL); //promenljiva koja se postavlja iz progress-bar-a kad stisnes cancel

    wxLogMessage("Filling list control from Hash map");
    FillListControlFromHashMap();
    wxLogMessage("Completed filling list control from Hash map");

    ThrowSelectDestinationDriveStopEvent();

    return (0);
}

//
//MyThreadLoadFilesFromDiskFAT::~MyThreadLoadFilesFromDiskFAT() {
//
//}
//TODO popravi mesta gde pise sranje
//TODO razvoji thread akcije u razlicite fajlove