/* 
 * File:   main.h
 * Author: mmilic
 *
 * Created on October 20, 2012, 10:59 AM
 */

#ifndef MAIN_H
#define	MAIN_H



#endif	/* MAIN_H */

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/frame.h"
    #include "wx/stattext.h"
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/checkbox.h"
    #include "wx/checklst.h"
    #include "wx/msgdlg.h"
    #include "wx/radiobox.h"
    #include "wx/menu.h"
    #include "wx/sizer.h"
#endif

#include "wx/textctrl.h"
#include "wx/wizard.h"

#include "resources/wiztest.xpm"
#include "resources/wiztest2.xpm"

#include "resources/sample.xpm"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// ids for menu items
enum
{
    Wizard_About = wxID_ABOUT,
    Wizard_Quit = wxID_EXIT,
    Wizard_RunModal = wxID_HIGHEST,

    Wizard_RunNoSizer,
    Wizard_RunModeless,

    Wizard_LargeWizard,
    Wizard_ExpandBitmap
};



// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------


// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    virtual bool OnInit();
};




/*
 
 celine:				treba da ima includovano

MFTRecord.h				nista
NTFSDrive.h				<wx/hashmap.h>
MyThreadClass.h			NTFSDrive.h
MyWizzard.h				NTFSDrive, MFTRecord, MyThreadClass


MFTRecord.cpp			MFTRecord.h	
NTFSDrive.cpp			MFTRecord.h, NTFSDrive.h
MyThreadClass.cpp		MFTRecord.h, NTFSDrive.h
MyWizzard.cpp			NTFSDrive, MFTRecord, MyThreadClass
-------------
UndeleteGui
 
 */