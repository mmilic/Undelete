/* 
 * File:   MyWizzard.h
 * Author: mmilic
 *
 * Created on October 20, 2012, 11:00 AM
 */

#ifndef MYWIZZARD_H
#define	MYWIZZARD_H



#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/font.h>
#include <wx/hashmap.h>
#include <wx/listctrl.h>
#include "NTFSDrive.h"
#include "languages/LanguageFactory.h"


// ----------------------------------------------------------------------------
// our wizard
// ----------------------------------------------------------------------------

#define COLOUR_LIGHT_BLUE 210, 222, 255
#define wxCOLOUR_BUTTON wxColour(110, 222, 255)

enum {
    DontUse = wxID_HIGHEST,
    BROWSE_BUTTON_ID,
    MY_WIZARD_ID,
    LIST_CTRL_DELETED_FILES,
    LIST_CTRL_SOURCE_DRIVES,
    SELECT_ALL_BUTTON,
    UNSELECT_ALL_BUTTON,
    FILTER_IMAGES_BUTTON,
    FILTER_VIDEO_BUTTON,
    FILTER_MUSIC_BUTTON,
    SEARCH_CONTROL
};

#define SET_COLOR_FOR_CURRENT_INSTANCE          this->SetOwnBackgroundColour(wxColour(210, 222, 255)); \
                                                this->SetOwnForegroundColour(wxColour(210, 222, 255)); \
                                                this->SetForegroundColour(wxColour(210, 222, 255));\
                                                this->SetBackgroundColour(wxColour(210, 222, 255));

class MyWizard : public wxWizard {
public:
    MyWizard(wxFrame *frame, bool useSizer = true);

    wxWizardPage *GetFirstPage() const {
        return m_page1;
    }

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnRunWizard(wxCommandEvent& event);
    void OnRunWizard();
    void OnRunWizardNoSizer(wxCommandEvent& event);
    void OnRunWizardModeless(wxCommandEvent& event);
    void OnWizardCancel(wxWizardEvent& event);
    void OnWizardFinished(wxWizardEvent& event);

    void SetPtrHLogFile(HANDLE *PtrHLogFile);
    HANDLE *GetPtrHLogFile();

    void SetPtrListMutex(wxMutex *PtrListMutex);
    wxMutex *GetPtrListMutex();

    void SetPtrFileInfoHash(FileInfoHash *PtrFileInfoHash);
    FileInfoHash *GetPtrFileInfoHash();

    void SetPtrCritsect(wxCriticalSection *PtrCritsect);
    wxCriticalSection *GetPtrCritsect();

    void SetPtrCNTFSDrive(CNTFSDrive *PtrCNTFSDrive);
    CNTFSDrive *GetPtrCNTFSDrive();

    void SetMw_ptrDestinationDir(wxString *PtrSelectedDir);
    wxString *GetMw_ptrDestinationDir();

    void SetPtrSelectedFiles(wxListCtrl* PtrSelectedFiles);
    wxListCtrl* GetPtrSelectedFiles() const;
    void SetMwPtr_strSourceDrive(wxString* mwPtr_strSourceDrive);
    wxString* GetMwPtr_strSourceDrive() const;
    void SetIptrNumberOfCopiedFiles(int iptrNumberOfCopiedFiles);
    int* GetIptrNumberOfCopiedFiles();
    void SetMw_iNumberOfSelectedFiles(int mw_iNumberOfSelectedFiles);
    int *GetMw_ptriNumberOfSelectedFiles();
    ILanguage *GetLanguagePack();
    int SetLanguagePack(std::string languageName);

private:
    wxWizardPageSimple *m_page1;

    //LOGIC ELEMENTS: mw_Ptr => mw: member wizzard, Ptr = pointer
    CNTFSDrive *mwPtr_CNTFSDrive;
    wxString *mwPtr_strSourceDrive;
    wxString *mwPtr_DestinationDir;
    FileInfoHash *mwPtr_FileInfoHash;
    wxCriticalSection *mwPtr_critsect; //Critical section mutex for control of hash map update :) :) sounds nice ;)
    wxListCtrl *mwPtr_SelectedFiles;
    int mw_iNumberOfCopiedFiles;
    int mw_iNumberOfSelectedFiles;
    wxMutex *mwPtr_ListMutex;
    HANDLE *mwPtr_hLogFile;
    ILanguage *languagePack;

    DECLARE_EVENT_TABLE()
};
#endif	/* MYWIZZARD_H */