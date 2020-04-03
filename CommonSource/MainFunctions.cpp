

#include "Main.h"
/////////////////////////////////////////////////////////////////////
//
// MainFunction.cpp
//
// Sets up menus, binds them to handlers which dispatch them to functions
//
/////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <cstdlib>
#include <iostream>
#include <regex>
#include <stdarg.h>
#include <stdio.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/mediactrl.h>
#include <iterator>
#include <wchar.h>
#include <thread>
#include <vector>

#include "Constants.h"
#include "FileSet.h"
#include "FileSetManager.h"
#include "FileProperties.h"
#include "GridTable.h"
#include "GridTableTest.h"
#include "ImagePanel.h"
#include "ImagesGrid.h"
#include "Logger.h"
#include "MediaPreviewPlayer.h"
#include "ShellExecute.h"
#include "TestDialog.h"
#include "Tryout.h"
#include "Utilities.h"
#include "VideoUpdaterDialog.h"
#include "VolumeManager.h"
#include "VolumeSelectDialog.h"

// the menu resource IDs
enum MenuIDsT
{
	ID_Dummy = 0,

	// menus
	ID_MenuFile,
	ID_MenuFileSelect,
	ID_MenuFileDelete,
	ID_MenuFileExit,
	ID_MenuFileImport,
	ID_MenuView,
	ID_MenuViewPlay,
	ID_MenuViewTogglePreview,
	ID_MenuViewMoreImages,
	ID_MenuViewLessImages,
	ID_MenuTools,
	ID_MenuToolsVideoUpdater,
	ID_MenuTest,
	ID_MenuTestTest,
	ID_MenuTestTryout,
	ID_MenuTestToLogger,
	ID_MenuHelp,
	ID_MenuHelpAbout,

	// other functions, accelerators
	ID_PageUp,
	ID_PageDown,
	ID_CursorUp,
	ID_CursorDown,
	ID_CursorLeft,
	ID_CursorRight
};

// sets up the menu bar and menus
void Main::setupMenus()
{
	// file menu
	wxMenu* menuFile = new wxMenu;
	menuFile->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuFile); }, wxID_ANY);

	// file, delete
	menus_[ID_MenuFileSelect] =
		menuFile->Append(ID_MenuFileSelect, "Select...\tCtrl-S", "Select volumes");
	
	// file, delete
	menus_[ID_MenuFileDelete] =
		menuFile->Append(ID_MenuFileDelete, "Delete...\tCtrl-D", "Delete file");

	menuFile->AppendSeparator();

	// file, exit
	menus_[ID_MenuFileExit] = menuFile->Append(wxID_EXIT);

	// view, menu
	wxMenu* menuView = new wxMenu;
	menuView->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuView); }, wxID_ANY);

	// view, play
	menus_[ID_MenuViewPlay] =
		menuView->Append(ID_MenuViewPlay, "Play\tF1", "Play file");

	// view, toggle preview
	menus_[ID_MenuViewTogglePreview] =
		menuView->Append(ID_MenuViewTogglePreview, "Preview mode on/off", "Toggles preview mode", wxITEM_CHECK);

	// view, more/less images
	menus_[ID_MenuViewMoreImages] =
		menuView->Append(ID_MenuViewMoreImages, "Show more images", "Shows more images in the image browser");
	menus_[ID_MenuViewLessImages] =
		menuView->Append(ID_MenuViewLessImages, "Show less images", "Shows less images in the image browser");

	// tools menu
	wxMenu* menuTools = new wxMenu;
	menuTools->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuTools); }, wxID_ANY);

	// tools, video updater
	menus_[ID_MenuToolsVideoUpdater] =
		menuTools->Append(ID_MenuToolsVideoUpdater, "&Update video...\tCtrl-V", "Video update tools");

	// tools menu
	wxMenu* menuTest = new wxMenu;
	menuTools->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuTest); }, wxID_ANY);

	// test, test dialog
	menus_[ID_MenuTestTest] =
		menuTest->Append(ID_MenuTestTest, "Test\tCtrl-V", "Runs unit tests");

	// test, tryout
	menus_[ID_MenuTestTryout] =
		menuTest->Append(ID_MenuTestTryout, "TryOut...\tCtrl-T", "Hook for experimental code");

	// test, to logger
	menus_[ID_MenuTestToLogger] =
		menuTest->Append(ID_MenuTestToLogger, "To Logger\tCtrl-", "Hook for experimental code");

	// help menu
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuHelp); }, wxID_ANY);

	menus_[ID_MenuHelpAbout] = menuHelp->Append(wxID_ABOUT);

	// setup menu bar
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "File");
	menuBar->Append(menuView, "View");
	menuBar->Append(menuTools, "Tools");
	menuBar->Append(menuTest, "Test");
	menuBar->Append(menuHelp, "Help");
	SetMenuBar(menuBar);

	// bind the menu selected event to the dispatch function
	Bind(wxEVT_MENU, &Main::menuSelectedDispatch, this, wxID_ANY);

	// accelerators
	std::vector<wxAcceleratorEntry> keys;
	keys.emplace_back(wxACCEL_NORMAL, WXK_RETURN, ID_MenuViewPlay);
	keys.emplace_back(wxACCEL_NORMAL, WXK_PAGEUP, ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'Q', ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'W', ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'E', ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'R', ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, WXK_PAGEDOWN, ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'Z', ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'X', ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'C', ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'V', ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, WXK_UP, ID_CursorUp);
	keys.emplace_back(wxACCEL_NORMAL, WXK_DOWN, ID_CursorDown);
	keys.emplace_back(wxACCEL_NORMAL, WXK_LEFT, ID_CursorLeft);
	keys.emplace_back(wxACCEL_NORMAL, WXK_RIGHT, ID_CursorRight);

	wxAcceleratorTable accel(keys.size(), &keys[0]);
	SetAcceleratorTable(accel);
}

// called as the menu is about to be  displayed, enable and check status etc. can be updated here
void Main::menuConfigure(wxMenuEvent& event, int menuId)
{
	// is there a row selected
	bool isSelected = false;
	FileSetT fs;
	int r = grid_->getSelectedRow();
	if (r != -1)
	{
		isSelected = true;
		fs = FileSetManager::getFileSet(r);
	}
	
	switch (menuId)
	{
	case ID_MenuFile:
		menus_[ID_MenuFileDelete]->Enable(isSelected);
		break;
	case ID_MenuView:
		menus_[ID_MenuViewPlay]->Enable(isSelected);
		menus_[ID_MenuViewTogglePreview]->Check(Constants::previewMode);
		if (Constants::previewMode)
			menus_[ID_MenuViewTogglePreview]->SetItemLabel("Preview mode off");
		else
			menus_[ID_MenuViewTogglePreview]->SetItemLabel("Preview mode on");
		menus_[ID_MenuViewMoreImages]->Enable(Constants::imageBrowserSize < Constants::imageBrowserSizeMax);
		menus_[ID_MenuViewLessImages]->Enable(Constants::imageBrowserSize > Constants::imageBrowserSizeMin);
		break;
	case ID_MenuTools:
		menus_[ID_MenuToolsVideoUpdater]->Enable(fs.get() != nullptr && fs->hasVideo());
		break;
	case ID_MenuTest:
		break;
	case ID_MenuHelp:
		break;
	}
}

// called when a menu option has been selected
void Main::menuSelectedDispatch(wxCommandEvent& event)
{
	FileSetT fs;
	int row = grid_->getSelectedRow();
	if (row != -1)
		fs = FileSetManager::getFileSet(row);

	switch (event.GetId())
	{
	case ID_MenuFileSelect:
		VolumeSelectDialog::Run(this);
		break;
	case ID_MenuFileDelete:
		deleteFile(event, row, *fs.get());
		break;
	case wxID_EXIT:
		Close();
		break;
	case ID_MenuViewPlay:
		play(event, row, *fs.get());
		break;
	case ID_MenuViewTogglePreview:
		togglePreviewMode();
		break;
	case ID_MenuViewMoreImages:
		updateNoOfImages(+1);
		break;
	case ID_MenuViewLessImages:
		updateNoOfImages(-1);
		break;
	case ID_MenuToolsVideoUpdater:
		VideoUpdaterDialog::Run(this, fs);
		break;
	case ID_MenuTestTest:
		unitTests();
		break;
	case ID_MenuTestTryout:
		TryOut::tryout(fs);
		break;
	case ID_MenuTestToLogger:
		toLogger();
		break;
	case ID_MenuHelpAbout:
		break;
	case ID_PageUp:
		pageUp();
		break;
	case ID_PageDown:
		pageDown();
		break;
	case ID_CursorUp:
		cursorUp();
		break;
	case ID_CursorDown:
		cursorDown();
		break;
	case ID_CursorLeft:
		cursorLeft();
		break;
	case ID_CursorRight:
		cursorRight();
		break;
	}
}

// gets the popup menu used by the grid and images browser,
// N.B. the popup menu must be deleted by the caller
wxMenu *Main::getPopupMenu(const int item)
{
	FileSetT fs;
	if (item != -1) fs = FileSetManager::getFileSet(item);

	wxMenu *menu = new wxMenu();
	wxMenuItem *menuItem;
	menuItem = menu->Append(ID_MenuFileDelete, "Delete ...");
	menuItem->Enable(fs.get() != nullptr);

	menuItem = menu->Append(ID_MenuViewPlay, "Play");
	menuItem->Enable(fs.get() != nullptr);

	menuItem = menu->Append(ID_MenuToolsVideoUpdater, "Update video ...");
	menuItem->Enable(fs.get() != nullptr && fs->hasVideo());

	menuItem = menu->Append(ID_MenuViewTogglePreview, "Preview mode", wxEmptyString, wxITEM_CHECK);
	menuItem->Check(Constants::previewMode);
	if (Constants::previewMode)
		menuItem->SetItemLabel("Preview mode off");
	else
		menuItem->SetItemLabel("Preview mode on");

	menu->Bind(wxEVT_MENU, &Main::menuSelectedDispatch, this, wxID_ANY);
	return menu;
}

//--------------------------------------------------------------------------
// the menu option handlers 
//--------------------------------------------------------------------------

void Main::deleteFile(wxCommandEvent& event, const int row, FileSet& fileset)
{
	Logger::info("Delete file %s, %d", fileset.getId().c_str(), row);
}

void Main::play(wxCommandEvent& event, const int row, FileSet &fileset)
{
	Logger::info("Play %s, %d", fileset.getId().c_str(), row);

	fileset.properties().setDateTimeNow("lasttime");
	fileset.properties().incCount("times");
	refresh(fileset);
}

void Main::pageUp()
{
	if (images_->hasFocus())
		images_->pageUp();
	else
	{
		grid_->MovePageUp();
		int r = grid_->GetGridCursorRow();
		grid_->SelectRow(r);
		images_->setSelected(r);
	}
}

void Main::pageDown()
{
	if (images_->hasFocus())
		images_->pageDown();
	else
	{
		grid_->MovePageDown();
		int r = grid_->GetGridCursorRow();
		grid_->SelectRow(r);
		images_->setSelected(r);
	}
}

void Main::cursorUp()
{
	if (images_->hasFocus())
		images_->cursorUp();
	else
	{
		grid_->MoveCursorUp(false);
		int r = grid_->GetGridCursorRow();
		grid_->SelectRow(r);
		images_->setSelected(r);
	}
}

void Main::cursorDown()
{
	if (images_->hasFocus())
		images_->cursorDown();
	else
	{
		grid_->MoveCursorDown(false);
		int r = grid_->GetGridCursorRow();
		grid_->SelectRow(r);
		images_->setSelected(r);
	}
}

void Main::cursorLeft()
{
	if (images_->hasFocus())
		images_->cursorLeft();
	else
	{
		grid_->MoveCursorUp(false);
		int r = grid_->GetGridCursorRow();
		grid_->SelectRow(r);
		images_->setSelected(r);
	}
}

void Main::cursorRight()
{
	if (images_->hasFocus())
		images_->cursorRight();
	else
	{
		grid_->MoveCursorDown(false);
		int r = grid_->GetGridCursorRow();
		grid_->SelectRow(r);
		images_->setSelected(r);
	}
}

void Main::toLogger()
{
	VolumeManager::toLogger();
	FileSetManager::toLogger();
}

void Main::togglePreviewMode()
{
	if (Constants::previewMode) images_->stopPreview();
	Constants::previewMode = !Constants::previewMode;
}

void Main::unitTests()
{
	bool result = true;

	result &= Duration::test();
	result &= FileProperties::test();

	if (result) Logger::info("All unit tests passed");
}

void Main::updateNoOfImages(const int delta)
{
	int s = Constants::imageBrowserSize + delta;
	if (s <= Constants::imageBrowserSizeMax && 
		s >= Constants::imageBrowserSizeMin)
	{
		int t = images_->getTop();
		images_->uninitialise();
		Constants::imageBrowserSize = s;
		images_->initialise();
		images_->displayAt(t);
	}
}






