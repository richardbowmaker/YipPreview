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
#include <wx/grid.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#ifdef LINUX_BUILD
	#include <X11/Xlib.h>
#endif

#include "FileSet.h"
#include "FileSetManager.h"
#include "Logger.h"
#include "Tryout.h"
#include "Utilities.h"
#include "ShellExecute.h"
#include "ImagePanel.h"
#include "MediaPreviewPlayer.h"
#include "GridTable.h"

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

	// add grid to top left panel
	wxPanel* pnlTop = new wxPanel(splitterHorizontal, wxID_ANY);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	setupGrid();
	sizerTop->Add(grid_, 1, wxEXPAND, 0);
	pnlTop->SetSizer(sizerTop);

	// create a bottom left panel with logger
	wxPanel* pnlBot = new wxPanel(splitterHorizontal, wxID_ANY);
	wxBoxSizer* sizerBot = new wxBoxSizer(wxVERTICAL);
	Logger* logger = setupLogger(pnlBot);
	sizerBot->Add(logger, 1, wxEXPAND, 0);
	pnlBot->SetSizer(sizerBot);

	// added panels to horizontal splitter
	splitterHorizontal->SplitHorizontally(pnlTop, pnlBot);

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

	Bind(wxEVT_SHELL_EXECUTE_RESULT, [this](wxShellExecuteEvent& e) {OnShellExecute(e);}, wxID_ANY);

	populateGrid();
}

MyFrame &MyFrame::getMainFrame()
{
	return *this_;
}

void MyFrame::setupGrid()
{
	grid_ = new wxGrid(this, wxID_ANY);
	table_ = new GridTable();
	table_->initialise();
	grid_->SetTable(table_);
	grid_->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid_->HideRowLabels();
}

void MyFrame::populateGrid()
{
	FileSetManager::addFiles(FU::pathToLocal(LR"(D:\Projects\WxWidgets\YipPreview\Tryout)").c_str());
	refreshGrid();
}

void MyFrame::refreshGrid()
{
	wxGridTableMessage push(table_,
		wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
		FileSetManager::getNoOfFileSets());
	grid_->ProcessTableMessage(push);
	grid_->ForceRefresh();
}

Logger *MyFrame::setupLogger(wxPanel *panel)
{
	Logger* logger = new Logger(panel, wxID_ANY);
	logger->SetMinSize(wxSize(800, 500));
	Logger::setLevel(Logger::Info);
	Logger::enableTime(true);
	Logger::enableLineCount(true);
	Logger::enableIdeOutput(true);
	return logger;
}

void MyFrame::OnClose(wxCloseEvent& event)
{
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

void MyFrame::OnShellExecute(wxShellExecuteEvent& event)
{
	Logger::info(L"Shell execute notify via GUI thread\n%ls", event.getResult().toString().c_str());
	event.Skip();
}




