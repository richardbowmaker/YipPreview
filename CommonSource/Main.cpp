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
#include <functional>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/mediactrl.h>
#include <iterator>
#include <wchar.h>
#include <thread>
#ifdef LINUX_BUILD
	#include <X11/Xlib.h>
#endif

#include "FileSet.h"
#include "Logger.h"
#include "Tryout.h"
#include "Utilities.h"
#include "ShellExecute.h"
#include "ImagePanel.h"
#include "MediaPreviewPlayer.h"


wxIMPLEMENT_APP(MyApp);

MyApp::MyApp()
{
#ifdef LINUX_BUILD
	// required for the media player
	XInitThreads();
#endif
}

bool MyApp::OnInit()
{
	wxInitAllImageHandlers();
	
	MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    frame->Show( true );
    return true;
}

MyFrame *MyFrame::this_;

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
	// keep static pointer to main frame
	this_ = this;

	setupMenus();

	CreateStatusBar();

	Bind(wxEVT_CLOSE_WINDOW, &MyFrame::OnClose, this, wxID_ANY);


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

	// create a top panel, with a media player
//	wxPanel* pnlTop = new wxPanel(splitterHorizontal, wxID_ANY);
//	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
//	player_ = new wxMediaCtrl();
//	player_->Create(this, wxID_ANY);
//	player_->ShowPlayerControls(wxMEDIACTRLPLAYERCONTROLS_DEFAULT);
//	Bind(wxEVT_MEDIA_LOADED, &MyFrame::OnMediaPlay, this, wxID_ANY);
//
//	sizerTop->Add(player_, 1, wxEXPAND, 0);

	player_ = new MediaPreviewPlayer(splitterHorizontal);
//	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
//	sizerTop->Add(player, 1, wxEXPAND, 0);
//	pnlTop->SetSizer(sizerTop);


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
//	splitterHorizontal->SplitHorizontally(pnlTop, pnlBot);
	splitterHorizontal->SplitHorizontally(player_, pnlBot);

	// create the RH pane, which is an image viewer
	wxPanel* pnlRh = new wxPanel(splitterVertical, wxID_ANY);
	ImagePanel* pnlIm = new ImagePanel(
			pnlRh,
#ifdef WINDOWS_BUILD
		LR"(D:\Projects\WxWidgets\YipPreview\Tryout\a12.jpg)",
#elif LINUX_BUILD
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a04.jpg)",
#endif
			wxBITMAP_TYPE_JPEG);

	wxBoxSizer* sizerRh = new wxBoxSizer(wxVERTICAL);
	sizerRh->Add(pnlIm, 1, wxEXPAND, 0);
	pnlRh->SetSizer(sizerRh);

	// added panels to horizontal splitter
	splitterVertical->SplitVertically(pnlLh, pnlRh);

	this->SetSizer(sizerMain);
	sizerMain->SetSizeHints(this);

	this->SetClientSize(wxSize(1000, 600));

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

//	Bind(wxEVT_SHELL_EXECUTE_RESULT, [this](wxShellExecuteResult& e) -> void {OnShellExecute1(e); }, wxID_ANY);
	Bind(wxEVT_SHELL_EXECUTE_RESULT, [this](wxShellExecuteEvent& e) {OnShellExecute1(e);}, wxID_ANY);
}

MyFrame & MyFrame::getMainFrame()
{
	return *this_;
}

void MyFrame::OnClose(wxCloseEvent& event)
{
	player_->stopPreview();
	Destroy();  // you may also do:  event.Skip();
				// since the default event handler does call Destroy(), too
}

FileSet dummyFS;

FileSet& MyFrame::getSelectedFileSet() const
{
	return dummyFS;
}


void MyFrame::OnThread(wxCommandEvent& event)
{
}

void MyFrame::OnProcessCustom(wxCommandEvent& event)
{
	Logger::info(event.GetString());
}

void MyFrame::OnShellExecute1(wxShellExecuteEvent& event)
{
	Logger::info(L"Shell execute notify via GUI thread 1\n%ls", event.getResult().toString().c_str());
	event.Skip();
}

void MyFrame::OnShellExecute2(wxShellExecuteEvent& event)
{
	Logger::info(L"Shell execute notify via GUI thread 2\n%ls", event.getResult().toString().c_str());
	event.Skip();
}

void MyFrame::OnShellExecuteAny(wxShellExecuteEvent& event)
{
	Logger::info(L"Shell execute notify via GUI thread any\n%ls", event.getResult().toString().c_str());
	event.Skip();
}

void MyFrame::OnMediaPlay(wxMediaEvent& event)
{
//	int n1 = 0;
//	Logger::info(L"Before play");
//	player_->Play();
//	Logger::info(L"After play");
}



