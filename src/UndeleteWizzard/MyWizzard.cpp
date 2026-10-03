#include "wx/imaglist.h"
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/dir.h>
#include <wx/progdlg.h>
#include <wx/tglbtn.h>

#include "main.h"
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/hashmap.h>
#include "NTFSDrive.h"
#include "MyThreadClass.h"

//icon image
#include "flag_rs.xpm"
#include "SidePictureZ_0.xpm"

//wizard pages header files
#include "Page_GreetingPage.h"
#include "Page_SelectSourceDrivePage.h"
#include "Page_SelectDestinationDirPage.h"
#include "Page_SelectFilesPage.h"
#include "Page_EndPage.h"

#include "MyWizzard.h"
#include "languages/LanguageDialog.h"
#include "languages/LanguageEnums.h"

#ifdef DEMO_BUILD
 #define VERSION "DEMO"
#else
 #define VERSION "v2.06"
#endif


MyWizard::MyWizard(wxFrame *frame, bool useSizer) {

    //    SetExtraStyle(wxWIZARD_EX_HELPBUTTON);
    std::wstring wcWindowName = L"GetMyFilesBack "; 
    wcWindowName += wxT(VERSION);
    
    //Create(frame, MY_WIZARD_ID, wxT("Undelete Wizard V 0.95"), // ovo verzioniranje treba da ide preko nekog makroa
    Create(frame, MY_WIZARD_ID, wcWindowName, // ovo verzioniranje treba da ide preko nekog makroa
            wxBitmap(SidePictureZ_0_xpm), wxDefaultPosition,
            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX);
    //    |wxCAPTION

    SetIcon(wxIcon(flag_rs));

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));

   //default language - not that it will ever be used
    std::string languageName("English");
    SetLanguagePack(languageName);
    
    // Language selection
    //LanguageDialog *languageDialog = 
    new LanguageDialog(wxT("Choose Language"), this);
    //languageDialog->Show(true);
    
    
    // Change button colours
    wxWindow *ptrButton = NULL;
    ptrButton = this->FindWindowById(wxID_FORWARD, this);
    ptrButton->SetOwnBackgroundColour(wxColour(110, 222, 0));
    ptrButton->SetLabel(languagePack->getWXText(GENERAL, NEXT_BUTTON_LABEL));

    ptrButton = this->FindWindowById(wxID_CANCEL, this);
    ptrButton->SetOwnBackgroundColour(wxCOLOUR_BUTTON);
    ptrButton->SetLabel(languagePack->getWXText(GENERAL, CANCEL_BUTTON_LABEL));

    ptrButton = this->FindWindowById(wxID_BACKWARD, this);
    ptrButton->SetOwnBackgroundColour(wxCOLOUR_BUTTON);
    ptrButton->SetLabel(languagePack->getWXText(GENERAL, BACK_BUTTON_LABEL));
    
   wxWindow* NextButton = FindWindowById(wxID_FORWARD, this);
    NextButton->SetLabel(wxT("&Retrieve >"));


    
    
    /*
     * Buttons can be:
     * wxID_OK = 5100,
    wxID_CANCEL,
    wxID_APPLY,
    wxID_YES,
    wxID_NO,
    wxID_STATIC,
    wxID_FORWARD,
    wxID_BACKWARD,
    wxID_DEFAULT,
    wxID_MORE,
    wxID_SETUP,
    wxID_RESET,
    wxID_CONTEXT_HELP,
    wxID_YESTOALL,
    wxID_NOTOALL,
    wxID_ABORT,
    wxID_RETRY,
    wxID_IGNORE,
    wxID_ADD,
    wxID_REMOVE,*/

    // Allow the bitmap to be expanded to fit the page height
    //    if (frame->GetMenuBar()->IsChecked(Wizard_ExpandBitmap))
    SetBitmapPlacement(wxWIZARD_VALIGN_CENTRE);

    /* Create all pointer values - witn new :) */


    // Enable scrolling adaptation
    //    if (frame->GetMenuBar()->IsChecked(Wizard_LargeWizard))
    SetLayoutAdaptationMode(wxDIALOG_ADAPTATION_MODE_ENABLED);

    // a wizard page may be either an object of predefined class
    m_page1 = new wxGreetingPage(this);

    //inicijalizuj sve "sistemske" varijable koje su potrebne
    mwPtr_CNTFSDrive = new CNTFSDrive;
    mwPtr_FileInfoHash = new FileInfoHash;
    mwPtr_strSourceDrive = new wxString;
    mwPtr_DestinationDir = new wxString;

    // ... or a derived class
    //    wxGreetingPage *page1 = new wxGreetingPage(this);
    wxSelectSourceDrivePage *page2 = new wxSelectSourceDrivePage(this);
    wxSelectDestinationDirPage *page3 = new wxSelectDestinationDirPage(this);
    wxSelectFilesPage *page4 = new wxSelectFilesPage(this);
    wxEndPage *page5 = new wxEndPage(this);

    wxSizer *MyWizardSizer = this->GetPageAreaSizer();
    MyWizardSizer->SetMinSize(1000, 1000);

    this->SetPageSize(wxSize(500, 400)); //jedini koji radi to sto treba - samo manuel kaze da ne treba tako

//    page4->SetSize(wxSize(1000, 1000));
    //    SetFinishLabel(wxT("Retrieve"));
    //    this->FinishLayout();
    //    this->

    // set the page order using a convenience function - could also use
    // SetNext/Prev directly as below
    wxWizardPageSimple::Chain(m_page1, page2);
    wxWizardPageSimple::Chain(page2, page3);
    wxWizardPageSimple::Chain(page3, page4);
    wxWizardPageSimple::Chain(page4, page5);

    // this page is not a wxWizardPageSimple, so we use SetNext/Prev to insert
    // it into the chain of pages
    //    wxCheckboxPage *page2 = new wxCheckboxPage(this, m_page1, page3);
    //    m_page1->SetNext(page2);
    //    page3->SetPrev(page2);
    //
    //    if (useSizer) {
    //        // allow the wizard to size itself around the pages
    //        GetPageAreaSizer()->Add(m_page1);
    //    }
}

void MyWizard::SetPtrHLogFile(HANDLE *PtrHLogFile) {

    this->mwPtr_hLogFile = PtrHLogFile;
}

HANDLE *MyWizard::GetPtrHLogFile() {

    return mwPtr_hLogFile;
}

void MyWizard::SetPtrListMutex(wxMutex *PtrListMutex) {

    this->mwPtr_ListMutex = PtrListMutex;
}

wxMutex *MyWizard::GetPtrListMutex() {

    return mwPtr_ListMutex;
}

void MyWizard::SetPtrFileInfoHash(FileInfoHash *PtrFileInfoHash) {

    this->mwPtr_FileInfoHash = PtrFileInfoHash;
}

FileInfoHash *MyWizard::GetPtrFileInfoHash() {

    return mwPtr_FileInfoHash;
}

void MyWizard::SetPtrCritsect(wxCriticalSection *PtrCritsect) {

    this->mwPtr_critsect = PtrCritsect;
}

wxCriticalSection *MyWizard::GetPtrCritsect() {

    return mwPtr_critsect;
}

void MyWizard::SetPtrCNTFSDrive(CNTFSDrive *PtrCNTFSDrive) {

    this->mwPtr_CNTFSDrive = PtrCNTFSDrive;
}

CNTFSDrive *MyWizard::GetPtrCNTFSDrive() {

    return mwPtr_CNTFSDrive;
}

void MyWizard::SetMw_ptrDestinationDir(wxString *PtrSelectedDir) {

    this->mwPtr_DestinationDir = PtrSelectedDir;
}

wxString *MyWizard::GetMw_ptrDestinationDir() {

    return mwPtr_DestinationDir;
}

void MyWizard::SetPtrSelectedFiles(wxListCtrl* PtrSelectedFiles) {

    this->mwPtr_SelectedFiles = PtrSelectedFiles;
}

wxListCtrl* MyWizard::GetPtrSelectedFiles() const {

    return mwPtr_SelectedFiles;
}

void MyWizard::SetMwPtr_strSourceDrive(wxString* mwPtr_strSourceDrive) {
    this->mwPtr_strSourceDrive = mwPtr_strSourceDrive;
}

wxString* MyWizard::GetMwPtr_strSourceDrive() const {
    return mwPtr_strSourceDrive;
}

void MyWizard::SetIptrNumberOfCopiedFiles(int iptrNumberOfCopiedFiles) {
    this->mw_iNumberOfCopiedFiles = iptrNumberOfCopiedFiles;
}

int* MyWizard::GetIptrNumberOfCopiedFiles() {
    return &mw_iNumberOfCopiedFiles;
}

void MyWizard::SetMw_iNumberOfSelectedFiles(int mw_iNumberOfSelectedFiles) {
    this->mw_iNumberOfSelectedFiles = mw_iNumberOfSelectedFiles;
}

int *MyWizard::GetMw_ptriNumberOfSelectedFiles() {
    return &mw_iNumberOfSelectedFiles;
}

ILanguage *MyWizard::GetLanguagePack() {
    return languagePack;
}

int MyWizard::SetLanguagePack(std::string languageName) {
    if (languageName.size() == 0)
        return 1;

    this->languagePack = GetLanguagePack();
    this->languagePack = LanguageFactory::Get()->CreateLanguage(languageName);
    
    /*if (MyWizard::languagePack != NULL) {
        free(MyWizard::languagePack);
    }*/
    return 1;
    

}

//void MyWizard::SetMwPtr_Dialog(wxProgressDialog* mwPtr_Dialog) {
//    this->mwPtr_Dialog = mwPtr_Dialog;
//}
//
//wxProgressDialog* MyWizard::GetMwPtr_Dialog() const {
//    return mwPtr_Dialog;
//}
//
//void MyWizard::SetMwPtr_DialogString(wxString* mwPtr_DialogString) {
//    this->mwPtr_DialogString = mwPtr_DialogString;
//}
//
//wxString* MyWizard::GetMwPtr_DialogString() const {
//    return mwPtr_DialogString;
//}

//void MyWizard::SetMwPtr_RunDialog(bool* mwPtr_ContinueDialog) {
//    this->mw_bRunDialog = mwPtr_ContinueDialog;
//}
//
//bool* MyWizard::GetMwPtr_bRunDialog() const {
//    return &mw_bRunDialog;
//}

void MyWizard::OnQuit(wxCommandEvent& WXUNUSED(event)) {
    // true is to force the frame to close

    Close(true);
}

void MyWizard::OnAbout(wxCommandEvent& WXUNUSED(event)) {

    wxMessageBox(wxT("File Undeleter\n")
    wxT("(c) 2012"),
            wxT("File Undeleter"), wxOK | wxICON_INFORMATION, this);
}

void MyWizard::OnRunWizard(wxCommandEvent& WXUNUSED(event)) {
    //    MyWizard wizard(this);
    //
    //    wizard.RunWizard(wizard.GetFirstPage());
}

void MyWizard::OnRunWizard() {
    //    MyWizard wizard(this);
    //
    //    wizard.RunWizard(wizard.GetFirstPage());
}

void MyWizard::OnRunWizardNoSizer(wxCommandEvent& WXUNUSED(event)) {
    //    MyWizard wizard(this, false);
    //
    //    wizard.RunWizard(wizard.GetFirstPage());
}

void MyWizard::OnRunWizardModeless(wxCommandEvent& WXUNUSED(event)) {
    //    MyWizard *wizard = new MyWizard(this);
    //    wizard->ShowPage(wizard->GetFirstPage());
    //    wizard->Show(true);
}

void MyWizard::OnWizardFinished(wxWizardEvent& WXUNUSED(event)) {

//    wxMessageBox(wxT("The wizard finished successfully."), wxT("Wizard notification"));
}

void MyWizard::OnWizardCancel(wxWizardEvent& WXUNUSED(event)) {
    //    wxMessageBox(wxT("The wizard was cancelled."), wxT("Wizard notification"));
}


// ----------------------------------------------------------------------------
// event tables and such
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyWizard, wxWizard)
EVT_MENU(Wizard_Quit, MyWizard::OnQuit)
EVT_MENU(Wizard_About, MyWizard::OnAbout)
EVT_MENU(Wizard_RunModal, MyWizard::OnRunWizard)
EVT_MENU(Wizard_RunNoSizer, MyWizard::OnRunWizardNoSizer)
EVT_MENU(Wizard_RunModeless, MyWizard::OnRunWizardModeless)

EVT_WIZARD_CANCEL(wxID_ANY, MyWizard::OnWizardCancel)
EVT_WIZARD_FINISHED(wxID_ANY, MyWizard::OnWizardFinished)
END_EVENT_TABLE()
