
#include "main.h"
#include "RecoveryDrive.h"

#include <wx/listctrl.h>
#include <wx/progdlg.h>
#include "MyWizzard.h"

IMPLEMENT_APP(MyApp)

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit() {
    if (!wxApp::OnInit())
        return false;

    // JPEG, PNG, GIF, TIFF... decoders for the image preview on the "select files" page
    wxInitAllImageHandlers();

    //    MyFrame *frame = new MyFrame(wxT("wxWizard Sample"));
    MyWizard wizard(NULL);
    wizard.RunWizard(wizard.GetFirstPage());

    // we're done
    exit(0);
}

