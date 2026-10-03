/* 
 * File:   SelectSourceDrivePage.h
 * Author: MilicM
 *
 * Created on November 8, 2012, 8:46 AM
 */

#ifndef PAGE_SELECTSOURCEDRIVEPAGE_H
#define	PAGE_SELECTSOURCEDRIVEPAGE_H



#endif	/* PAGE_SELECTSOURCEDRIVEPAGE_H */

class wxSelectSourceDrivePage : public wxWizardPageSimple {
public:
    wxSelectSourceDrivePage(wxWizard *parent);
    bool InvalidDiskAccessRights(wxString *ptrSelectedDesk);
    
    //event processing
    void OnWizardPageChanging(wxWizardEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnPageShown(wxWizardEvent& event);

    wxListCtrl *m_DriveListCtrlPtr;
    wxImageList *m_imageListNormal;
    wxImageList *m_imageListSmall;

private:
    DECLARE_EVENT_TABLE()
};