// FileDetailsPanel.cpp: preview and details of the focused file

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/mstream.h>
#include "FileDetailsPanel.h"
#include "DeletedFilesList.h"
#include "FileContents.h"
#include "Theme.h"
#include "languages/Translate.h"

#define MAX_PREVIEW_FILE_SIZE (32 * 1024 * 1024)

// sText broken into lines at spaces, none wider than iWidth

static wxString WrapText(wxDC &dc, const wxString &sText, int iWidth) {
    wxString sResult, sLine;
    wxArrayString asWords = wxSplit(sText, ' ', 0);
    for (size_t i = 0; i < asWords.size(); i++) {
        wxString sTry = sLine.IsEmpty() ? asWords[i] : sLine + " " + asWords[i];
        if (!sLine.IsEmpty() && dc.GetTextExtent(sTry).x > iWidth) {
            sResult += sLine + "\n";
            sLine = asWords[i];
        } else {
            sLine = sTry;
        }
    }
    return sResult + sLine;
}

// the grey box: the decoded image, or the file type's icon with a word on why there is no preview

class CPreviewBox : public wxWindow {
public:
    CPreviewBox(wxWindow *parent, const wxSize &size) : wxWindow(parent, wxID_ANY, wxDefaultPosition, size) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetMinSize(size);
        SetFont(Theme::Tiny());
        m_iFileType = FILE_TYPE_OTHER;
        Bind(wxEVT_PAINT, &CPreviewBox::OnPaint, this);
    }

    void ShowImage(const wxImage &image) {
        m_image = image;
        m_sMessage.Clear();
        Refresh();
    }

    void ShowIcon(int iFileType, const wxString &sMessage) {
        m_image = wxImage();
        m_iFileType = iFileType;
        m_sMessage = sMessage;
        Refresh();
    }

private:
    wxImage m_image;
    int m_iFileType;
    wxString m_sMessage;

    void OnPaint(wxPaintEvent &WXUNUSED(event)) {
        wxAutoBufferedPaintDC dcPaint(this);
        dcPaint.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
        dcPaint.Clear();
        wxGCDC dc(dcPaint);
        wxRect rect = GetClientRect();
        Theme::DrawRoundedBox(dc, rect, FromDIP(6), Theme::WindowBg, wxNullColour);

        if (m_image.IsOk()) {
            // fit the box, never enlarged
            double dScale = wxMin(1.0, wxMin((double) rect.width / m_image.GetWidth(), (double) rect.height / m_image.GetHeight()));
            int w = wxMax(1, (int) (m_image.GetWidth() * dScale)), h = wxMax(1, (int) (m_image.GetHeight() * dScale));
            wxBitmap bitmap(dScale < 1.0 ? m_image.Scale(w, h, wxIMAGE_QUALITY_HIGH) : m_image);
            dc.DrawBitmap(bitmap, rect.x + (rect.width - w) / 2, rect.y + (rect.height - h) / 2, true);
            return;
        }

        wxBitmap icon = Theme::Icon(Theme::FileTypeIcon(m_iFileType), Theme::TextDisabled, 56, 1.2).GetBitmapFor(this);
        int iTextHeight = 0;
        dc.SetFont(GetFont());
        wxString sMessage = WrapText(dc, m_sMessage, rect.width - FromDIP(24));
        if (!sMessage.IsEmpty())
            iTextHeight = dc.GetMultiLineTextExtent(sMessage).y + FromDIP(6);
        int y = rect.y + (rect.height - icon.GetLogicalHeight() - iTextHeight) / 2;
        dc.DrawBitmap(icon, rect.x + (rect.width - icon.GetLogicalWidth()) / 2, y, true);
        if (!sMessage.IsEmpty()) {
            dc.SetTextForeground(Theme::TextSecondary);
            wxRect text(rect.x + FromDIP(8), y + icon.GetLogicalHeight() + FromDIP(6), rect.width - FromDIP(16), iTextHeight);
            dc.DrawLabel(sMessage, text, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP); // centres each line
        }
    }
};

// image formats the preview can decode (wxWidgets has no HEIC, WebP, RAW... decoders)

static bool CanPreview(const wchar_t *szFilename) {
    static const char *const PreviewExtensions[] = {"jpg", "jpeg", "jpe", "gif", "png", "bmp", "tif", "tiff", "ico"};
    wxString name(szFilename);
    name.MakeLower();
    for (size_t i = 0; i < sizeof (PreviewExtensions) / sizeof (PreviewExtensions[0]); i++)
        if (name.EndsWith(wxString(".") + PreviewExtensions[i]))
            return true;
    return false;
}

static wxStaticText *DetailLabel(wxWindow *parent, const wxString &sText) {
    wxStaticText *pLabel = new wxStaticText(parent, wxID_ANY, sText);
    pLabel->SetFont(Theme::Small());
    pLabel->SetForegroundColour(Theme::TextSecondary);
    return pLabel;
}

static wxStaticText *DetailValue(wxWindow *parent, int iWidth) {
    // long folders lose their middle, not their end
    wxStaticText *pValue = new wxStaticText(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(iWidth, -1),
            wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE);
    pValue->SetFont(Theme::Small());
    pValue->SetForegroundColour(Theme::Text);
    return pValue;
}

CFileDetailsPanel::CFileDetailsPanel(wxWindow *parent, int iWidth)
: wxPanel(parent, wxID_ANY) {
    m_pFile = NULL;
    m_pDrive = NULL;
    m_iWidth = iWidth;
    SetBackgroundColour(Theme::PanelBg);

    int iBoxWidth = FromDIP(iWidth);
    m_pPreview = new CPreviewBox(this, wxSize(iBoxWidth, FromDIP(160)));
    m_pName = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(iBoxWidth, -1),
            wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE);
    m_pName->SetFont(Theme::Bold(Theme::Body()));

    int iValueWidth = iBoxWidth - FromDIP(92);
    wxFlexGridSizer *pGrid = new wxFlexGridSizer(2, FromDIP(6), FromDIP(4));
    pGrid->AddGrowableCol(1);
    pGrid->Add(DetailLabel(this, Tr(L"Folder")));
    pGrid->Add(m_pFolder = DetailValue(this, iValueWidth), 1, wxEXPAND);
    pGrid->Add(DetailLabel(this, Tr(L"Size")));
    pGrid->Add(m_pSize = DetailValue(this, iValueWidth), 1, wxEXPAND);
    pGrid->Add(DetailLabel(this, Tr(L"Modified")));
    pGrid->Add(m_pModified = DetailValue(this, iValueWidth), 1, wxEXPAND);
    pGrid->Add(DetailLabel(this, Tr(L"Recoverable")));
    pGrid->Add(m_pRecoverable = DetailValue(this, iValueWidth), 1, wxEXPAND);
    pGrid->Add(DetailLabel(this, Tr(L"Contents")));
    pGrid->Add(m_pContents = DetailValue(this, iValueWidth), 1, wxEXPAND);
    pGrid->Add(m_pStatusLabel = DetailLabel(this, Tr(L"Status")));
    pGrid->Add(m_pStatus = DetailValue(this, iValueWidth), 1, wxEXPAND);
    pGrid->SetFlexibleDirection(wxHORIZONTAL);
    pGrid->SetItemMinSize((size_t) 0, FromDIP(88), -1);

    m_pHint = new wxStaticText(this, wxID_ANY, Tr(L"The preview is read straight from the drive - nothing is written to it."));
    m_pHint->SetFont(Theme::Tiny());
    m_pHint->SetForegroundColour(Theme::TextSecondary);
    m_pHint->Wrap(iBoxWidth);

    wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_pPreview, 0);
    pSizer->Add(m_pName, 0, wxEXPAND | wxTOP, FromDIP(12));
    pSizer->Add(pGrid, 0, wxEXPAND | wxTOP, FromDIP(8));
    pSizer->Add(m_pHint, 0, wxTOP, FromDIP(12));
    SetSizer(pSizer);

    m_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &CFileDetailsPanel::OnTimer, this);
    ShowFile(NULL, NULL);
}

void CFileDetailsPanel::UpdateTexts() {
    t_FileInfo *pFile = m_pFile;
    m_pName->SetLabel(pFile ? wxString(pFile->sFilename.c_str()) : wxString(Tr(L"No file selected")));
    m_pName->SetToolTip(pFile ? wxString(pFile->sFilename.c_str()) : wxString());
    m_pFolder->SetLabel(pFile ? wxString(pFile->sFolder.c_str()) : wxString());
    m_pFolder->SetToolTip(pFile ? wxString(pFile->sFolder.c_str()) : wxString());
    m_pSize->SetLabel(pFile ? FormatFileSize(pFile->n64RealSize) : wxString());
    m_pModified->SetLabel(pFile ? FormatFileTime(pFile->n64Modify) : wxString());
    m_pRecoverable->SetLabel(pFile ? RecoverabilityText(*pFile) : wxString());
    m_pRecoverable->SetForegroundColour(pFile ? RecoverabilityColour(pFile->iRecoverability) : Theme::Text);
    m_pContents->SetLabel(pFile ? ContentText(pFile->iContent) : wxString());
    m_pContents->SetForegroundColour(pFile ? ContentColour(pFile->iContent) : Theme::Text);
    bool bRecycled = pFile && pFile->bInRecycleBin;
    m_pStatus->SetLabel(bRecycled ? wxString(Tr(L"Deleted from the Recycle Bin")) : wxString());
    m_pStatus->Show(bRecycled);
    m_pStatusLabel->Show(bRecycled);
    Layout();
}

void CFileDetailsPanel::ShowFile(t_FileInfo *pFile, CRecoveryDrive *pDrive) {
    m_pFile = pFile;
    m_pDrive = pDrive;
    m_timer.Stop();
    UpdateTexts();

    if (!pFile) {
        m_pPreview->ShowIcon(FILE_TYPE_OTHER, Tr(L"Click a file to see its preview and details here."));
        return;
    }
    int iType = FileTypeOf(pFile->sFilename.c_str());
    if (!pDrive) {
        m_pPreview->ShowIcon(iType, Tr(L"No preview while the drive is busy"));
        return;
    }
    m_pPreview->ShowIcon(iType, wxEmptyString);
    // a short delay, so scrolling through the list with the arrow keys doesn't read every file on the way
    m_timer.StartOnce(250);
}

// Reads the deleted file's data from the drive (in the GUI thread - only while no scan or copy thread is using
// the drive) and tries to decode it as an image. If it doesn't decode, the data is most likely damaged.

void CFileDetailsPanel::OnTimer(wxTimerEvent &WXUNUSED(event)) {
    t_FileInfo *pFile = m_pFile;
    if (!pFile || !m_pDrive)
        return;

    if (pFile->iContent == CONTENT_NOT_CHECKED) {
        m_pDrive->CheckFileContents(*pFile);
        UpdateTexts();
        if (OnContentChecked)
            OnContentChecked(pFile);
    }

    int iType = FileTypeOf(pFile->sFilename.c_str());
    if (iType != FILE_TYPE_IMAGE)
        return;
    if (!CanPreview(pFile->sFilename.c_str())) {
        m_pPreview->ShowIcon(iType, Tr(L"No preview for this image format"));
        return;
    }
    if (pFile->n64RealSize > MAX_PREVIEW_FILE_SIZE) {
        m_pPreview->ShowIcon(iType, Tr(L"Too large for a preview"));
        return;
    }

    BYTE *puchData = NULL;
    DWORD dwLen = 0;
    int nRet;
    {
        wxBusyCursor busy;
        nRet = m_pDrive->Read_File(pFile->iRecordId, puchData, dwLen);
    }
    if (nRet) {
        m_pPreview->ShowIcon(iType, Tr(L"No preview - the data can't be read"));
        return;
    }

    wxImage image;
    {
        wxLogNull noLog; // a damaged image is expected here, not an error worth a message
        wxMemoryInputStream stream(puchData, dwLen);
        image.LoadFile(stream, wxBITMAP_TYPE_ANY);
    }
    delete[] puchData;

    if (image.IsOk())
        m_pPreview->ShowImage(image);
    else
        m_pPreview->ShowIcon(iType, Tr(L"No preview - the data is probably damaged"));
}
