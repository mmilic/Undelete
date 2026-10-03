
#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/listctrl.h>
#include "NTFSDrive.h"
#include "MyThreadClass.h"
#include "MyWizzard.h"

#include "Page_EndPage.h"
#include "languages/LanguageEnums.h"

wxEndPage::wxEndPage(wxWizard *parent) : wxWizardPageSimple(parent) {

    this->SetOwnBackgroundColour(wxColour(210, 222, 255));
    this->SetOwnForegroundColour(wxColour(210, 222, 255));
    this->SetForegroundColour(wxColour(210, 222, 255));
    this->SetBackgroundColour(wxColour(210, 222, 255));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    //Ova se nece prikazati na poslednjoj strani - moras da promenis u funkciji koja se zove
    //kad se javi event
    int *iNumberOfCopiedFiles = ((MyWizard *) GetParent())->GetIptrNumberOfCopiedFiles();
    int *iNumberOfSelectedFiles = ((MyWizard *) GetParent())->GetMw_ptriNumberOfSelectedFiles();
    wxString *wxDestDirPtr = ((MyWizard *) GetParent())->GetMw_ptrDestinationDir();

    ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();
    //languagePack->getWXText(END_PAGE, SUCCESSFULLY_RETRIEVED)
    
    m_wxEndMessage.Printf(
            "Successfully retrieved %d files out of %d selected\n"
            "You can find them at %s.\n\n\n\n\n",
            *iNumberOfCopiedFiles, *iNumberOfSelectedFiles,
            wxDestDirPtr->mb_str()
            );
    
    
    
    //m_wxEndPageText = new wxStaticText(this, -1, m_wxEndMessage);
    m_wxEndPageText = new wxStaticText(this, -1, wxString::Format(languagePack->getWXText(END_PAGE, SUCCESSFULLY_RETRIEVED),*iNumberOfCopiedFiles));

    mainSizer->Add(m_wxEndPageText, 5, wxALL, 5);

    SetSizerAndFit(mainSizer);
}

void wxEndPage::OnPageShown(wxWizardEvent& event) {
    int *iNumberOfCopiedFiles = ((MyWizard *) GetParent())->GetIptrNumberOfCopiedFiles();
    int *iNumberOfSelectedFiles = ((MyWizard *) GetParent())->GetMw_ptriNumberOfSelectedFiles();
    wxString *wxDestDirPtr = ((MyWizard *) GetParent())->GetMw_ptrDestinationDir();

    ILanguage *languagePack = ((MyWizard *) GetParent())->GetLanguagePack();
    
    /*m_wxEndMessage.Printf(
            "Successfully retrieved %d files out of %d selected\n\n\n"
            "You can find them at\n%s.",
            *iNumberOfCopiedFiles, *iNumberOfSelectedFiles,
            wxDestDirPtr->mb_str()
            );*/

//    m_wxEndPageText->SetLabel(m_wxEndMessage);
    m_wxEndPageText = new wxStaticText(this, -1,
            wxString::Format(languagePack->getWXText(END_PAGE, SUCCESSFULLY_RETRIEVED),
                             *iNumberOfCopiedFiles,
                             *iNumberOfSelectedFiles,
                             wxDestDirPtr->mb_str())
            );
    m_wxEndPageText->SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL));

    wxSizer *mainSizer = this->GetSizer();
    mainSizer->RecalcSizes();

    wxWindow* NextButton = FindWindowById(wxID_FORWARD, GetParent());
    NextButton->SetLabel(languagePack->getWXText(GENERAL, FINISH_BUTTON_LABEL));
    
    wxWindow* wxCancelButton = FindWindowById(wxID_CANCEL, GetParent());
    wxCancelButton->Hide();
}

void wxEndPage::OnWizardPageChanging(wxWizardEvent& event) {
    if (event.GetDirection()) {
        wxString *wxDestDirPtr = ((MyWizard *) GetParent())->GetMw_ptrDestinationDir();

        ShellExecute(GetDesktopWindow(), L"explore", wxDestDirPtr->wc_str(), NULL, NULL, SW_SHOWNOACTIVATE);

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
// TODO: Koliko fajlova je obrisano
// TODO: Gde se nalaze obrisani fajlovi



BEGIN_EVENT_TABLE(wxEndPage, wxWizardPageSimple)
EVT_WIZARD_PAGE_SHOWN(wxID_ANY, wxEndPage::OnPageShown)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, wxEndPage::OnWizardPageChanging)
END_EVENT_TABLE()