/*
 * Main.h
 *
 *  Created on: 19 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_MAIN_H_
#define COMMON_MAIN_H_


#include <iostream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "Events.h"

class MyApp: public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame: public wxFrame
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    //DECLARE_EVENT_TABLE()

private:
    void OnHello(wxCommandEvent& event);
    void OnTryOut(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnThread(wxCommandEvent& event);
	void OnProcessCustom(wxCommandEvent& event);
	void OnLogger(wxLoggerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif /* COMMON_MAIN_H_ */
