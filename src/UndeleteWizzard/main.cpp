
#include "main.h"
#include "NTFSDrive.h"

#include <wx/listctrl.h>
#include <wx/progdlg.h>
#include "MyWizzard.h"

IMPLEMENT_APP(MyApp)

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit() {
    if (!wxApp::OnInit())
        return false;

    //provera datuma. Postoje dva bitna datuma:
    //DateWarningLimit - posle ovoga krece da javlja korisniku da mu istice pretplata i da treba da nabavi novu verziju
    //DateUsageLimit - posle ovoga krece da se gasi - odbija da radi
    wxDateTime now = wxDateTime::Now();
    wxDateTime DateWarningLimit = wxDateTime::Today();
    wxDateTime DateUsageLimit = wxDateTime::Today();

    DateWarningLimit.SetYear(2016);
    DateWarningLimit.SetMonth(wxDateTime::Jan);
    DateWarningLimit.SetDay(01);

    // debugging purpose
    //    Sleep(10000);

    DateUsageLimit.SetYear(2016);
    DateUsageLimit.SetMonth(wxDateTime::Oct);
    DateUsageLimit.SetDay(01);

    if (now > DateWarningLimit) {
        wxString myErrorMessage;
        myErrorMessage.Printf("This version of GetMyFilesBack expires on 01/01/2014.\nPlease contact us on \ninfo@getmyfilesback.com\nso that we can send you the newest version of our software for FREE!!!");

        wxMessageDialog *dial = new wxMessageDialog(NULL,
                myErrorMessage, wxT("Info"), wxOK | wxICON_INFORMATION);
        dial->ShowModal();
    }

    //if (now > DateUsageLimit)
    //    exit(0);

    //    MyFrame *frame = new MyFrame(wxT("wxWizard Sample"));
    MyWizard wizard(NULL);
    wizard.RunWizard(wizard.GetFirstPage());

    // we're done
    exit(0);
}

