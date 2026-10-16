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
    void OnPulseDialogEvent(wxCommandEvent & event);
    void OnStopDialogEvent(wxCommandEvent & event);  
    void OnPageShown(wxWizardEvent& event);
    void OnDialogColourChanging(wxSysColourChangedEvent& event);
    
    //misc.
    bool InitLogFile(wxString wxDestinationDir);

    wxTextCtrl *m_DestDirTextBox;
    wxButton *m_buttonBrowseDestinationDir;

    wxProgressDialog *mwPtr_Dialog;
    volatile bool mw_bRunDialog; // cleared by the scan thread (stop event)

    void SetDialogString(const wxString &sText);
    wxString GetDialogString();

private:
    wxString mw_sDialogString; // only through Set/GetDialogString - shared with the scan thread
    wxCriticalSection m_csDialogString;

    // the destination this page filled in itself - replaced when another source drive is chosen,
    // kept as soon as the user types or browses another one
    wxString m_sSuggestedDestination;
    static wxString SuggestDestination(const wxString &sSourceDrive);

    DECLARE_EVENT_TABLE()
};