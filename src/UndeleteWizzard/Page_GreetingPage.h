/* 
 * File:   GreetingPage.h
 * Author: MilicM
 *
 * Created on November 8, 2012, 8:38 AM
 */

#ifndef PAGE_GREETINGPAGE_H
#define	PAGE_GREETINGPAGE_H



#endif	/* PAGE_GREETINGPAGE_H */

class wxGreetingPage : public wxWizardPageSimple {
public:
    wxGreetingPage(wxWizard *parent);
    void OnPageShown(wxWizardEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

