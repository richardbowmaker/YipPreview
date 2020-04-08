
/////////////////////////////////////////////////////////////////////
//
// MainFunction.cpp
//
// Sets up menus, binds them to handlers which dispatch them to functions
//
/////////////////////////////////////////////////////////////////////

#include "Main.h"

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
#include "Properties.h"
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
	ID_MenuFileImport,
	ID_MenuFileExit,
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
	menuFile->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { onMenuConfigure(e, ID_MenuFile); }, wxID_ANY);

	// file, select
	menus_[ID_MenuFileSelect] =
		menuFile->Append(ID_MenuFileSelect, "Select...\tCtrl-S", "Select volumes");

	// file, delete
	menus_[ID_MenuFileDelete] =
		menuFile->Append(ID_MenuFileDelete, "Delete...\tCtrl-D", "Delete file");

	// file, select
	menus_[ID_MenuFileImport] =
		menuFile->Append(ID_MenuFileImport, "Import files...\tCtrl-I", "Import files");

	menuFile->AppendSeparator();

	// file, exit
	menus_[ID_MenuFileExit] = menuFile->Append(wxID_EXIT);

	// view, menu
	wxMenu* menuView = new wxMenu;
	menuView->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { onMenuConfigure(e, ID_MenuView); }, wxID_ANY);

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
	menuTools->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { onMenuConfigure(e, ID_MenuTools); }, wxID_ANY);

	// tools, video updater
	menus_[ID_MenuToolsVideoUpdater] =
		menuTools->Append(ID_MenuToolsVideoUpdater, "&Update video...\tCtrl-V", "Video update tools");

	// test menu
	wxMenu* menuTest = new wxMenu;
	menuTools->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { onMenuConfigure(e, ID_MenuTest); }, wxID_ANY);

	// test, test dialog
	menus_[ID_MenuTestTest] =
		menuTest->Append(ID_MenuTestTest, "Test\tCtrl-V", "Runs unit tests");

	// test, tryout
	menus_[ID_MenuTestTryout] =
		menuTest->Append(ID_MenuTestTryout, "TryOut...\tCtrl-T", "Hook for experimental code");

	// test, to logger
	menus_[ID_MenuTestToLogger] =
		menuTest->Append(ID_MenuTestToLogger, "To Logger\tCtrl-", "Output data to logger");

	// help menu
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { onMenuConfigure(e, ID_MenuHelp); }, wxID_ANY);

	menus_[ID_MenuHelpAbout] = menuHelp->Append(wxID_ABOUT);

	// setup menu bar
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile,  "File");
	menuBar->Append(menuView,  "View");
	menuBar->Append(menuTools, "Tools");
	menuBar->Append(menuTest,  "Test");
	menuBar->Append(menuHelp,  "Help");
	SetMenuBar(menuBar);

	// bind the menu selected event to the dispatch function
	Bind(wxEVT_MENU, &Main::onMenuSelectedDispatch, this, wxID_ANY);

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
void Main::onMenuConfigure(wxMenuEvent& event, int menuId)
{
	int item = grid_->getSelectedRow();

	switch (menuId)
	{
	case ID_MenuFile:
		menus_[ID_MenuFileSelect]->Enable(menuEnabled(ID_MenuFileSelect, item));
		menus_[ID_MenuFileDelete]->Enable(menuEnabled(ID_MenuFileDelete, item));
		menus_[ID_MenuFileImport]->Enable(menuEnabled(ID_MenuFileImport, item));
		menus_[ID_MenuFileExit]  ->Enable(menuEnabled(ID_MenuFileExit,   item));
		break;
	case ID_MenuView:
		menus_[ID_MenuViewPlay]			->Enable(menuEnabled(ID_MenuViewPlay, 		   item));
		menus_[ID_MenuViewTogglePreview]->Enable(menuEnabled(ID_MenuViewTogglePreview, item));
		menus_[ID_MenuViewTogglePreview]->Check(Constants::previewMode);
		if (Constants::previewMode)
			menus_[ID_MenuViewTogglePreview]->SetItemLabel("Preview mode off");
		else
			menus_[ID_MenuViewTogglePreview]->SetItemLabel("Preview mode on");
		menus_[ID_MenuViewMoreImages]->Enable(menuEnabled(ID_MenuViewMoreImages, item));
		menus_[ID_MenuViewLessImages]->Enable(menuEnabled(ID_MenuViewLessImages, item));
		break;
	case ID_MenuTools:
		menus_[ID_MenuToolsVideoUpdater]->Enable(menuEnabled(ID_MenuToolsVideoUpdater, 	item));
		break;
	case ID_MenuTest:
		menus_[ID_MenuTestTest]	   ->Enable(menuEnabled(ID_MenuTestTest, 	 item));
		menus_[ID_MenuTestTryout]  ->Enable(menuEnabled(ID_MenuTestTryout,   item));
		menus_[ID_MenuTestToLogger]->Enable(menuEnabled(ID_MenuTestToLogger, item));
		break;
	case ID_MenuHelp:
		break;
	}
	event.Skip();
}

bool Main::menuEnabled(const int menuId, const int item) const
{
	FileSetT fs;
	if (item != -1)
		fs = FileSetManager::getFileSet(item);

	switch (menuId)
	{
	case ID_MenuFileSelect:
		return true;
	case ID_MenuFileDelete:
		return (fs.get() != nullptr);
	case ID_MenuFileImport:
		return true;
	case ID_MenuFileExit:
	case wxID_EXIT:
		return true;
	case ID_MenuViewPlay:
		return (fs.get() != nullptr && fs->hasVideo());
	case ID_MenuViewTogglePreview:
		return true;
	case ID_MenuViewMoreImages:
		return Constants::imageBrowserSize < Constants::imageBrowserSizeMax;
	case ID_MenuViewLessImages:
		return Constants::imageBrowserSize > Constants::imageBrowserSizeMin;
	case ID_MenuToolsVideoUpdater:
		return (fs.get() != nullptr && fs->hasVideo());
	case ID_MenuTestTest:
		return true;
	case ID_MenuTestTryout:
		return true;
	case ID_MenuTestToLogger:
		return true;
	case ID_MenuHelpAbout:
		return true;
	case ID_PageUp:
		return true;
	case ID_PageDown:
		return true;
	case ID_CursorUp:
		return true;
	case ID_CursorDown:
		return true;
	case ID_CursorLeft:
		return true;
	case ID_CursorRight:
		return true;
	default:
		return false;
	}
}

// called when a menu option has been selected
void Main::onMenuSelectedDispatch(wxCommandEvent& event)
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
	case ID_MenuFileImport:
		importFile();
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
	event.Skip();
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
	menuItem->Enable(menuEnabled(ID_MenuFileDelete, item));

	menuItem = menu->Append(ID_MenuViewPlay, "Play");
	menuItem->Enable(menuEnabled(ID_MenuViewPlay, item));

	menuItem = menu->Append(ID_MenuToolsVideoUpdater, "Update video ...");
	menuItem->Enable(menuEnabled(ID_MenuToolsVideoUpdater, item));

	menuItem = menu->Append(ID_MenuViewTogglePreview, "Preview mode", wxEmptyString, wxITEM_CHECK);
	menuItem->Enable(menuEnabled(ID_MenuViewTogglePreview, item));
	menuItem->Check(Constants::previewMode);
	if (Constants::previewMode)
		menuItem->SetItemLabel("Preview mode off");
	else
		menuItem->SetItemLabel("Preview mode on");

	return menu;
}

//--------------------------------------------------------------------------
// the menu option handlers
//--------------------------------------------------------------------------

void Main::deleteFile(wxCommandEvent& event, const int row, FileSet& fileset)
{
	Logger::info("Delete file {}, row = {}", fileset.getId(), row);
}

void Main::play(wxCommandEvent& event, const int row, FileSet &fileset)
{
	Logger::info("Play {}, row = {}", fileset.getId(), row);

	fileset.properties().setDateTimeNow("lasttime");
	fileset.properties().incCount("times");
	refresh(fileset);

	std::string cmd = Constants::videoPlayer + SU::doubleQuotes(fileset.getVideo());

	ShellExecute::shell(cmd);
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

void Main::importFile()
{
	// user selects a file
	std::string prompt = Constants::title + std::string(" - Select file");
    wxFileDialog dlg(this, prompt.c_str(), Constants::lastDirectory, "",
                   "all files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (dlg.ShowModal() == wxID_CANCEL) return;

	Constants::lastDirectory = dlg.GetDirectory();

	// check file is a valid type
	std::string src = std::string(dlg.GetPath());
	if (!FileSet::isValidType(src))
	{
		Utilities::messageBox("{}\ncannot be imported, it is not a valid file type, image, video or link", "Import", wxOK, this, src);
		return;
	}

	// find a volume with enough free space
	long long size = FU::getFileSize(src);
	VolumeT vol = VolumeManager::findVolumeWithFreeSpace(size);

	if (vol.get() != nullptr)
	{
		// destination file name
		std::string dest = vol->getFilesDirectory() + Constants::pathSeparator +
				FileSetManager::getNextId() + std::string(".") + FU::getExt(src);

		// ask user whether to move or copy file
//		int ret = Utilities::messageBox(
//				"Do you want to copy or move the file\n{}\n to {}\n\nNo=Copy, Yes=Copy", "Import",
//				wxYES_NO | wxCANCEL, this, src, dest);

		wxMessageDialog *box = new wxMessageDialog(
				this,
				fmt::format("Do you want to copy or move the file\\n{}\n\n to {}", src, dest),
				(Constants::title + std::string(" - Import")).c_str(),
				wxYES_NO | wxCANCEL | wxICON_QUESTION);

		box->SetYesNoLabels("Move", "Copy");
		int ret = box->ShowModal();
		if (ret == wxCANCEL) return;

		if (FU::fileExists(dest))
		{
			// shouldn't ever happen as a unique dest filename has been generated
			if (Utilities::messageBox(
					"The destination file {} already exists", "Import",
					wxOK | wxCANCEL, this, dest)
				== wxCANCEL) return;
		}

		// move/copy file
		bool ok = true;
		if (ret == wxYES) ok = FU::moveFile(src,  dest, true);
		else              ok = FU::copyFile(src, dest, true);
		if (!ok) return;

		FileSetT fs = std::make_shared<FileSet>(vol.get(), dest);

		// add to data and refresh gui
		vol->addFileSet(fs);
		FileSetManager::addFileSet(fs);
		Main::get().addFileSet(fs);
	}
	else
		Logger::error("Insufficient disk space to save import file {}", src);

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
	result &= Properties::test();

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





