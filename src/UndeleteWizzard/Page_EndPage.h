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

    wxString m_wxEndMessage;
    wxStaticText *m_wxEndPageText;
private:
    DECLARE_EVENT_TABLE()
};
