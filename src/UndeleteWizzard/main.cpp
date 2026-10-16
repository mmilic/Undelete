#include "main.h"
#include <wx/filename.h>
#include "AppLog.h"
#include "WizardFrame.h"
#include "languages/LanguageDialog.h"
#include "languages/LanguageFactory.h"
#include "languages/Translate.h"

IMPLEMENT_APP(MyApp)

void MyApp::OnInitCmdLine(wxCmdLineParser &parser) {
    wxApp::OnInitCmdLine(parser);
    parser.AddParam("disk image", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
}

bool MyApp::OnCmdLineParsed(wxCmdLineParser &parser) {
    for (size_t i = 0; i < parser.GetParamCount(); i++) {
        wxFileName image(parser.GetParam(i));
        image.MakeAbsolute();
        m_vImages.push_back(image.GetFullPath());
    }
    return wxApp::OnCmdLineParsed(parser);
}

// `Main program' equivalent: the program execution "starts" here

bool MyApp::OnInit() {
    // before anything logs: no message boxes from the log, lines kept until the log file is open
    CAppLog::Install();
    if (!wxApp::OnInit())
        return false;

    // JPEG, PNG, GIF, TIFF... decoders for the image preview
    wxInitAllImageHandlers();

    SetUiLanguage(LanguageFactory::Get()->CreateLanguage("English"));
    {
        LanguageDialog dialog;
        if (dialog.ShowModal() != wxID_OK)
            return false;
        if (ILanguage *pLanguage = LanguageFactory::Get()->CreateLanguage(dialog.GetLanguage()))
            SetUiLanguage(pLanguage);
    }

    CWizardFrame *pFrame = new CWizardFrame(m_vImages);
    pFrame->Show();
    return true;
}
