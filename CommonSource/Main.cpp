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
#include "ImagePanel.h"


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

	// the main frame sizer
	wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);

	// create the vertical splitter and add to sizer
	wxSplitterWindow* splitterVertical = new wxSplitterWindow(this, wxID_ANY);
	splitterVertical->SetSashGravity(0.5);
	splitterVertical->SetMinimumPaneSize(20); // Smallest size the
	sizerMain->Add(splitterVertical, 1, wxEXPAND, 0);

	// create lh horizontal splitter
	wxPanel* pnlLh = new wxPanel(splitterVertical, wxID_ANY);

	wxSplitterWindow* splitterHorizontal = new wxSplitterWindow(pnlLh, wxID_ANY);
	splitterHorizontal->SetSashGravity(0.5);
	splitterHorizontal->SetMinimumPaneSize(20); // Smallest size the

	wxBoxSizer* sizerLh = new wxBoxSizer(wxVERTICAL);
	sizerLh->Add(splitterHorizontal, 1, wxEXPAND, 0);
	pnlLh->SetSizer(sizerLh);

	// create a top panel
	wxPanel* pnlTop = new wxPanel(splitterHorizontal, wxID_ANY);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
//	sizerTop->Add(pnlTop, 1, wxEXPAND, 0);
	pnlTop->SetSizer(sizerTop);

	// create a bottom panel with logger
	wxPanel* pnlBot = new wxPanel(splitterHorizontal, wxID_ANY);
	wxBoxSizer* sizerBot = new wxBoxSizer(wxVERTICAL);

	Logger* lb1 = new Logger(pnlBot, wxID_ANY);
	Logger::setLevel(Logger::Info);
	Logger::enableTime(true);
	Logger::enableLineCount(true);
	Logger::enableIdeOutput(true);
	lb1->SetMinSize(wxSize(800, 500));

	sizerBot->Add(lb1, 1, wxEXPAND, 0);
	pnlBot->SetSizer(sizerBot);

	// added panels to horizontal splitter
	splitterHorizontal->SplitHorizontally(pnlTop, pnlBot);

	// create the RH pane, which is an image viewer
	wxPanel* pnlRh = new wxPanel(splitterVertical, wxID_ANY);
	ImagePanel* pnlIm = new ImagePanel(
			pnlRh,
			LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
			wxBITMAP_TYPE_JPEG);

	wxBoxSizer* sizerRh = new wxBoxSizer(wxVERTICAL);
	sizerRh->Add(pnlIm, 1, wxEXPAND, 0);
	pnlRh->SetSizer(sizerRh);

	// added panels to horizontal splitter
	splitterVertical->SplitVertically(pnlLh, pnlRh);

	this->SetSizer(sizerMain);
	sizerMain->SetSizeHints(this);

	this->SetClientSize(wxSize(1200, 800));

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

	TryOut::AsyncShell(GetEventHandler());
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
	Logger::info(L"Shell execute notify via GUI thread 1\n%ls", event.getResult().toString().c_str());
	event.Skip();
}

void MyFrame::OnShellExecute2(wxShellExecuteResult& event)
{
	Logger::info(L"Shell execute notify via GUI thread 2\n%ls", event.getResult().toString().c_str());
	event.Skip();
}

void MyFrame::OnShellExecuteAny(wxShellExecuteResult& event)
{
	Logger::info(L"Shell execute notify via GUI thread any\n%ls", event.getResult().toString().c_str());
	event.Skip();
}



