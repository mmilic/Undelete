/*
 * File:   languageDialog.h
 * Author: ezormil
 *
 * Created on 7. Juli 2014, 17:35
 *
 * The language of the UI, asked once at the start. German Windows starts with Deutsch preselected.
 */

#ifndef LANGUAGEDIALOG_H
#define	LANGUAGEDIALOG_H

#include <wx/wx.h>
#include <string>

class LanguageDialog : public wxDialog {
public:
    LanguageDialog();

    // the LanguageFactory name of the chosen language: "English", "Deutsch"
    std::string GetLanguage() const;

private:
    wxChoice *m_pChoice;
};

#endif	/* LANGUAGEDIALOG_H */
