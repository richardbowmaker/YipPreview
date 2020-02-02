//============================================================================
// Name        : Main.cpp
//
//
//
//
//============================================================================

// For compilers that support precompilation, includes "wx/wx.h".

#include "Main.h"


#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <cstdlib>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <iterator>
#include <wchar.h>

#include "Events.h"
#include "Logger.h"
#include "Tryout.h"
#include "Utilities.h"
#include "ShellExecute.h"


enum
{
    ID_Hello = 1,
    ID_TryOut = 2
};

// it may also be convenient to define an event table macro for this event type
#define EVT_MY_CUSTOM_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_MY_CUSTOM_COMMAND, id, wxID_ANY, \
        wxCommandEventHandler(fn), \
        (wxObject *) NULL \
    ),

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Hello,   MyFrame::OnHello)
    EVT_MENU(ID_TryOut, MyFrame::OnTryOut)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
	EVT_MY_CUSTOM_COMMAND(wxID_ANY, MyFrame::OnProcessCustom)
	EVT_SHELL_EXECUTE_RESULT_COMMAND(1, MyFrame::OnShellExecute1)
	EVT_SHELL_EXECUTE_RESULT_COMMAND(2, MyFrame::OnShellExecute2)
	EVT_SHELL_EXECUTE_RESULT_COMMAND(wxID_ANY, MyFrame::OnShellExecuteAny)
//	EVT_LOGGER_EVENT_COMMAND(wxID_ANY, MyFrame::OnLogger)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    frame->Show( true );
    return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->Append(ID_TryOut, "&TryOut...\tCtrl-T",
        "Hook for experimental code");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuHelp, "&Help" );
    SetMenuBar( menuBar );
    CreateStatusBar();


	wxBoxSizer* sizermain = new wxBoxSizer(wxVERTICAL);
	wxSplitterWindow* splittermain = new wxSplitterWindow(this, wxID_ANY);
	splittermain->SetSashGravity(0.5);
	splittermain->SetMinimumPaneSize(20); // Smalest size the
	sizermain->Add(splittermain, 1, wxEXPAND, 0);

	wxPanel* pnl1 = new wxPanel(splittermain, wxID_ANY);

	wxBoxSizer* txt1sizer = new wxBoxSizer(wxVERTICAL);
	Logger* lb1 = new Logger(pnl1, wxID_ANY);
	Logger::setLevel(Logger::Info);
	Logger::enableLineCount(true);
	lb1->SetMinSize(wxSize(800, 500));
	txt1sizer->Add(lb1, 1, wxEXPAND, 0);
	pnl1->SetSizer(txt1sizer);

	wxPanel* pnl2 = new wxPanel(splittermain, wxID_ANY);

	wxBoxSizer* txt2sizer = new wxBoxSizer(wxVERTICAL);
	wxListBox* lb2 = new wxListBox(pnl2, wxID_ANY);
	txt2sizer->Add(lb2, 1, wxEXPAND, 0);
	pnl2->SetSizer(txt2sizer);

	splittermain->SplitVertically(pnl1, pnl2);

	this->SetSizer(sizermain);
	sizermain->SetSizeHints(this);

	Logger::info(L"PID %d", (int)Utilities::getProcessId());


	FILE* fp = std::fopen("test.txt", "r");
#ifdef WINDOWS_BUILD
	int err = ::GetLastError();
#elif LINUX_BUILD
	int err = errno;
#endif
	Logger::systemError(err, L"my error message %d", 67);

#ifdef WINDOWS_BUILD
    SetStatusText( "Welcome to wxWidgets for Windows!" );
#elif LINUX_BUILD
    SetStatusText("Welcome to wxWidgets for Linux!");
#endif

}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox( "This is a wxWidgets' Hello world sample",
                  "About Hello World", wxOK | wxICON_INFORMATION );
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnTryOut(wxCommandEvent& event)
{
	TryOut::ExecIt();

//	TryOut::AsyncShell(GetEventHandler());
//	TryOut::WorkerThread();
//	TryOut::ExecIt();

//	ShellExecuteResult result;
//    ShellExecute::shellSync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", result);
//    //ShellExecute::shellSync(L"/bin/nonexistentprogram", result);
//    //ShellExecute::shellSync(L"/bin/notepadqq", result, 5000);
//    std::wstring ws = result.toString();
//    Logger::info(ws.c_str());
}

void MyFrame::OnThread(wxCommandEvent& event)
{
}

void MyFrame::OnProcessCustom(wxCommandEvent& event)
{
	Logger::info(event.GetString());
}

void MyFrame::OnLogger(wxLoggerEvent& event)
{
	Logger::info(L"level %d: %ls", event.getLevel(), event.GetString().wc_str());
}

void MyFrame::OnShellExecute1(wxShellExecuteResult& event)
{
	Logger::info(L"Shell execute notify via GUI thread 1");
	Logger::info(event.getResult().toString().c_str());
	event.Skip();
}

void MyFrame::OnShellExecute2(wxShellExecuteResult& event)
{
	Logger::info(L"Shell execute notify via GUI thread 2");
	Logger::info(event.getResult().toString().c_str());
	event.Skip();
}

void MyFrame::OnShellExecuteAny(wxShellExecuteResult& event)
{
	Logger::info(L"Shell execute notify via GUI thread any");
	Logger::info(event.getResult().toString().c_str());
	event.Skip();
}



