

#include <string>
#include <wx/tglbtn.h>

#include "MyWizzard.h"
#include "Page_GreetingPage.h"
#include "languages/LanguageEnums.h"


wxGreetingPage::wxGreetingPage(wxWizard *parent) : wxWizardPageSimple(parent) {

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));

    ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();

    wxStaticText *wstaticGreetingMessage_1 = new wxStaticText(this, -1,
            languagePack->getWXText(GREETINGS_PAGE, WELCOME));
    
    wstaticGreetingMessage_1->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));

    wxStaticText *wstaticGreetingMessage_2 = new wxStaticText(this, -1,
            languagePack->getWXText(GREETINGS_PAGE, WINDOWS_UNDELETE_WIZARD));
    
    wstaticGreetingMessage_2->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));

    wxStaticText *wstaticGreetingMessage_3 = new wxStaticText(this, -1,
            languagePack->getWXText(GREETINGS_PAGE, THIS_WIZARD_WILL_GUIDE_YOU));
    
    wstaticGreetingMessage_3->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));

    wxStaticText *wstaticGreetingMessage_4 = new wxStaticText(this, -1,
            languagePack->getWXText(GREETINGS_PAGE, THE_UNDELETE_PROCESS_CONSISTS));
    
    wstaticGreetingMessage_4->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));

    wxStaticText *wstaticGreetingMessage_5 = new wxStaticText(this, -1,
            languagePack->getWXText(GREETINGS_PAGE, CLICK_NEXT_TO_CONTINUE));
    
    wstaticGreetingMessage_5->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(wstaticGreetingMessage_1, 0, wxALL, 0);
    mainSizer->Add(wstaticGreetingMessage_2, 0, wxALL, 0);
    mainSizer->Add(wstaticGreetingMessage_3, 0, wxALL, 0);
    mainSizer->Add(wstaticGreetingMessage_4, 0, wxALL, 0);
    mainSizer->Add(wstaticGreetingMessage_5, 0, wxALL, 0);

    SetSizerAndFit(mainSizer);
};

void wxGreetingPage::OnPageShown(wxWizardEvent& event) {
    wxWindow* NextButton = FindWindowById(wxID_FORWARD, GetParent());
    ILanguage *languagePackLocal = ((MyWizard *) GetParent())->GetLanguagePack();
    
    NextButton->SetLabel(languagePackLocal->getWXText(GENERAL, NEXT_BUTTON_LABEL));
}

BEGIN_EVENT_TABLE(wxGreetingPage, wxWizardPageSimple)
   EVT_WIZARD_PAGE_SHOWN(wxID_ANY, wxGreetingPage::OnPageShown)
END_EVENT_TABLE()