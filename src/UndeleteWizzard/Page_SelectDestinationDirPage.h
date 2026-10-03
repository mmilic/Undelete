/* 
 * File:   Page_SelectDestinationDrivePage.h
 * Author: MilicM
 *
 * Created on November 8, 2012, 9:05 AM
 */

#ifndef PAGE_SELECTDESTINATIONDRIVEPAGE_H
#define	PAGE_SELECTDESTINATIONDRIVEPAGE_H



#endif	/* PAGE_SELECTDESTINATIONDRIVEPAGE_H */

class wxSelectDestinationDirPage : public wxWizardPageSimple {
public:

    wxSelectDestinationDirPage(wxWizard *parent);
    
    //event processing
    void OnWizardPageChanging(wxWizardEvent& event);
    void ChooseDestination(wxCommandEvent & event);
    void OnShowNotification(wxCommandEvent & event);
    void OnPulseDialogEvent(wxCommandEvent & event);
    void OnStopDialogEvent(wxCommandEvent & event);  
    void OnPageShown(wxWizardEvent& event);
    void OnDialogColourChanging(wxSysColourChangedEvent& event);
    
    //misc.
    bool InitLogFile(wxString wxDestinationDir);

    wxTextCtrl *m_DestDirTextBox;
    wxButton *m_buttonBrowseDestinationDir;

    wxProgressDialog *mwPtr_Dialog;
    bool mw_bRunDialog;
    wxString mw_sDialogString;

private:
    DECLARE_EVENT_TABLE()
};