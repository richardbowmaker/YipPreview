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
#include <wx/cmdline.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#ifdef LINUX_BUILD
	#include <X11/Xlib.h>
#endif

#include "Constants.h"
#include "FileSet.h"
#include "FileSetManager.h"
#include "GridTable.h"
#include "GridTableTest.h"
#include "ImagePanel.h"
#include "ImagesBrowser.h"
#include "ImagesGrid.h"
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
	
	// when running in sudo mode the user id must be supplied via the command
	// line so that privileges can be lowered immediately and only raised when needed
	if (SudoMode::inSudoMode())
	{
		if (argc == 3)
		{
			std::string s{this->argv[1]};
			if (s.compare("-uid") == 0)
			{
				int uid = atoi(this->argv[2]);
				SudoMode::initialise(uid);
			}
		}
		else
		{
			std::cout << "Invalid argument, usage is YipPreview -uid <user id>" << std::endl;
			return false;
		}
	}
	else
	{
		if (argc > 1)
		{
			std::cout << "arguments only valid when running in SUDO mode" << std::endl;
			return false;
		}
	}

	MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    frame->Show( true );
    return true;
}

MyFrame *MyFrame::this_;

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
	wxFrame(NULL, wxID_ANY, title, pos, size),
	grid_(nullptr),
	table_(nullptr),
	images_(nullptr)
{
	// keep static pointer to main frame
	this_ = this;

	SudoMode::initialise(1000);

	Constants::initialise();
	FileSetManager::initialise();

	VolumeManager::initialise();
	VolumeT vol1 = std::make_shared<Volume>(FU::pathToLocal(LR"(/YipPreview/Tryout)"), false);
	VolumeT vol2 = std::make_shared<Volume>(FU::pathToLocal(LR"(/YipPreview/Encrypted/TestVol1.hc)"), true);
	VolumeManager::add(vol1);
	VolumeManager::add(vol2);
//	FileSetManager::addFiles(vol);

	setupMenus();
	CreateStatusBar();

	Bind(wxEVT_CLOSE_WINDOW, &MyFrame::onClose, this, wxID_ANY);

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
//	initialiseGrid(pnlTop);
//	populateGrid();
	grid_ = new ImagesGrid(pnlTop, wxID_ANY);
	table_ = new GridTable();
	grid_->initialise(this);
	grid_->populate();

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
	images_->displayAt(0);

	// added panels to horizontal splitter
	splitterVertical->SplitVertically(pnlLh, pnlRh);

	SetClientSize(wxSize(1500, 1000));

	if (SudoMode::inSudoMode())
		Logger::error(L"Application is currently running at SUDO level");

	Logger::info(L"euid %d", geteuid());

	Logger::error(L"test");
}

//--------------------------------------------------------------
// public methods
//--------------------------------------------------------------

MyFrame &MyFrame::get()
{
	return *this_;
}

void MyFrame::refresh(const FileSet &fileset)
{
	// better solution required
	grid_->refresh();
}

void MyFrame::refresh()
{
	grid_->refresh();
}

void MyFrame::populateGui()
{
	grid_->populate();
	images_->uninitialise();
	images_->initialise();
	images_->displayAt(0);
}

//--------------------------------------------------------------
// ImagesGridServer interface
//--------------------------------------------------------------

wxMenu *MyFrame::gridGetPopupMenu(const int item)
{
	return getPopupMenu(item);
}

wxGridTableBase *MyFrame::gridGetTable()
{
	return table_;
}

void MyFrame::gridSetSelected(const int selected)
{
	images_->setSelected(selected);
}

void MyFrame::gridGotFocus()
{
	images_->setFocus(false);
}

//--------------------------------------------------------------
// ImagesBrowserData interface
//--------------------------------------------------------------

int MyFrame::browserGetNoOfRows()
{
	return Constants::imageBrowserSize;
}

int MyFrame::browserGetNoOfCols()
{
	return Constants::imageBrowserSize;
}

int MyFrame::browserGetNoOfImages()
{
	return FileSetManager::getNoOfFileSets();
}

int MyFrame::browserGetSelected()
{
	return grid_->getSelectedRow();
}

void MyFrame::browserSetSelected(const int selected)
{
	grid_->SelectRow(selected);
}

std::wstring MyFrame::browserGetImage(const int n)
{
	return FileSetManager::getFileSet(n)->getImage();
}

std::wstring MyFrame::browserGetVideo(const int n)
{
	return FileSetManager::getFileSet(n)->getVideo();
}

wxMenu *MyFrame::browserGetPopupMenu(const int item)
{
	return getPopupMenu(item);
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
	images_->setFocus(false);
}

void MyFrame::onClose(wxCloseEvent& event)
{
	int unmount{wxNO};
	if (VolumeManager::hasMountedVolumes())
	{
		unmount = Utilities::messageBox(L"Do you want to unmount all volumes ?",
				L"Close", wxYES_NO | wxCANCEL , this);
		if (unmount == wxCANCEL)
		{
			event.Veto(true);
			return;
		}
	}

	images_->uninitialise();
	grid_->uninitialise();
	VolumeManager::writeProperties();
	FileSetManager::uninitialise();
	if (unmount == wxYES) VolumeManager::unmountVolumes();
	VolumeManager::uninitialise();

	if (table_ != nullptr)
		delete table_;

	Destroy();  // you may also do:  event.Skip();
				// since the default event handler does call Destroy(), too
}

