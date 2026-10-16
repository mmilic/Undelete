/*
 * File:   DeletedFilesList.h
 *
 * The list of deleted files. A virtual list control: it only keeps pointers to the files and asks for the
 * text of the rows that are on screen, so it stays fast with hundreds of thousands of files. Filtering
 * (search, recoverability, type) and sorting work on the pointer vector.
 *
 * The files to recover are the ticked ones (the first column), not the highlighted rows - a highlight is
 * lost with one careless click, a tick isn't. Only ticked files that are shown count: a filter that hides
 * a file also takes it out of the recovery.
 */

#ifndef DELETEDFILESLIST_H
#define	DELETEDFILESLIST_H

#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include <wx/listctrl.h>
#include <unordered_set>
#include <vector>
#include "RecoveryDrive.h"

// "512 B", "12.3 KB", "4.5 MB", "1.20 GB"
wxString FormatFileSize(LONGLONG llSize);

// general file types, by file extension - for the "Type" filter, the icons and the preview
enum {
    FILE_TYPE_ANY = 0, // as a filter: every type
    FILE_TYPE_IMAGE,
    FILE_TYPE_VIDEO,
    FILE_TYPE_MUSIC,
    FILE_TYPE_DOCUMENT,
    FILE_TYPE_TEXT,
    FILE_TYPE_ARCHIVE,
    FILE_TYPE_OTHER,
    FILE_TYPE_COUNT
};

int FileTypeOf(const wchar_t *szFilename);
wxString FileTypeFilterLabel(int iType); // "All types", "Images", ... - translated

// the "Recoverable" filter
enum {
    RECOVERABILITY_FILTER_ALL = 0,
    RECOVERABILITY_FILTER_GOOD, // only files whose clusters are all free
    RECOVERABILITY_FILTER_NOT_OVERWRITTEN, // everything except overwritten files
    RECOVERABILITY_FILTER_COUNT
};

wxString RecoverabilityFilterLabel(int iFilter); // translated

// "2024-05-12 14:33" in local time, "" for no time
wxString FormatFileTime(LONGLONG llFileTime);

// "Good", "Partly · 62 % intact", "Overwritten" - translated; its colours
wxString RecoverabilityText(const t_FileInfo &file);
wxColour RecoverabilityColour(int iRecoverability);
wxString ContentText(int iContent); // "Looks valid", "Damaged"... - translated
wxColour ContentColour(int iContent);

enum {
    DELETED_FILES_COLUMN_CHECK = 0, // the tick box - no text
    DELETED_FILES_COLUMN_NAME,
    DELETED_FILES_COLUMN_FOLDER,
    DELETED_FILES_COLUMN_SIZE,
    DELETED_FILES_COLUMN_MODIFIED,
    DELETED_FILES_COLUMN_RECOVERABILITY,
    DELETED_FILES_COLUMN_CONTENTS,
    DELETED_FILES_COLUMN_COUNT
};

// sent (to the list itself, so it reaches its parents) when files were ticked or unticked
wxDECLARE_EVENT(EVT_DELETED_FILES_CHECKED, wxCommandEvent);

class CDeletedFilesList : public wxListCtrl {
public:
    CDeletedFilesList(wxWindow *parent, wxWindowID id);

    // files a running scan has just found - shown at the end of the list, in the order found
    void AddFiles(const std::vector<t_FileInfo *> &vFiles);
    // the files of a finished scan, sorted. Ticks stay on the files that are still there (FinishScan removes
    // the Recycle Bin's $I files - their pointers are only compared, never followed).
    void SetFiles(FileInfoHash *finfohash);
    void Clear();

    // show only the files whose folder or name contains sFilter (case insensitive), "" = all
    void SetNameFilter(const wxString &sFilter);
    void SetRecoverabilityFilter(int iFilter); // RECOVERABILITY_FILTER_*
    void SetTypeFilter(int iType); // FILE_TYPE_*, FILE_TYPE_ANY = all
    void SetValidOnly(bool bValidOnly); // only files whose contents were checked and look valid
    void ClearFilters();
    void Refilter() { Rebuild(); } // after the files changed (e.g. their contents were checked)

    // how many files of each type pass the other filters - aCounts[FILE_TYPE_ANY] is all of them
    void CountByType(int aCounts[FILE_TYPE_COUNT]) const;

    // sort by a column - clicking the same column again reverses the order
    void SortByColumn(int iColumn);

    long GetFileCount() const { return (long) m_vVisible.size(); } // shown, after the filters
    long GetTotalFileCount() const { return (long) m_vAll.size(); }
    t_FileInfo *GetFile(long lItem) const;
    t_FileInfo *GetFocusedFile() const;
    void RefreshFile(const t_FileInfo *pFile); // its row, if it is shown

    // the ticked files that are shown - what gets recovered
    void GetCheckedFiles(std::vector<t_FileInfo *> &vChecked) const;
    int GetCheckedCount() const { return m_iCheckedShown; }
    LONGLONG GetCheckedBytes() const { return m_llCheckedShownBytes; }
    void CheckAllShown(bool bCheck);
    void CheckFiles(const std::vector<t_FileInfo *> &vFiles, bool bCheck);
    bool AreAllShownChecked() const { return !m_vVisible.empty() && m_iCheckedShown == (int) m_vVisible.size(); }

protected:
    wxString OnGetItemText(long lItem, long lColumn) const override;
    int OnGetItemImage(long lItem) const override;
    int OnGetItemColumnImage(long lItem, long lColumn) const override;
    wxItemAttr *OnGetItemColumnAttr(long lItem, long lColumn) const override;

private:
    std::vector<t_FileInfo *> m_vAll; // every file of the scan
    std::vector<unsigned char> m_vAllTypes; // FileTypeOf() of each of m_vAll - the filters ask it a lot
    std::vector<t_FileInfo *> m_vVisible; // what the list shows, filtered and sorted
    std::unordered_set<const t_FileInfo *> m_setChecked;
    int m_iCheckedShown;
    LONGLONG m_llCheckedShownBytes;
    wxString m_sFilterUpper;
    int m_iRecoverabilityFilter;
    int m_iTypeFilter;
    bool m_bValidOnly;
    int m_iSortColumn;
    bool m_bSortAscending;

    // a text colour per kind of cell, on a white or a "ticked" background
    enum {
        CELL_TEXT, CELL_SECONDARY, CELL_GOOD, CELL_PARTIAL, CELL_OVERWRITTEN, CELL_DAMAGED, CELL_UNCHECKED, CELL_KINDS
    };
    mutable wxItemAttr m_aAttr[CELL_KINDS][2];

    void Rebuild(); // re-apply filters and sort, keep the focus
    void FilterChanged();
    bool PassesFilters(const t_FileInfo *pFile, int iType, bool bTypeFilter = true) const;
    void CountChecked();
    void SetChecked(t_FileInfo *pFile, bool bCheck);
    void NotifyChecked();
    void UpdateHeaderCheckBox();

    void OnLeftDown(wxMouseEvent &event);
    void OnKeyDown(wxListEvent &event);
    void OnColumnClick(wxListEvent &event);
    void OnSize(wxSizeEvent &event);
    void FitColumns(); // the columns fill the list's width - no horizontal scrolling
};

#endif	/* DELETEDFILESLIST_H */
