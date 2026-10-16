/*
 * File:   DeletedFilesList.h
 *
 * The list of deleted files on the "select files" page. A virtual list control: it only keeps
 * pointers to the files and asks for the text of the rows that are on screen, so it stays fast
 * with hundreds of thousands of files. Filtering (search) and sorting work on the pointer vector.
 */

#ifndef DELETEDFILESLIST_H
#define	DELETEDFILESLIST_H

#include <windows.h>
#include <wx/wx.h>
#include <wx/hashmap.h>
#include <wx/listctrl.h>
#include <vector>
#include "NTFSDrive.h"

// "512 B", "12.3 KB", "4.5 MB", "1.20 GB"
wxString FormatFileSize(LONGLONG llSize);

// general file types, by file extension - for the "Type" filter, the type buttons and the preview
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
const char *FileTypeFilterLabel(int iType); // "All types", "Images", ...

// the "Show" filter on the Recoverable column
enum {
    RECOVERABILITY_FILTER_ALL = 0,
    RECOVERABILITY_FILTER_GOOD, // only files whose clusters are all free
    RECOVERABILITY_FILTER_NOT_OVERWRITTEN, // everything except overwritten files
    RECOVERABILITY_FILTER_COUNT
};
const char *RecoverabilityFilterLabel(int iFilter);

enum {
    DELETED_FILES_COLUMN_NAME = 0,
    DELETED_FILES_COLUMN_SIZE,
    DELETED_FILES_COLUMN_RECOVERABILITY,
    DELETED_FILES_COLUMN_STATUS,
    DELETED_FILES_COLUMN_COUNT
};

class CDeletedFilesList : public wxListCtrl {
public:
    CDeletedFilesList(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size);

    // show the files of a finished scan (main thread only)
    void SetFiles(FileInfoHash *finfohash);
    void Clear();

    // show only the files whose name contains sFilter (case insensitive), "" = all
    void SetNameFilter(const wxString &sFilter);
    void SetRecoverabilityFilter(int iFilter); // RECOVERABILITY_FILTER_*
    void SetTypeFilter(int iType); // FILE_TYPE_*, FILE_TYPE_ANY = all

    // sort by a column - clicking the same column again reverses the order
    void SortByColumn(int iColumn);

    long GetFileCount() const { return (long) m_vVisible.size(); } // shown, after the filters
    long GetTotalFileCount() const { return (long) m_vAll.size(); }
    t_FileInfo *GetFile(long lItem) const;
    void GetSelectedFiles(std::vector<t_FileInfo *> &vSelected) const;
    void SelectAll(bool bSelect);
    void SelectItem(long lItem, bool bSelect);

protected:
    virtual wxString OnGetItemText(long lItem, long lColumn) const;
    virtual wxListItemAttr *OnGetItemAttr(long lItem) const;
    void OnListKeyDown(wxListEvent &event);

private:
    void Rebuild(); // re-apply filters and sort, keep the selection
    bool PassesFilters(const t_FileInfo *pFile) const;

    std::vector<t_FileInfo *> m_vAll; // every file of the scan
    std::vector<t_FileInfo *> m_vVisible; // what the list shows, filtered and sorted
    wxString m_sFilterUpper;
    int m_iRecoverabilityFilter;
    int m_iTypeFilter;
    int m_iSortColumn;
    bool m_bSortAscending;

    mutable wxListItemAttr m_attrPartial;
    mutable wxListItemAttr m_attrOverwritten;
};

#endif	/* DELETEDFILESLIST_H */
