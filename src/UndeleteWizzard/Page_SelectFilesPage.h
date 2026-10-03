/* 
 * File:   Page_SelectFilesPage.h
 * Author: MilicM
 *
 * Created on November 8, 2012, 9:19 AM
 */

#ifndef PAGE_SELECTFILESPAGE_H
#define	PAGE_SELECTFILESPAGE_H



#endif	/* PAGE_SELECTFILESPAGE_H */

#define FILE_NAME_COLUMN 0

#include <wx/srchctrl.h>

class wxSelectFilesPage : public wxWizardPageSimple {
public:

    wxSelectFilesPage(wxWizard *parent);
    
    //event processing
    void OnWizardPageChanging(wxWizardEvent& event);
    void OnStopDialogEvent(wxCommandEvent & event);
    void OnPulseDialogEvent(wxCommandEvent & event);
    void OnSelectAllClick(wxCommandEvent & event);
    void OnUnselectAllClick(wxCommandEvent & event);
    void OnSearchAction(wxCommandEvent & event);
    void OnSearchCancel(wxCommandEvent & event);
    void OnFilterImagesClick(wxCommandEvent & event);
    void OnFilterVideosClick(wxCommandEvent & event);
    void OnFilterMusicClick(wxCommandEvent & event);
    void OnFilesListClick(wxCommandEvent & event);
    void OnListColumnClick(wxCommandEvent & event);
    void OnPageShown(wxWizardEvent& event);

    wxListCtrl *m_lcFilesInSelectedDrive;
    //    int m_NumberOfSelectedFiles;
    int m_NumberOfFileBeingCopied;
    bool mw_bRunDialog;
    wxString mw_sDialogString;
    wxStaticText *wxSelectFilesMessage;
    wxSearchCtrl *m_wxSearchCtrlSearchList;

    wxToggleButton *wxFilterImages;
    wxToggleButton *wxFilterVideo;
    wxToggleButton *wxFilterMusic;
private:
    DECLARE_EVENT_TABLE()
};
