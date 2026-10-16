
#include <wx/wx.h>
#include <wx/wizard.h>
#include <shellapi.h> // ShellExecute - not in a WIN32_LEAN_AND_MEAN windows.h
#include <wx/listctrl.h>
#include "RecoveryDrive.h"
#include "MyThreadClass.h"
#include "MyWizzard.h"

#include "Page_EndPage.h"
#include "languages/LanguageEnums.h"

// the summary of the recovery: how many files, where they are, which ones failed and why

wxEndPage::wxEndPage(wxWizard *parent) : wxWizardPageSimple(parent) {

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    // filled in OnPageShown - the numbers are only known after the recovery
    m_wxEndPageText = new wxStaticText(this, -1, wxEmptyString);
    m_wxEndPageText->SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    m_wxOverwrittenText = new wxStaticText(this, -1, wxEmptyString);
    m_wxOverwrittenText->SetForegroundColour(wxColour(170, 90, 0)); // dark orange, like partly overwritten rows

    m_wxFailuresLabel = new wxStaticText(this, -1, wxT("These files could not be recovered:"));
    m_wxFailuresList = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(-1, 120),
            wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);

    wxButton *wxOpenFolderButton = new wxButton(this, END_PAGE_OPEN_FOLDER_BUTTON, wxT("Open folder"));
    wxOpenFolderButton->SetOwnBackgroundColour(wxColour(63, 119, 246)); //darker blue
    wxOpenFolderButton->SetOwnForegroundColour(wxColour(255, 255, 255)); // white

    mainSizer->Add(m_wxEndPageText, 0, wxALL | wxEXPAND, 5);
    mainSizer->Add(wxOpenFolderButton, 0, wxLEFT | wxBOTTOM, 5);
    mainSizer->Add(m_wxOverwrittenText, 0, wxALL | wxEXPAND, 5);
    mainSizer->Add(m_wxFailuresLabel, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(m_wxFailuresList, 1, wxALL | wxEXPAND, 5);

    Connect(END_PAGE_OPEN_FOLDER_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(wxEndPage::OnOpenFolderClick));

    SetSizerAndFit(mainSizer);
}

void wxEndPage::OnPageShown(wxWizardEvent& event) {
    MyWizard *pWizard = (MyWizard *) GetParent();
    int iNumberOfCopiedFiles = *pWizard->GetIptrNumberOfCopiedFiles();
    int iNumberOfSelectedFiles = *pWizard->GetMw_ptriNumberOfSelectedFiles();
    int iNumberOfOverwritten = *pWizard->GetPtrNumberOfOverwrittenSelected();
    std::vector<wxString> *pvFailures = pWizard->GetPtrRecoveryFailures();
    wxString *wxDestDirPtr = pWizard->GetMw_ptrDestinationDir();

    ILanguage *languagePack = pWizard->GetLanguagePack();

    m_wxEndPageText->SetLabel(wxString::Format(languagePack->getWXText(END_PAGE, SUCCESSFULLY_RETRIEVED),
            iNumberOfCopiedFiles, iNumberOfSelectedFiles, *wxDestDirPtr));

    if (iNumberOfOverwritten)
        m_wxOverwrittenText->SetLabel(wxString::Format(
            "%d of the selected files were marked as overwritten. They were saved, but other files have since "
            "been stored where their data was, so they will most likely be unreadable.", iNumberOfOverwritten));
    m_wxOverwrittenText->Show(iNumberOfOverwritten != 0);
    m_wxOverwrittenText->Wrap(GetClientSize().GetWidth() > 100 ? GetClientSize().GetWidth() - 20 : 500);

    wxString sFailures;
    for (size_t i = 0; i < pvFailures->size(); i++)
        sFailures += (*pvFailures)[i] + "\n";
    m_wxFailuresList->SetValue(sFailures);
    m_wxFailuresLabel->Show(!pvFailures->empty());
    m_wxFailuresList->Show(!pvFailures->empty());

    Layout();

    wxWindow* NextButton = FindWindowById(wxID_FORWARD, GetParent());
    NextButton->SetLabel(languagePack->getWXText(GENERAL, FINISH_BUTTON_LABEL));

    wxWindow* wxCancelButton = FindWindowById(wxID_CANCEL, GetParent());
    wxCancelButton->Hide();
}

void wxEndPage::OpenDestinationFolder() {
    wxString *wxDestDirPtr = ((MyWizard *) GetParent())->GetMw_ptrDestinationDir();
    ShellExecute(GetDesktopWindow(), L"explore", wxDestDirPtr->wc_str(), NULL, NULL, SW_SHOWNOACTIVATE);
}

void wxEndPage::OnOpenFolderClick(wxCommandEvent & event) {
    OpenDestinationFolder();
}

void wxEndPage::OnWizardPageChanging(wxWizardEvent& event) {
    if (event.GetDirection()) {
        OpenDestinationFolder();

        //moze i ovako - cisto da se ne zaboravi
        //    wxString explorrerCommand;
        //    explorrerCommand.Printf("@start explorer %s", wxDestDirPtr->mb_str());
        //    system(explorrerCommand.mb_str());
        //
    } else {
        wxWindow* wxCancelButton = FindWindowById(wxID_CANCEL, GetParent());
        wxCancelButton->Show();
    }
}

BEGIN_EVENT_TABLE(wxEndPage, wxWizardPageSimple)
EVT_WIZARD_PAGE_SHOWN(wxID_ANY, wxEndPage::OnPageShown)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, wxEndPage::OnWizardPageChanging)
END_EVENT_TABLE()
