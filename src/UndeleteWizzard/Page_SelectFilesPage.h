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
#define PREVIEW_SIZE 160 // the preview box, in pixels

#include <wx/timer.h>

#include <wx/srchctrl.h>
#include "DeletedFilesList.h"

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
    void OnFilterChoice(wxCommandEvent & event);
    void OnPageShown(wxWizardEvent& event);

    void SelectFilesOfType(int iFileType, bool bSelect);
    void UpdateFoundFilesLabel();
    void OnPreviewTimer(wxTimerEvent & event);

    wxStaticBitmap *m_bmpPreview;
    wxStaticText *m_txtPreview;
    wxTimer m_previewTimer; // the preview is loaded a moment after the focus stops moving
    bool m_bCopying; // the copy thread uses the drive - no preview reads meanwhile

    CDeletedFilesList *m_lcFilesInSelectedDrive;
    //    int m_NumberOfSelectedFiles;
    volatile int m_NumberOfFileBeingCopied; // written by the copy thread
    volatile bool mw_bRunDialog; // cleared by the copy thread (stop event)

    void SetDialogString(const wxString &sText);
    wxString GetDialogString();
    wxStaticText *wxSelectFilesMessage;
    wxSearchCtrl *m_wxSearchCtrlSearchList;

    wxToggleButton *wxFilterImages;
    wxToggleButton *wxFilterVideo;
    wxToggleButton *wxFilterMusic;

    wxChoice *m_choiceRecoverability; // show: all / good only / hide overwritten
    wxChoice *m_choiceType; // show: all types / images / videos / ...
    wxStaticText *m_txtShowing; // "Showing 120 of 3 400 files"
private:
    wxString mw_sDialogString; // only through Set/GetDialogString - shared with the copy thread
    wxCriticalSection m_csDialogString;

    DECLARE_EVENT_TABLE()
};
