#include "LanguageDialog.h"
#include <wx/wx.h>
#include <wx/intl.h>
#include "../Theme.h"

// the languages offered, by their LanguageFactory name
static const char *const s_aLanguages[] = {"English", "Deutsch"};

LanguageDialog::LanguageDialog()
: wxDialog(NULL, wxID_ANY, "GetMyFilesBack") {
    SetBackgroundColour(Theme::PanelBg);
    SetFont(Theme::Body());

    wxStaticText *pLabel = new wxStaticText(this, wxID_ANY, "Language / Sprache");
    pLabel->SetFont(Theme::Bold(Theme::Body()));
    m_pChoice = new wxChoice(this, wxID_ANY);
    for (size_t i = 0; i < WXSIZEOF(s_aLanguages); i++)
        m_pChoice->Append(s_aLanguages[i]);
    m_pChoice->SetSelection(wxLocale::GetSystemLanguage() >= wxLANGUAGE_GERMAN && wxLocale::GetSystemLanguage() <= wxLANGUAGE_GERMAN_SWISS ? 1 : 0);

    wxBoxSizer *pInner = new wxBoxSizer(wxVERTICAL);
    pInner->Add(pLabel, 0);
    pInner->Add(m_pChoice, 0, wxEXPAND | wxTOP, FromDIP(8));
    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(pInner, 0, wxEXPAND | wxALL, FromDIP(20));
    pSizer->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(20));
    pSizer->SetMinSize(FromDIP(wxSize(320, -1)));
    SetSizerAndFit(pSizer);
    Centre();
}

std::string LanguageDialog::GetLanguage() const {
    int i = m_pChoice->GetSelection();
    return (i >= 0 && i < (int) WXSIZEOF(s_aLanguages)) ? s_aLanguages[i] : "English";
}
