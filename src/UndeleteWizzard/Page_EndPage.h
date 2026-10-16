/* 
 * File:   Page_EndPage.h
 * Author: MilicM
 *
 * Created on November 8, 2012, 9:28 AM
 */

#ifndef PAGE_ENDPAGE_H
#define	PAGE_ENDPAGE_H



#endif	/* PAGE_ENDPAGE_H */

class wxEndPage : public wxWizardPageSimple {
public:    
    wxEndPage(wxWizard *parent);
    void OnWizardPageChanging(wxWizardEvent& event);
    void OnPageShown(wxWizardEvent& event);
    void OnOpenFolderClick(wxCommandEvent & event);
    void OpenDestinationFolder();

    wxStaticText *m_wxEndPageText; // "recovered N of M files, you can find them in ..."
    wxStaticText *m_wxOverwrittenText; // shown when overwritten files were recovered after the warning
    wxStaticText *m_wxFailuresLabel;
    wxTextCtrl *m_wxFailuresList; // "name - reason", one per line
private:
    DECLARE_EVENT_TABLE()
};
