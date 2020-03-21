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
#include <wx/splitter.h>
#include <wx/sizer.h>
#ifdef LINUX_BUILD
	#include <X11/Xlib.h>
#endif

#include "Constants.h"
#include "FileSet.h"
#include "FileSetManager.h"
#include "GridEx.h"
#include "GridTable.h"
#include "GridTableTest.h"
#include "ImagePanel.h"
#include "ImagesBrowser.h"
#include "Logger.h"
#include "MediaPreviewPlayer.h"
#include "ShellExecute.h"
#include "Tryout.h"
#include "Utilities.h"
#include "VolumeManager.h"

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

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
	wxFrame(NULL, wxID_ANY, title, pos, size),
	player_(nullptr),
	grid_(nullptr),
	table_(nullptr),
	images_(nullptr),
	browserRows_(2),
	browserCols_(2)
{
	// keep static pointer to main frame
	this_ = this;

	Constants::initialise();
	FileSetManager::initialise();

	setupMenus();
	CreateStatusBar();

	Bind(wxEVT_CLOSE_WINDOW, &MyFrame::OnClose, this, wxID_ANY);

	// create vertical splitter, images in rh pane, grid and logger in lh
	wxSplitterWindow* splitterVertical = new wxSplitterWindow(this, wxID_ANY);
	splitterVertical->SetSashGravity(0.5);
	splitterVertical->SetMinimumPaneSize(20); // Smallest size the

	// create main frame sizer and add vertical splitter to it
	wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizerMain);
	sizerMain->Add(splitterVertical, 1, wxEXPAND, 0);

	// create lh panel and its sizer
	wxPanel* pnlLh = new wxPanel(splitterVertical, wxID_ANY);
	wxBoxSizer* sizerLh = new wxBoxSizer(wxVERTICAL);
	pnlLh->SetSizer(sizerLh);

	// create rh panel and its sizer
	wxPanel* pnlRh = new wxPanel(splitterVertical, wxID_ANY);
	wxBoxSizer* sizerRh = new wxBoxSizer(wxVERTICAL);
	pnlRh->SetSizer(sizerRh);

	// the lh pane has a horizontal splitter, grid top and logger bottom
	wxSplitterWindow* splitterHorizontal = new wxSplitterWindow(pnlLh, wxID_ANY);
	splitterHorizontal->SetSashGravity(0.5);
	splitterHorizontal->SetMinimumPaneSize(20); // Smallest size the
	sizerLh->Add(splitterHorizontal, 1, wxEXPAND, 0);

	// create top left panel and its sizer
	wxPanel* pnlTop = new wxPanel(splitterHorizontal, wxID_ANY);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	pnlTop->SetSizer(sizerTop);

	// create grid and add it to top panel
	initialiseGrid(pnlTop);
	populateGrid();
	sizerTop->Add(grid_, 1, wxEXPAND, 0);

	// create a bottom panel 
	wxPanel* pnlBot = new wxPanel(splitterHorizontal, wxID_ANY);
	wxBoxSizer* sizerBot = new wxBoxSizer(wxVERTICAL);
	pnlBot->SetSizer(sizerBot);

	// create logger and add it to bottom panel
	Logger* logger = setupLogger(pnlBot);
	sizerBot->Add(logger, 1, wxEXPAND, 0);

	// added panels to horizontal splitter
	splitterHorizontal->SplitHorizontally(pnlTop, pnlBot);

	images_ = new ImagesBrowser(pnlRh, this);
	sizerRh->Add(images_, 1, wxEXPAND, 0);
	images_->initialise();
	images_->setTop(0);

	// added panels to horizontal splitter
	splitterVertical->SplitVertically(pnlLh, pnlRh);

	SetClientSize(wxSize(1000, 600));
	Bind(wxEVT_SHELL_EXECUTE_RESULT, &MyFrame::OnShellExecute, this, wxID_ANY);

}

MyFrame &MyFrame::getMainFrame()
{
	return *this_;
}

//--------------------------------------------------------------
// grid functions
//--------------------------------------------------------------

void MyFrame::initialiseGrid(wxPanel* panel)
{
	grid_ = new GridEx(panel, wxID_ANY);
	table_ = new GridTable();
	grid_->SetTable(table_);
	grid_->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid_->HideRowLabels();

	grid_->Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &MyFrame::gridEventDispatch, this, wxID_ANY);
	grid_->Bind(wxEVT_GRID_SELECT_CELL, &MyFrame::gridEventDispatch, this, wxID_ANY);
	grid_->Bind(wxEVT_SET_FOCUS, &MyFrame::onFocus, this, wxID_ANY);

	grid_->SetColLabelSize(grid_->GetDefaultRowSize());
}

void MyFrame::uninitialiseGrid()
{
	grid_->SetTable(nullptr);
	delete table_;
}

void MyFrame::populateGrid()
{
	volume_ = std::make_shared<Volume>(FU::pathToLocal(LR"(/YipPreview/Tryout)"));
	VolumeManager::add(volume_);
	FileSetManager::addFiles(volume_);
	grid_->SetTable(table_);
	grid_->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid_->HideRowLabels();
	grid_->EnableEditing(false);
	grid_->SetColSize(0, 220);
	grid_->SetColSize(1, 50);
	grid_->SetColSize(2, 100);
	grid_->SetColSize(3, 150);
	grid_->SetColSize(4, 50);
}

void MyFrame::gridEventDispatch(wxGridEvent &event)
{
	int id = event.GetEventType();

	if (id == wxEVT_GRID_CELL_RIGHT_CLICK)
	{
		int row = grid_->YToRow(event.GetPosition().y - grid_->GetColLabelSize()) +
			grid_->getTopRow();

		if (row >= 0 && row < grid_->GetNumberRows())
		{
			grid_->SelectRow(row);
			images_->setSelected(row);
			grid_->PopupMenu(getGridPopupMenu());
		}
	}
	if (id == wxEVT_GRID_SELECT_CELL)
	{
		images_->setSelected(grid_->getSelectedRow());
	}
}

//--------------------------------------------------------------
// ImagesBrowserData interface
//--------------------------------------------------------------

int MyFrame::getNoOfRows()
{
	return browserRows_;
}

int MyFrame::getNoOfCols()
{
	return browserCols_;
}

int MyFrame::getNoOfImages()
{
	return FileSetManager::getNoOfFileSets();
}

int MyFrame::getSelected()
{
	return grid_->getSelectedRow();
}

void MyFrame::setSelected(const int selected)
{
	grid_->SelectRow(selected);
}

std::wstring MyFrame::getImage(const int n)
{
	return FileSetManager::getFileSet(n)->getImage();
}

std::wstring MyFrame::getVideo(const int n)
{
	return FileSetManager::getFileSet(n)->getVideo();
}

//--------------------------------------------------------------
//
//--------------------------------------------------------------

Logger *MyFrame::setupLogger(wxPanel *panel)
{
	Logger* logger = new Logger(panel, wxID_ANY);
	logger->SetMinSize(wxSize(800, 500));
	Logger::setLevel(Logger::Info);
	Logger::enableTime(true);
	Logger::enableLineCount(true);
	Logger::enableIdeOutput(true);
	logger->Bind(wxEVT_SET_FOCUS, &MyFrame::onFocus, this, wxID_ANY);
	return logger;
}

void MyFrame::onFocus(wxFocusEvent& event)
{
	Logger::info(L"Grid got focus");
	images_->setFocus(false);
}
void MyFrame::OnClose(wxCloseEvent& event)
{
	images_->uninitialise();
	VolumeManager::writeProperties();
	VolumeManager::uninitialise();
	FileSetManager::uninitialise();
	uninitialiseGrid();

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




