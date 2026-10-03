#include "CustomButton.h"
 

wxCustomButton::wxCustomButton(wxFrame* parent, wxString text) :
 wxWindow(parent, wxID_ANY)
{
    SetMinSize( wxSize(buttonWidth, buttonHeight) );
    this->text = text;
    pressedDown = false;
}
 

wxCustomButton::wxCustomButton(wxDialog* parent, wxString text) :
 wxWindow(parent, wxID_ANY)
{
    SetMinSize( wxSize(buttonWidth, buttonHeight) );
    this->text = text;
    pressedDown = false;
}
 
/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */
 
void wxCustomButton::paintEvent(wxPaintEvent & evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}
 
/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void wxCustomButton::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}
 
/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void wxCustomButton::render(wxDC&  dc)
{
    if (pressedDown)
        dc.SetBrush( *wxRED_BRUSH );
    else
        dc.SetBrush( *wxGREY_BRUSH );
 
    //dc.DrawRectangle( 0, 0, buttonWidth, buttonHeight ); // ovde stavis drawcircle ili slicno da pravis okruglo dugme
    dc.DrawCircle(wxPoint(buttonHeight, buttonHeight),buttonHeight);
    dc.DrawText( text, 20, 15 );
}
 
void wxCustomButton::mouseDown(wxMouseEvent& event)
{
    pressedDown = true;
    paintNow();
}
void wxCustomButton::mouseReleased(wxMouseEvent& event)
{
    pressedDown = false;
    paintNow();
 
    wxMessageBox( wxT("You pressed a custom button") );
}
void wxCustomButton::mouseLeftWindow(wxMouseEvent& event)
{
    if (pressedDown)
    {
        pressedDown = false;
        paintNow();
    }
}
 

 

BEGIN_EVENT_TABLE(wxCustomButton, wxWindow)//mozda treba ovde wxWindow da bude drugi parametar
  //  EVT_MOTION(wxCustomButton::mouseMoved)
    EVT_LEFT_DOWN(wxCustomButton::mouseDown)
    EVT_LEFT_UP(wxCustomButton::mouseReleased)
  //  EVT_RIGHT_DOWN(wxCustomButton::rightClick)
    EVT_LEAVE_WINDOW(wxCustomButton::mouseLeftWindow)
  //  EVT_KEY_DOWN(wxCustomButton::keyPressed)
  //  EVT_KEY_UP(wxCustomButton::keyReleased)
   // EVT_MOUSEWHEEL(wxCustomButton::mouseWheelMoved)
    // catch paint events
    EVT_PAINT(wxCustomButton::paintEvent)
END_EVENT_TABLE()
 
        
        
        // currently unused events
/*void wxCustomButton::mouseMoved(wxMouseEvent& event) {}
void wxCustomButton::mouseWheelMoved(wxMouseEvent& event) {}
void wxCustomButton::rightClick(wxMouseEvent& event) {}
void wxCustomButton::keyPressed(wxKeyEvent& event) {}
void wxCustomButton::keyReleased(wxKeyEvent& event) {}
 */
// ----------------------------------------
// how-to-use example
 /*
class MyApp: public wxApp
{
 
    wxFrame *frame;
    wxCustomButton* m_btn_1;
    wxCustomButton* m_btn_2;
    wxCustomButton* m_btn_3;
 
public:
 
    bool OnInit()
    {
        // make sure to call this first
        wxInitAllImageHandlers();
 
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        frame = new wxFrame(NULL, wxID_ANY, wxT("Hello wxDC"), wxPoint(50,50), wxSize(800,600));
 
        // then simply create like this
        m_btn_1 = new wxCustomButton( frame, wxT("My Custom Button 11") );
        sizer->Add(m_btn_1, 0, wxALL, 5);
        m_btn_2 = new wxCustomButton( frame, wxT("Hello World!") );
        sizer->Add(m_btn_2, 0, wxALL, 5);
        m_btn_3 = new wxCustomButton( frame, wxT("Foo Bar") );
        sizer->Add(m_btn_3, 0, wxALL, 5);
 
        frame->SetSizer(sizer);
 
        frame->Show();
        return true;
    } 
 
};
 
IMPLEMENT_APP(MyApp)*/
//http://wiki.wxwidgets.org/Painting_your_custom_control