/*
 * File:   main.h
 * Author: mmilic
 *
 * Created on October 20, 2012, 10:59 AM
 */

#ifndef MAIN_H
#define	MAIN_H

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <vector>

class MyApp : public wxApp {
public:
    bool OnInit() override;
    void OnInitCmdLine(wxCmdLineParser &parser) override;
    bool OnCmdLineParsed(wxCmdLineParser &parser) override;

private:
    // disk image files given on the command line - offered beside the drives (the test images, or a
    // copy of a failing disk)
    std::vector<wxString> m_vImages;
};

#endif	/* MAIN_H */
