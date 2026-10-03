#include "LanguageDialog.h"
#include <wx/wx.h>
#include "../CustomButton.h"

#define CYRILLIC 1

//#include "resources/flag_rs.xpm"

//#include "../MyWizzard.h"
///run for i=1,GetAchievementNumCriteria(7298)do local a,b,c,d,e=GetAchievementCriteriaInfo(7298,i) print(format("%s %d/%d",a,d,e)) end

LanguageDialog::LanguageDialog(const wxString & title, MyWizard *wizard)
: wxDialog(NULL, -1, title, wxDefaultPosition, wxSize(320, 63)) {

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));
    localWizard = wizard;

    wxBoxSizer *hFlagBox = new wxBoxSizer(wxHORIZONTAL);
    
    wxString wxsChoices[] = {
              wxT("English")
            , wxT("Deutsch")
            //, wxT("Srpski")
    };
    LanguageComboBox = new wxComboBox(this, -1, wxT("English"),
            wxDefaultPosition, wxDefaultSize, 2, wxsChoices, wxCB_READONLY);

    OKButton = new wxButton(this, 1, "OK");
   
    hFlagBox->Add(LanguageComboBox, 3, wxEXPAND | wxALL, 5);
    hFlagBox->Add(OKButton, 1, wxEXPAND | wxALL, 5);
   
    LanguageComboBox->Bind(wxEVT_COMBOBOX, &LanguageDialog::setLanguage, this);

    SetSizer(hFlagBox);

    Centre();

    ShowModal();

    Destroy();
}

void LanguageDialog::HandleExit(wxCommandEvent &) {
    exit(0);
}

void LanguageDialog::setLanguage(wxCommandEvent &evnt) {
    wxString LanguageChosen = LanguageComboBox->GetValue();

    if (wxT("English") == LanguageChosen)
        //setEnglishLanguage(); //default
         EndModal(1);
    else if (wxT("Deutsch") == LanguageChosen)
        setGermanLanguage();
    else if (wxT("Srpski") == LanguageChosen)
        setSerbianLanguage();
}

void LanguageDialog::setEnglishLanguage() {
    localWizard->SetLanguagePack("English");
    EndModal(1);
    return;
}

void LanguageDialog::setSerbianLanguage() {

#if CYRILLIC>0   
    localWizard->SetLanguagePack("Srpski_Cyr");
#else     
    localWizard->SetLanguagePack("Srpski");
#endif    
    EndModal(1);
    return;
}

void LanguageDialog::setGermanLanguage() {
    localWizard->SetLanguagePack("Deutsch");
    EndModal(1);
    return;
}

void LanguageDialog::OnExit(wxCloseEvent& event) {
    exit(0);
}

wxBEGIN_EVENT_TABLE(LanguageDialog, wxDialog)
EVT_CLOSE(LanguageDialog::OnExit)
EVT_BUTTON(1, LanguageDialog::setLanguage)
wxEND_EVENT_TABLE()

#if 0
class wxRadioboxPage : public wxWizardPageSimple {
public: // directions in which we allow the user to proceed from this page 

    enum {
        Forward, Backward, Both, Neither
    };

    wxRadioboxPage(wxWizard *parent) : wxWizardPageSimple(parent) { // should correspond to the enum above

    }
    static wxString choices[] = {wxT("forward"), wxT("backward"), wxT("both"), wxT("neither")};
    m_radio = new wxRadioBox(this, wxID_ANY, wxT("Allow to proceed:"), wxDefaultPosition, wxDefaultSize, WXSIZEOF(choices), choices, 1, wxRA_SPECIFY_COLS);
    m_radio->SetSelection(Both);
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_radio, 0, // No stretching 
            wxALL,
} 5
// Border
);
SetSizer(mainSizer);
mainSizer->Fit(this);
}
#endif
