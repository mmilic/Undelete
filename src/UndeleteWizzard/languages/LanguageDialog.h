/* 
 * File:   languageDialog.h
 * Author: ezormil
 *
 * Created on 7. Juli 2014, 17:35
 */

#ifndef LANGUAGEDIALOG_H
#define	LANGUAGEDIALOG_H

//#include "../MyWizzard.h"




#include <wx/wx.h>
#include "../MyWizzard.h"

class LanguageDialog : public wxDialog
{
public:
  LanguageDialog(const wxString& title,  MyWizard *wizard);
  void HandleExit(wxCommandEvent &);
  void setLanguage(wxCommandEvent &);
  void setEnglishLanguage(); //TODO umesto ovih funkcija, trebalo bi napraviti nesto dinamicki
  void setGermanLanguage();
  void setSerbianLanguage();
  void OnExit(wxCloseEvent&);
  
  MyWizard *localWizard;
  wxButton *closeButton1;
  wxButton *closeButton;
  wxButton *OKButton;
  
  wxComboBox *LanguageComboBox;
private:
    DECLARE_EVENT_TABLE()
};

#endif	/* LANGUAGEDIALOG_H */