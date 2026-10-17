#include <windows.h>
#include <commctrl.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <algorithm>

#include "DeletedFilesList.h"
#include "FileContents.h"
#include "Theme.h"
#include "languages/Translate.h"

wxDEFINE_EVENT(EVT_DELETED_FILES_CHECKED, wxCommandEvent);

// the list's images, all on one canvas (the list's rows are as tall as its images)
enum {
    IMAGE_UNCHECKED = 0,
    IMAGE_CHECKED,
    IMAGE_FILE_TYPE, // + FILE_TYPE_IMAGE - 1 ... FILE_TYPE_OTHER - 1
    IMAGE_DOT_GOOD = IMAGE_FILE_TYPE + FILE_TYPE_OTHER,
    IMAGE_DOT_PARTIAL,
    IMAGE_DOT_OVERWRITTEN,
    IMAGE_DOT_UNKNOWN
};

// order of the "Recoverable" column: most promising first

static int RecoverabilityRank(const t_FileInfo *pFile) {
    switch (pFile->iRecoverability) {
        case RECOVERABILITY_GOOD: return 0;
        case RECOVERABILITY_PARTIAL: return 1;
        case RECOVERABILITY_UNKNOWN: return 2;
        default: return 3; // overwritten
    }
}

// order of the "Contents" column: valid first, not checked last

static int ContentRank(const t_FileInfo *pFile) {
    switch (pFile->iContent) {
        case CONTENT_OK: return 0;
        case CONTENT_NO_SIGNATURE: return 1;
        case CONTENT_UNREADABLE: return 2;
        case CONTENT_DAMAGED: return 3;
        case CONTENT_ZEROS: return 4;
        case CONTENT_START_OVERWRITTEN: return 5;
        default: return 6; // not checked
    }
}

static int CompareLongLong(LONGLONG a, LONGLONG b) {
    return (a < b) ? -1 : (a > b ? 1 : 0);
}

struct DeletedFilesOrder {
    int iColumn;
    bool bAscending;

    bool operator()(const t_FileInfo *a, const t_FileInfo *b) const {
        int r = 0;
        switch (iColumn) {
            case DELETED_FILES_COLUMN_FOLDER:
                if (a->sFolder.c_str() != b->sFolder.c_str()) // files of one folder share the string
                    r = _wcsicmp(a->sFolder.c_str(), b->sFolder.c_str());
                break;
            case DELETED_FILES_COLUMN_SIZE:
                r = CompareLongLong(a->n64RealSize, b->n64RealSize);
                break;
            case DELETED_FILES_COLUMN_MODIFIED:
                r = CompareLongLong(a->n64Modify, b->n64Modify);
                break;
            case DELETED_FILES_COLUMN_RECOVERABILITY:
                r = RecoverabilityRank(a) - RecoverabilityRank(b);
                if (!r) // more intact first
                    r = b->iPercentIntact - a->iPercentIntact;
                break;
            case DELETED_FILES_COLUMN_CONTENTS:
                r = ContentRank(a) - ContentRank(b);
                if (!r)
                    r = RecoverabilityRank(a) - RecoverabilityRank(b);
                break;
        }
        if (!r) // ties (and the name column) - by name, so the order is stable and predictable
            r = _wcsicmp(a->sFilename.c_str(), b->sFilename.c_str());
        if (!r)
            r = a->iRecordId - b->iRecordId;
        return bAscending ? (r < 0) : (r > 0);
    }
};

wxString FormatFileSize(LONGLONG llSize) {
    if (llSize < 1024)
        return wxString::Format("%d B", (int) llSize);
    if (llSize < 1024 * 1024)
        return wxString::Format("%.1f KB", llSize / 1024.0);
    if (llSize < 1024LL * 1024 * 1024)
        return wxString::Format("%.1f MB", llSize / (1024.0 * 1024));
    return wxString::Format("%.2f GB", llSize / (1024.0 * 1024 * 1024));
}

struct FileTypeExtension {
    const wchar_t *szExtension;
    int iType;
};

static const FileTypeExtension s_aFileTypes[] = {
    {L"jpg", FILE_TYPE_IMAGE}, {L"jpeg", FILE_TYPE_IMAGE}, {L"jpe", FILE_TYPE_IMAGE}, {L"gif", FILE_TYPE_IMAGE},
    {L"png", FILE_TYPE_IMAGE}, {L"bmp", FILE_TYPE_IMAGE}, {L"tif", FILE_TYPE_IMAGE}, {L"tiff", FILE_TYPE_IMAGE},
    {L"ico", FILE_TYPE_IMAGE}, {L"heic", FILE_TYPE_IMAGE}, {L"heif", FILE_TYPE_IMAGE}, {L"webp", FILE_TYPE_IMAGE},
    {L"psd", FILE_TYPE_IMAGE}, {L"svg", FILE_TYPE_IMAGE}, {L"raw", FILE_TYPE_IMAGE}, {L"cr2", FILE_TYPE_IMAGE},
    {L"nef", FILE_TYPE_IMAGE}, {L"arw", FILE_TYPE_IMAGE}, {L"dng", FILE_TYPE_IMAGE},

    {L"avi", FILE_TYPE_VIDEO}, {L"mov", FILE_TYPE_VIDEO}, {L"wmv", FILE_TYPE_VIDEO}, {L"mp4", FILE_TYPE_VIDEO},
    {L"mkv", FILE_TYPE_VIDEO}, {L"m4v", FILE_TYPE_VIDEO}, {L"mpg", FILE_TYPE_VIDEO}, {L"mpeg", FILE_TYPE_VIDEO},
    {L"3gp", FILE_TYPE_VIDEO}, {L"webm", FILE_TYPE_VIDEO}, {L"flv", FILE_TYPE_VIDEO}, {L"vob", FILE_TYPE_VIDEO},
    {L"mts", FILE_TYPE_VIDEO}, {L"m2ts", FILE_TYPE_VIDEO},

    {L"mp3", FILE_TYPE_MUSIC}, {L"wma", FILE_TYPE_MUSIC}, {L"wav", FILE_TYPE_MUSIC}, {L"flac", FILE_TYPE_MUSIC},
    {L"m4a", FILE_TYPE_MUSIC}, {L"ogg", FILE_TYPE_MUSIC}, {L"aac", FILE_TYPE_MUSIC}, {L"aif", FILE_TYPE_MUSIC},
    {L"aiff", FILE_TYPE_MUSIC}, {L"opus", FILE_TYPE_MUSIC}, {L"mid", FILE_TYPE_MUSIC}, {L"midi", FILE_TYPE_MUSIC},

    {L"pdf", FILE_TYPE_DOCUMENT}, {L"doc", FILE_TYPE_DOCUMENT}, {L"docx", FILE_TYPE_DOCUMENT}, {L"xls", FILE_TYPE_DOCUMENT},
    {L"xlsx", FILE_TYPE_DOCUMENT}, {L"ppt", FILE_TYPE_DOCUMENT}, {L"pptx", FILE_TYPE_DOCUMENT}, {L"odt", FILE_TYPE_DOCUMENT},
    {L"ods", FILE_TYPE_DOCUMENT}, {L"odp", FILE_TYPE_DOCUMENT}, {L"rtf", FILE_TYPE_DOCUMENT}, {L"epub", FILE_TYPE_DOCUMENT},
    {L"pages", FILE_TYPE_DOCUMENT}, {L"numbers", FILE_TYPE_DOCUMENT}, {L"key", FILE_TYPE_DOCUMENT},

    {L"txt", FILE_TYPE_TEXT}, {L"csv", FILE_TYPE_TEXT}, {L"log", FILE_TYPE_TEXT}, {L"md", FILE_TYPE_TEXT},
    {L"xml", FILE_TYPE_TEXT}, {L"json", FILE_TYPE_TEXT}, {L"htm", FILE_TYPE_TEXT}, {L"html", FILE_TYPE_TEXT},
    {L"ini", FILE_TYPE_TEXT},

    {L"zip", FILE_TYPE_ARCHIVE}, {L"rar", FILE_TYPE_ARCHIVE}, {L"7z", FILE_TYPE_ARCHIVE}, {L"gz", FILE_TYPE_ARCHIVE},
    {L"tgz", FILE_TYPE_ARCHIVE}, {L"tar", FILE_TYPE_ARCHIVE}, {L"bz2", FILE_TYPE_ARCHIVE}, {L"xz", FILE_TYPE_ARCHIVE},
    {L"cab", FILE_TYPE_ARCHIVE}, {L"iso", FILE_TYPE_ARCHIVE},
};

int FileTypeOf(const wchar_t *szFilename) {
    const wchar_t *pDot = wcsrchr(szFilename, L'.');
    if (!pDot || pDot == szFilename || !pDot[1]) // no extension (".profile" is a name, not an extension)
        return FILE_TYPE_OTHER;
    for (size_t i = 0; i < sizeof (s_aFileTypes) / sizeof (s_aFileTypes[0]); i++)
        if (!_wcsicmp(pDot + 1, s_aFileTypes[i].szExtension))
            return s_aFileTypes[i].iType;
    return FILE_TYPE_OTHER;
}

wxString FileTypeFilterLabel(int iType) {
    switch (iType) {
        case FILE_TYPE_ANY: return Tr(L"All types");
        case FILE_TYPE_IMAGE: return Tr(L"Images");
        case FILE_TYPE_VIDEO: return Tr(L"Videos");
        case FILE_TYPE_MUSIC: return Tr(L"Music");
        case FILE_TYPE_DOCUMENT: return Tr(L"Documents");
        case FILE_TYPE_TEXT: return Tr(L"Text");
        case FILE_TYPE_ARCHIVE: return Tr(L"Archives");
        default: return Tr(L"Other");
    }
}

wxString RecoverabilityFilterLabel(int iFilter) {
    switch (iFilter) {
        case RECOVERABILITY_FILTER_GOOD: return Tr(L"Good only");
        case RECOVERABILITY_FILTER_NOT_OVERWRITTEN: return Tr(L"Hide overwritten");
        default: return Tr(L"All files");
    }
}

wxString FormatFileTime(LONGLONG llFileTime) {
    if (llFileTime <= 0)
        return wxEmptyString;
    FILETIME ftUtc, ftLocal;
    SYSTEMTIME st;
    ftUtc.dwLowDateTime = (DWORD) llFileTime;
    ftUtc.dwHighDateTime = (DWORD) (llFileTime >> 32);
    if (!FileTimeToLocalFileTime(&ftUtc, &ftLocal) || !FileTimeToSystemTime(&ftLocal, &st))
        return wxEmptyString;
    return wxString::Format("%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
}

wxString RecoverabilityText(const t_FileInfo &file) {
    // NTFS: the data on the drive isn't the file's - recovering would give ciphertext / compressed blocks
    if (file.dwAttributes & FILE_ATTRIBUTE_ENCRYPTED)
        return Tr(L"Encrypted - can't be recovered");
    if (file.dwAttributes & FILE_ATTRIBUTE_COMPRESSED)
        return Tr(L"Compressed - can't be recovered");
    switch (file.iRecoverability) {
        case RECOVERABILITY_GOOD: return Tr(L"Good");
        case RECOVERABILITY_PARTIAL: return wxString::Format(Tr(L"Partly · %d %% intact"), file.iPercentIntact);
        case RECOVERABILITY_OVERWRITTEN: return Tr(L"Overwritten");
        default: return Tr(L"Unknown");
    }
}

wxColour RecoverabilityColour(int iRecoverability) {
    switch (iRecoverability) {
        case RECOVERABILITY_GOOD: return Theme::Good;
        case RECOVERABILITY_PARTIAL: return Theme::Partial;
        default: return Theme::Overwritten;
    }
}

wxString ContentText(int iContent) {
    if (iContent == CONTENT_NOT_CHECKED)
        return Tr(L"Not checked");
    return Tr(wxString(ContentLabel(iContent)));
}

wxColour ContentColour(int iContent) {
    switch (iContent) {
        case CONTENT_OK: return Theme::Good;
        case CONTENT_DAMAGED:
        case CONTENT_ZEROS: return Theme::Damaged;
        case CONTENT_NOT_CHECKED: return Theme::TextDisabled;
        default: return Theme::Overwritten;
    }
}

CDeletedFilesList::CDeletedFilesList(wxWindow *parent, wxWindowID id)
: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxBORDER_NONE) {
    m_iSortColumn = DELETED_FILES_COLUMN_RECOVERABILITY; // the most useful order: what can still be recovered first
    m_bSortAscending = true;
    m_iRecoverabilityFilter = RECOVERABILITY_FILTER_ALL;
    m_iTypeFilter = FILE_TYPE_ANY;
    m_bValidOnly = false;
    m_iCheckedShown = 0;
    m_llCheckedShownBytes = 0;
    SetFont(Theme::Small());

    // one canvas for all images: 20 x 30 - it makes the rows 30 px high
    wxSize canvas(20, 30);
    wxVector<wxBitmapBundle> vImages;
    vImages.push_back(Theme::CheckBox(false, canvas, 16));
    vImages.push_back(Theme::CheckBox(true, canvas, 16));
    for (int iType = FILE_TYPE_IMAGE; iType <= FILE_TYPE_OTHER; iType++)
        vImages.push_back(Theme::IconOnCanvas(Theme::FileTypeIcon(iType), wxColour(0x44, 0x44, 0x44), canvas, 18));
    vImages.push_back(Theme::Dot(Theme::GoodDot, canvas, 8));
    vImages.push_back(Theme::Dot(Theme::PartialDot, canvas, 8));
    vImages.push_back(Theme::Dot(Theme::OverwrittenDot, canvas, 8));
    vImages.push_back(Theme::Dot(Theme::Border, canvas, 8));
    SetSmallImages(vImages);

    InsertColumn(DELETED_FILES_COLUMN_CHECK, wxEmptyString, wxLIST_FORMAT_LEFT, FromDIP(34));
    InsertColumn(DELETED_FILES_COLUMN_NAME, Tr(L"Name"), wxLIST_FORMAT_LEFT, FromDIP(230));
    InsertColumn(DELETED_FILES_COLUMN_FOLDER, Tr(L"Folder"), wxLIST_FORMAT_LEFT, FromDIP(170));
    InsertColumn(DELETED_FILES_COLUMN_SIZE, Tr(L"Size"), wxLIST_FORMAT_RIGHT, FromDIP(80));
    InsertColumn(DELETED_FILES_COLUMN_MODIFIED, Tr(L"Modified"), wxLIST_FORMAT_LEFT, FromDIP(120));
    InsertColumn(DELETED_FILES_COLUMN_RECOVERABILITY, Tr(L"Recoverable"), wxLIST_FORMAT_LEFT, FromDIP(160));
    InsertColumn(DELETED_FILES_COLUMN_CONTENTS, Tr(L"Contents"), wxLIST_FORMAT_LEFT, FromDIP(110));
    ShowSortIndicator(m_iSortColumn, m_bSortAscending);
    UpdateHeaderCheckBox();

    wxColour aText[CELL_KINDS] = {Theme::Text, Theme::TextSecondary, Theme::Good, Theme::Partial, Theme::Overwritten,
        Theme::Damaged, Theme::TextDisabled};
    for (int i = 0; i < CELL_KINDS; i++)
        for (int iChecked = 0; iChecked < 2; iChecked++) {
            m_aAttr[i][iChecked].SetTextColour(aText[i]);
            m_aAttr[i][iChecked].SetBackgroundColour(iChecked ? Theme::RowChecked : Theme::PanelBg);
        }

    SetItemCount(0);

    Bind(wxEVT_LEFT_DOWN, &CDeletedFilesList::OnLeftDown, this);
    Bind(wxEVT_LEFT_DCLICK, &CDeletedFilesList::OnLeftDown, this);
    Bind(wxEVT_LIST_KEY_DOWN, &CDeletedFilesList::OnKeyDown, this);
    Bind(wxEVT_LIST_COL_CLICK, &CDeletedFilesList::OnColumnClick, this);
    Bind(wxEVT_SIZE, &CDeletedFilesList::OnSize, this);
}

void CDeletedFilesList::OnSize(wxSizeEvent &event) {
    event.Skip();
    CallAfter([this] { FitColumns(); }); // after the list has its new size (and knows about its scroll bar)
}

// The tick box, size, date and contents have their width; name, folder and recoverability share the rest,
// like the design's grid: 2.2 : 1.6 : 1.3

void CDeletedFilesList::FitColumns() {
    // room for the vertical scroll bar even before it shows - it comes with the files, after the last resize
    int iWidth = GetSize().x - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);
    int aFixed[] = {DELETED_FILES_COLUMN_CHECK, DELETED_FILES_COLUMN_SIZE, DELETED_FILES_COLUMN_MODIFIED, DELETED_FILES_COLUMN_CONTENTS};
    int aFixedWidth[] = {34, 76, 120, 120};
    for (size_t i = 0; i < WXSIZEOF(aFixed); i++) {
        SetColumnWidth(aFixed[i], FromDIP(aFixedWidth[i]));
        iWidth -= FromDIP(aFixedWidth[i]);
    }
    // each gets its minimum, the rest is shared - or, if even the minimums don't fit, they shrink alike
    int aFlexible[] = {DELETED_FILES_COLUMN_NAME, DELETED_FILES_COLUMN_FOLDER, DELETED_FILES_COLUMN_RECOVERABILITY};
    double aShare[] = {2.2, 1.6, 1.3};
    int aMinimum[] = {120, 70, 130};
    iWidth = wxMax(iWidth - FromDIP(2), 0); // rounding must not bring the horizontal scroll bar back
    int iMinimums = 0;
    for (size_t i = 0; i < WXSIZEOF(aFlexible); i++)
        iMinimums += FromDIP(aMinimum[i]);
    int iRest = iWidth - iMinimums;
    for (size_t i = 0; i < WXSIZEOF(aFlexible); i++) {
        int iColumn = (iRest >= 0) ? FromDIP(aMinimum[i]) + (int) (iRest * aShare[i] / 5.1)
                : FromDIP(aMinimum[i]) * iWidth / wxMax(iMinimums, 1);
        SetColumnWidth(aFlexible[i], iColumn);
    }
}

void CDeletedFilesList::AddFiles(const std::vector<t_FileInfo *> &vFiles) {
    if (vFiles.empty())
        return;
    for (size_t i = 0; i < vFiles.size(); i++) {
        int iType = FileTypeOf(vFiles[i]->sFilename.c_str());
        m_vAll.push_back(vFiles[i]);
        m_vAllTypes.push_back((unsigned char) iType);
        if (PassesFilters(vFiles[i], iType))
            m_vVisible.push_back(vFiles[i]);
    }
    SetItemCount((long) m_vVisible.size()); // the rows on screen stay where they are
    UpdateHeaderCheckBox();
}

void CDeletedFilesList::SetFiles(FileInfoHash *finfohash) {
    m_vAll.clear();
    m_vAllTypes.clear();
    m_vAll.reserve(finfohash->size());
    m_vAllTypes.reserve(finfohash->size());
    std::unordered_set<const t_FileInfo *> setChecked;
    for (FileInfoHash::iterator it = finfohash->begin(); it != finfohash->end(); ++it) {
        m_vAll.push_back(it->second);
        m_vAllTypes.push_back((unsigned char) FileTypeOf(it->second->sFilename.c_str()));
        if (m_setChecked.count(it->second))
            setChecked.insert(it->second);
    }
    m_setChecked.swap(setChecked);
    Rebuild();
    NotifyChecked();
}

void CDeletedFilesList::Clear() {
    m_vAll.clear();
    m_vAllTypes.clear();
    m_vVisible.clear();
    m_setChecked.clear();
    SetItemCount(0);
    CountChecked();
    UpdateHeaderCheckBox();
    Refresh();
}

void CDeletedFilesList::FilterChanged() {
    Rebuild();
    NotifyChecked(); // the ticked files that are hidden now don't count
}

void CDeletedFilesList::SetNameFilter(const wxString &sFilter) {
    m_sFilterUpper = sFilter.Upper();
    FilterChanged();
}

void CDeletedFilesList::SetRecoverabilityFilter(int iFilter) {
    m_iRecoverabilityFilter = iFilter;
    FilterChanged();
}

void CDeletedFilesList::SetTypeFilter(int iType) {
    m_iTypeFilter = iType;
    FilterChanged();
}

void CDeletedFilesList::SetValidOnly(bool bValidOnly) {
    m_bValidOnly = bValidOnly;
    FilterChanged();
}

void CDeletedFilesList::ClearFilters() {
    m_sFilterUpper.clear();
    m_iRecoverabilityFilter = RECOVERABILITY_FILTER_ALL;
    m_iTypeFilter = FILE_TYPE_ANY;
    m_bValidOnly = false;
    FilterChanged();
}

void CDeletedFilesList::CountByType(int aCounts[FILE_TYPE_COUNT]) const {
    for (int i = 0; i < FILE_TYPE_COUNT; i++)
        aCounts[i] = 0;
    for (size_t i = 0; i < m_vAll.size(); i++)
        if (PassesFilters(m_vAll[i], m_vAllTypes[i], false)) {
            aCounts[FILE_TYPE_ANY]++;
            aCounts[m_vAllTypes[i]]++;
        }
}

bool CDeletedFilesList::PassesFilters(const t_FileInfo *pFile, int iType, bool bTypeFilter) const {
    if (m_iRecoverabilityFilter == RECOVERABILITY_FILTER_GOOD && pFile->iRecoverability != RECOVERABILITY_GOOD)
        return false;
    if (m_iRecoverabilityFilter == RECOVERABILITY_FILTER_NOT_OVERWRITTEN && pFile->iRecoverability == RECOVERABILITY_OVERWRITTEN)
        return false;
    if (m_bValidOnly && pFile->iContent != CONTENT_OK)
        return false;
    if (bTypeFilter && m_iTypeFilter != FILE_TYPE_ANY && iType != m_iTypeFilter)
        return false;
    if (!m_sFilterUpper.IsEmpty()) { // the whole path, so "Pictures\IMG" or a folder name finds files too
        wxString sPath(pFile->sFolder.c_str());
        if (!sPath.EndsWith("\\"))
            sPath += "\\";
        sPath += pFile->sFilename.c_str();
        if (!sPath.Upper().Contains(m_sFilterUpper))
            return false;
    }
    return true;
}

void CDeletedFilesList::SortByColumn(int iColumn) {
    if (iColumn <= DELETED_FILES_COLUMN_CHECK || iColumn >= DELETED_FILES_COLUMN_COUNT)
        return;
    if (iColumn == m_iSortColumn)
        m_bSortAscending = !m_bSortAscending;
    else {
        m_iSortColumn = iColumn;
        m_bSortAscending = true;
    }
    ShowSortIndicator(m_iSortColumn, m_bSortAscending);
    Rebuild();
}

t_FileInfo *CDeletedFilesList::GetFile(long lItem) const {
    if (lItem < 0 || lItem >= (long) m_vVisible.size())
        return NULL;
    return m_vVisible[lItem];
}

t_FileInfo *CDeletedFilesList::GetFocusedFile() const {
    return GetFile(GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED));
}

void CDeletedFilesList::RefreshFile(const t_FileInfo *pFile) {
    std::vector<t_FileInfo *>::const_iterator it = std::find(m_vVisible.begin(), m_vVisible.end(), pFile);
    if (it != m_vVisible.end())
        RefreshItem((long) (it - m_vVisible.begin()));
}

void CDeletedFilesList::GetCheckedFiles(std::vector<t_FileInfo *> &vChecked) const {
    vChecked.clear();
    if (m_setChecked.empty())
        return;
    for (size_t i = 0; i < m_vVisible.size(); i++)
        if (m_setChecked.count(m_vVisible[i]))
            vChecked.push_back(m_vVisible[i]);
}

void CDeletedFilesList::CountChecked() {
    m_iCheckedShown = 0;
    m_llCheckedShownBytes = 0;
    if (m_setChecked.empty())
        return;
    for (size_t i = 0; i < m_vVisible.size(); i++)
        if (m_setChecked.count(m_vVisible[i])) {
            m_iCheckedShown++;
            m_llCheckedShownBytes += m_vVisible[i]->n64RealSize;
        }
}

// for a file that is shown - the counts are of shown files

void CDeletedFilesList::SetChecked(t_FileInfo *pFile, bool bCheck) {
    if (bCheck == (m_setChecked.count(pFile) != 0))
        return;
    if (bCheck) {
        m_setChecked.insert(pFile);
        m_iCheckedShown++;
        m_llCheckedShownBytes += pFile->n64RealSize;
    } else {
        m_setChecked.erase(pFile);
        m_iCheckedShown--;
        m_llCheckedShownBytes -= pFile->n64RealSize;
    }
}

void CDeletedFilesList::CheckAllShown(bool bCheck) {
    for (size_t i = 0; i < m_vVisible.size(); i++)
        SetChecked(m_vVisible[i], bCheck);
    Refresh();
    NotifyChecked();
}

void CDeletedFilesList::CheckFiles(const std::vector<t_FileInfo *> &vFiles, bool bCheck) {
    for (size_t i = 0; i < vFiles.size(); i++)
        SetChecked(vFiles[i], bCheck);
    Refresh();
    NotifyChecked();
}

void CDeletedFilesList::NotifyChecked() {
    UpdateHeaderCheckBox();
    wxCommandEvent event(EVT_DELETED_FILES_CHECKED, GetId());
    event.SetEventObject(this);
    ProcessWindowEvent(event);
}

// the header of the tick column shows whether every shown file is ticked - a click on it ticks them all, or none

void CDeletedFilesList::UpdateHeaderCheckBox() {
    wxListItem column;
    column.SetMask(wxLIST_MASK_IMAGE);
    column.SetImage(AreAllShownChecked() ? IMAGE_CHECKED : IMAGE_UNCHECKED);
    SetColumn(DELETED_FILES_COLUMN_CHECK, column);
}

void CDeletedFilesList::OnColumnClick(wxListEvent &event) {
    if (event.GetColumn() == DELETED_FILES_COLUMN_CHECK)
        CheckAllShown(!AreAllShownChecked());
    else
        SortByColumn(event.GetColumn());
}

// a click on the tick box ticks the file (the row also gets the focus, as for any click)

void CDeletedFilesList::OnLeftDown(wxMouseEvent &event) {
    int iFlags = 0;
    long lColumn = -1;
    long lItem = HitTest(event.GetPosition(), iFlags, &lColumn);
    if (lItem >= 0 && lColumn == DELETED_FILES_COLUMN_CHECK) {
        t_FileInfo *pFile = GetFile(lItem);
        if (pFile) {
            SetChecked(pFile, !m_setChecked.count(pFile));
            RefreshItem(lItem);
            NotifyChecked();
        }
    }
    event.Skip();
}

// Space ticks (or unticks) the highlighted files, Ctrl+A ticks every shown file

void CDeletedFilesList::OnKeyDown(wxListEvent &event) {
    if ((event.GetKeyCode() == 'A' || event.GetKeyCode() == 'a') && wxGetKeyState(WXK_CONTROL)) {
        CheckAllShown(true);
        return;
    }
    if (event.GetKeyCode() == WXK_SPACE) {
        std::vector<t_FileInfo *> vHighlighted;
        bool bAllChecked = true;
        long lItem = -1;
        while ((lItem = GetNextItem(lItem, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
            if (t_FileInfo *pFile = GetFile(lItem)) {
                vHighlighted.push_back(pFile);
                bAllChecked = bAllChecked && m_setChecked.count(pFile);
            }
        if (!vHighlighted.empty())
            CheckFiles(vHighlighted, !bAllChecked);
        return;
    }
    event.Skip();
}

void CDeletedFilesList::Rebuild() {
    t_FileInfo *pFocused = GetFocusedFile();

    m_vVisible.clear();
    if (m_sFilterUpper.IsEmpty() && m_iRecoverabilityFilter == RECOVERABILITY_FILTER_ALL && m_iTypeFilter == FILE_TYPE_ANY && !m_bValidOnly)
        m_vVisible = m_vAll;
    else
        for (size_t i = 0; i < m_vAll.size(); i++)
            if (PassesFilters(m_vAll[i], m_vAllTypes[i]))
                m_vVisible.push_back(m_vAll[i]);

    DeletedFilesOrder order;
    order.iColumn = m_iSortColumn;
    order.bAscending = m_bSortAscending;
    std::sort(m_vVisible.begin(), m_vVisible.end(), order);

    // the highlight belongs to row numbers - clear it, keep only the focus on the same file
    long lItem = -1;
    while ((lItem = GetNextItem(lItem, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
        SetItemState(lItem, 0, wxLIST_STATE_SELECTED);
    SetItemCount((long) m_vVisible.size());
    if (pFocused) {
        std::vector<t_FileInfo *>::const_iterator it = std::find(m_vVisible.begin(), m_vVisible.end(), pFocused);
        if (it != m_vVisible.end()) {
            long lFocus = (long) (it - m_vVisible.begin());
            SetItemState(lFocus, wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED, wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED);
            EnsureVisible(lFocus);
        }
    }
    CountChecked();
    UpdateHeaderCheckBox();
    Refresh();
}

wxString CDeletedFilesList::OnGetItemText(long lItem, long lColumn) const {
    const t_FileInfo *pFile = GetFile(lItem);
    if (!pFile)
        return wxEmptyString;

    switch (lColumn) {
        case DELETED_FILES_COLUMN_NAME:
            return wxString(pFile->sFilename.c_str());
        case DELETED_FILES_COLUMN_FOLDER:
            return wxString(pFile->sFolder.c_str());
        case DELETED_FILES_COLUMN_SIZE:
            return FormatFileSize(pFile->n64RealSize);
        case DELETED_FILES_COLUMN_MODIFIED:
            return FormatFileTime(pFile->n64Modify);
        case DELETED_FILES_COLUMN_CONTENTS:
            return ContentText(pFile->iContent);
        case DELETED_FILES_COLUMN_RECOVERABILITY:
            return RecoverabilityText(*pFile);
    }
    return wxEmptyString;
}

int CDeletedFilesList::OnGetItemImage(long lItem) const {
    const t_FileInfo *pFile = GetFile(lItem);
    return (pFile && m_setChecked.count(pFile)) ? IMAGE_CHECKED : IMAGE_UNCHECKED;
}

int CDeletedFilesList::OnGetItemColumnImage(long lItem, long lColumn) const {
    const t_FileInfo *pFile = GetFile(lItem);
    if (!pFile)
        return -1;
    switch (lColumn) {
        case DELETED_FILES_COLUMN_CHECK:
            return OnGetItemImage(lItem);
        case DELETED_FILES_COLUMN_NAME:
            return IMAGE_FILE_TYPE + FileTypeOf(pFile->sFilename.c_str()) - FILE_TYPE_IMAGE;
        case DELETED_FILES_COLUMN_RECOVERABILITY:
            switch (pFile->iRecoverability) {
                case RECOVERABILITY_GOOD: return IMAGE_DOT_GOOD;
                case RECOVERABILITY_PARTIAL: return IMAGE_DOT_PARTIAL;
                case RECOVERABILITY_OVERWRITTEN: return IMAGE_DOT_OVERWRITTEN;
                default: return IMAGE_DOT_UNKNOWN;
            }
    }
    return -1;
}

wxItemAttr *CDeletedFilesList::OnGetItemColumnAttr(long lItem, long lColumn) const {
    const t_FileInfo *pFile = GetFile(lItem);
    if (!pFile)
        return NULL;
    int iChecked = m_setChecked.count(pFile) ? 1 : 0;
    int iKind = CELL_TEXT;
    switch (lColumn) {
        case DELETED_FILES_COLUMN_FOLDER:
        case DELETED_FILES_COLUMN_MODIFIED:
            iKind = CELL_SECONDARY;
            break;
        case DELETED_FILES_COLUMN_RECOVERABILITY:
            iKind = pFile->iRecoverability == RECOVERABILITY_GOOD ? CELL_GOOD
                    : (pFile->iRecoverability == RECOVERABILITY_PARTIAL ? CELL_PARTIAL : CELL_OVERWRITTEN);
            break;
        case DELETED_FILES_COLUMN_CONTENTS:
            switch (pFile->iContent) {
                case CONTENT_OK: iKind = CELL_GOOD;
                    break;
                case CONTENT_DAMAGED:
                case CONTENT_ZEROS: iKind = CELL_DAMAGED;
                    break;
                case CONTENT_NOT_CHECKED: iKind = CELL_UNCHECKED;
                    break;
                default: iKind = CELL_OVERWRITTEN;
            }
            break;
    }
    return &m_aAttr[iKind][iChecked];
}
