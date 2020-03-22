

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

enum MenuIDsT
{
	ID_Dummy = 0,

	// menus
	ID_MenuFile,
	ID_MenuFileDelete,
	ID_MenuFileExit,
	ID_MenuFileImport,
	ID_MenuView,
	ID_MenuViewPlay,
	ID_MenuViewTogglePreview,
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

void MyFrame::menuConfigure(wxMenuEvent& event, int menuId)
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
			menus_[ID_MenuViewTogglePreview]->SetItemLabel(L"Preview mode off");
		else
			menus_[ID_MenuViewTogglePreview]->SetItemLabel(L"Preview mode on");
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

void MyFrame::menuSelectedDispatch(wxCommandEvent& event)
{
	FileSetT fs;
	int row = grid_->getSelectedRow();
	if (row != -1)
		fs = FileSetManager::getFileSet(row);

	switch (event.GetId())
	{
	case ID_MenuFileDelete:
		deleteFile(event, row, *fs.get());
		break;
	case ID_MenuViewPlay:
		play(event, row, *fs.get());
		break;
	case ID_MenuViewTogglePreview:
		togglePreviewMode();
		break;
	case ID_MenuToolsVideoUpdater:
		//VideoUpdaterDialog::Run(this, fs);
		TestDialog::Run(this, fs);
		break;
	case ID_MenuTestTest:
		unitTests();
		break;
	case ID_MenuTestTryout:
		tryout(event, row);
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

void MyFrame::setupMenus()
{
	// file menu
	wxMenu* menuFile = new wxMenu;
	menuFile->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuFile); }, wxID_ANY);
	
	// file, delete
	menus_[ID_MenuFileDelete] = 
		menuFile->Append(ID_MenuFileDelete, L"Delete...\tCtrl-D", L"Delete file");

	menuFile->AppendSeparator();

	// file, exit
	menus_[ID_MenuFileExit] = menuFile->Append(wxID_EXIT);

	// view, menu
	wxMenu* menuView = new wxMenu;
	menuView->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuView); }, wxID_ANY);

	// view, play
	menus_[ID_MenuViewPlay] =
		menuView->Append(ID_MenuViewPlay, L"Play\tF1", L"Play file");

	// view, toggle preview
	menus_[ID_MenuViewTogglePreview] =
		menuView->Append(ID_MenuViewTogglePreview, L"Preview mode on/off", L"Toggles preview mode", wxITEM_CHECK);

	// tools menu
	wxMenu* menuTools = new wxMenu;
	menuTools->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuTools); }, wxID_ANY);

	// tools, video updater
	menus_[ID_MenuToolsVideoUpdater] =
		menuTools->Append(ID_MenuToolsVideoUpdater, L"&Update video...\tCtrl-V", L"Video update tools");

	// tools menu
	wxMenu* menuTest = new wxMenu;
	menuTools->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuTest); }, wxID_ANY);

	// test, test dialog
	menus_[ID_MenuTestTest] =
		menuTest->Append(ID_MenuTestTest, L"Test\tCtrl-V", L"Runs unit tests");

	// test, tryout
	menus_[ID_MenuTestTryout] =
		menuTest->Append(ID_MenuTestTryout, L"TryOut...\tCtrl-T", L"Hook for experimental code");

	// test, to logger
	menus_[ID_MenuTestToLogger] =
		menuTest->Append(ID_MenuTestToLogger, L"To Logger\tCtrl-L", L"Hook for experimental code");

	// help menu
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuConfigure(e, ID_MenuHelp); }, wxID_ANY);
	
	menus_[ID_MenuHelpAbout] = menuHelp->Append(wxID_ABOUT);

	// setup menu bar
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile,  L"File");
	menuBar->Append(menuView,  L"View");
	menuBar->Append(menuTools, L"Tools");
	menuBar->Append(menuTest,  L"Test");
	menuBar->Append(menuHelp,  L"Help");
	SetMenuBar(menuBar);

	// bind the menu selected event to the dispatch function
	Bind(wxEVT_MENU, &MyFrame::menuSelectedDispatch, this, wxID_ANY);

	// accelerators
	std::vector<wxAcceleratorEntry> keys;
	keys.emplace_back(wxACCEL_NORMAL, WXK_RETURN,		ID_MenuViewPlay);
	keys.emplace_back(wxACCEL_NORMAL, WXK_PAGEUP,		ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'Q',			ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'W',			ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'E',			ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, (int)'R',			ID_PageUp);
	keys.emplace_back(wxACCEL_NORMAL, WXK_PAGEDOWN,		ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'Z',			ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'X',			ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'C',			ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, (int)'V',			ID_PageDown);
	keys.emplace_back(wxACCEL_NORMAL, WXK_UP,           ID_CursorUp);
	keys.emplace_back(wxACCEL_NORMAL, WXK_DOWN,			ID_CursorDown);
	keys.emplace_back(wxACCEL_NORMAL, WXK_LEFT,			ID_CursorLeft);
	keys.emplace_back(wxACCEL_NORMAL, WXK_RIGHT,		ID_CursorRight);

	wxAcceleratorTable accel(keys.size(), &keys[0]);
	SetAcceleratorTable(accel);
}

wxMenu *MyFrame::getPopupMenu(const int item)
{
	FileSetT fs;
	if (item != -1) fs = FileSetManager::getFileSet(item);

	wxMenu *menu = new wxMenu();
	wxMenuItem *menuItem;
	menuItem = menu->Append(ID_MenuFileDelete, L"Delete ...");
	menuItem->Enable(fs.get() != nullptr);

	menuItem = menu->Append(ID_MenuViewPlay, L"Play");
	menuItem->Enable(fs.get() != nullptr);

	menuItem = menu->Append(ID_MenuToolsVideoUpdater, L"Update video ...");
	menuItem->Enable(fs.get() != nullptr && fs->hasVideo());

	menuItem = menu->Append(ID_MenuViewTogglePreview, L"Preview mode", wxEmptyString, wxITEM_CHECK);
	menuItem->Check(Constants::previewMode);
	if (Constants::previewMode)
		menuItem->SetItemLabel(L"Preview mode off");
	else
		menuItem->SetItemLabel(L"Preview mode on");

//	menuItem->Enable(fs.get() != nullptr && fs->hasVideo());

	menu->Bind(wxEVT_MENU, &MyFrame::menuSelectedDispatch, this, wxID_ANY);
	return menu;
}


//--------------------------------------------------------------------------
// functions 
//--------------------------------------------------------------------------

void MyFrame::deleteFile(wxCommandEvent& event, const int row, FileSet& fileset)
{
	Logger::info(L"Delete file %ls, %d", fileset.getId().c_str(), row);
}

void MyFrame::play(wxCommandEvent& event, const int row, FileSet &fileset)
{
	Logger::info(L"Play %ls, %d", fileset.getId().c_str(), row);

	fileset.properties().setDateTimeNow(L"lasttime");
	fileset.properties().incCount(L"times");
	refresh(fileset);
}

void MyFrame::pageUp()
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

void MyFrame::pageDown()
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

void MyFrame::cursorUp()
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

void MyFrame::cursorDown()
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

void MyFrame::cursorLeft()
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

void MyFrame::cursorRight()
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

void MyFrame::toLogger()
{
	VolumeManager::toLogger();
	FileSetManager::toLogger();
}

void MyFrame::togglePreviewMode()
{
	if (Constants::previewMode) images_->stopPreview();
	Constants::previewMode = !Constants::previewMode;
}

void MyFrame::unitTests()
{
	bool result = true;

	result &= Duration::test();
	result &= FileProperties::test();

	if (result) Logger::info(L"All unit tests passed");
}


//--------------------------------------------------------------------------
// trying out area 
//--------------------------------------------------------------------------

//DWORD MyThread(void*)
//{
//	for (int i = 0; i < 10; ++i)
//	{
//		Logger::info(L"From thread %d", i);
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	}
//	return 0;
//}

void MyFrame::tryout(wxCommandEvent& event, const int row)
{
//	std::wstring s(L"12:34:56");
//
//    const std::regex rex(R"(((\d{1,2}):)?(\d{1,2}):(\d{1,2}))");
//
//    std::smatch m;
//    std::string s1 = SU::wStrToStr(s);
//    int hh = 0, mm = 0, ss = 0;
//
//    if (std::regex_search(s1, m, rex))
//    {
//    	hh = atoi(m[1].str().c_str());
//    	mm = atoi(m[3].str().c_str());
//        ss = atoi(m[4].str().c_str());
//    }
//
//
//    s1 = "4:56";
//    if (std::regex_search(s1, m, rex))
//    {
//    	hh = atoi(m[1].str().c_str());
//    	mm = atoi(m[3].str().c_str());
//        ss = atoi(m[4].str().c_str());
//    }
//



    return;




//	bool b = FileProperties::test();
//
//	if (!b) Logger::error(L"property test failed");
//
//	return;
//
////	FileProperties fp;
//	std::wstring s;
//	fp.setString(L"p1", L"v1");
//	fp.setInt(L"p2", 2);
//	fp.incCount(L"p3");
//	fp.incCount(L"p3");
//	fp.incCount(L"p3");
//	fp.setDateTimeNow(L"p4");
//	s = fp.toString();
//
//	fp.clear();
//	s = fp.toString();
//	fp.fromString(L"p1;v1;p2;2");
//	s = fp.toString();
//	int c = fp.getSize();
//	s = fp.getString(L"p1");
//	c = fp.getInt(L"p2");
//	s = fp.getCount(L"p2");
//	fp.remove(L"p1");
//	s = fp.toString();
//	fp.setString(L"p2", L"");
//	s = fp.toString();
//	fp.fromString(L"");
//	s = fp.toString();
//	fp.fromString(L"p1;v1;p2");
//	s = fp.toString();
//	fp.fromString(L"p1;v1;p2;2;");
//	s = fp.toString();
//	fp.fromString(L"p1;v1;p2;;");
//	s = fp.toString();
//	fp.fromString(L"p1;v1;p2;");
//	s = fp.toString();




	int n = 0;

//	bool b;

//
//	std::wstring afn;
//	int n;
//
//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a0123456789.jpg)", 30);
//	n = afn.size();
//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Proects/WxWidgets/YipPreview/Tryout1/a0123456789.jpg)", 30);
//	n = afn.size();
//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a123456789.jpg)", 30);
//	n = afn.size();
//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projcts/WxWidgets/YipPreview/Tryout1/a012456789.jpg)", 29);
//	n = afn.size();
//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a0123456789.jpg)", 5);
//	afn = FU::abbreviateFilename(LR"(/media/012345678901234567890123456789.jpg)", 30);
//	afn = FU::abbreviateFilename(LR"(a0123456789.jpg)", 30);
//
//
//
//	b = FileSetManager::addFiles(FU::pathToLocal(LR"(\YipPreview\Tryout)").c_str());
//	Logger::info(L"Files %ls", FileSetManager::toString().c_str());
//	return;
//
//
//
//	b = SU::startsWith(L"abcde", L"abc");
//	b = SU::startsWith(L"ab", L"abc");
//	b = SU::startsWith(L"ab", L"");
//	b = SU::startsWith(L"", L"abc");
//	b = SU::startsWith(L"", L"");
//
//
//
//#ifdef WINDOWS_BUILD
//	std::wstring s;
//	s = FU::getFileStem(LR"(abc)");
//	s = FU::getFileStem(LR"(abc.txt)");
//	s = FU::getFileStem(LR"(\home\me\abc)");
//	s = FU::getFileStem(LR"(\home\me\abc.txt)");
//	s = FU::getFileStem(LR"(\home\me.you\abc)");
//	s = FU::getFileStem(LR"(\home\me.you\abc.txt)");
//	s = FU::getFileStem(LR"(\home\me\abc.txt.exe)");
//
//	s = FU::getPathNoExt(LR"(abc.txt)");
//	s = FU::getPathNoExt(LR"(abc)");
//	s = FU::getPathNoExt(LR"(\home\me\abc)");
//	s = FU::getPathNoExt(LR"(\home\me\abc.txt)");
//	s = FU::getPathNoExt(LR"(\home\me.you\abc.txt)");
//	s = FU::getPathNoExt(LR"(\home\me.you\abc)");
//
//	s = FU::getExt(LR"(abc.txt)");
//	s = FU::getExt(LR"(abc)");
//	s = FU::getExt(LR"(\home\me\abc)");
//	s = FU::getExt(LR"(\home\me\abc.txt)");
//	s = FU::getExt(LR"(\home\me.you\abc)");
//	s = FU::getExt(LR"(\home\me.you\abc.txt)");
//	return;
//#elif LINUX_BUILD
//	std::wstring s;
//	s = FU::getFileStem(LR"(abc)");
//	s = FU::getFileStem(LR"(abc.txt)");
//	s = FU::getFileStem(LR"(/home/me/abc)");
//	s = FU::getFileStem(LR"(/home/me/abc.txt)");
//	s = FU::getFileStem(LR"(/home/me.you/abc)");
//	s = FU::getFileStem(LR"(/home/me.you/abc.txt)");
//	s = FU::getFileStem(LR"(/home/me/abc.txt.exe)");
//
//	s = FU::getPathNoExt(LR"(abc.txt)");
//	s = FU::getPathNoExt(LR"(abc)");
//	s = FU::getPathNoExt(LR"(/home/me/abc)");
//	s = FU::getPathNoExt(LR"(/home/me/abc.txt)");
//	s = FU::getPathNoExt(LR"(/home/me.you/abc.txt)");
//	s = FU::getPathNoExt(LR"(/home/me.you/abc)");
//
//	s = FU::getExt(LR"(abc.txt)");
//	s = FU::getExt(LR"(abc)");
//	s = FU::getExt(LR"(/home/me/abc)");
//	s = FU::getExt(LR"(/home/me/abc.txt)");
//	s = FU::getExt(LR"(/home/me.you/abc)");
//	s = FU::getExt(LR"(/home/me.you/abc.txt)");
//	return;
//#endif
//
//
//	ShellExecute::shellAsyncGui(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)",
//		MyFrame::getMainFrame().GetEventHandler());
//	return;
//
//	//CreateThread(NULL, 0, &MyThread, NULL, 0, NULL);
//	//Logger::info(L"thread started");
//	//return;
//
//	player_->setFile(FU::pathToLocal(LR"(\YipPreview\Tryout\f3.mp4)"));
//	player_->startPreview();
//	//	player_->Load(LR"(D:\Projects\WxWidgets\YipPreview\Tryout\f3.mp4)");
//	return;
//
//
//	// linux copy file
//	b = FU::copyFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
//	b = FU::copyFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)", false);
//	b = FU::copyFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a12345.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a12345.jpg)");
//
//	// linux file exists
//	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
//	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1)");
//	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a12345.jpg)");
//
//	// linux delete file
//	b = FU::deleteFile(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
//	b = FU::deleteFile(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
//
//	// linux move file
//	b = FU::moveFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
//	b = FU::moveFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)", false);
//	b = FU::moveFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
//	b = FU::moveFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a03.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
//	b = FU::moveFile(
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a03.jpg)",
//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout2/a03.jpg)");
//
//	//b = FU::deleteFile(LR"(D:\IMAG0036_Copy.jpg)");
	//b = FU::deleteFile(LR"(D:\IMAG0036_Copy.jpg)");
//	bool b = FU::copyFile(LR"(D:\IMAG0036.jpg)", LR"(D:\IMAG0036_Copy.jpg)");
//	b = FU::copyFile(LR"(D:\IMAG0036.jpg)", LR"(D:\IMAG0036_Copy.jpg)", false);
	//b = FU::fileExists(LR"(D:\emails)");
	//b = FU::fileExists(LR"(D:\myimage.jpg)");
	//b = FU::fileExists(LR"(D:\xyz)");
	//b = false;

	// linux

//	StringsT files;
//	FU::findFiles(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout)", files);
//	Logger::info(files, L"All files");
//
//	files.clear();
//	FU::findMatchingFiles(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout)", files, L"a1*.jpg");
//	Logger::info(files, L"a1*.jpg files");
//
//
//	return;


	// windows
		//StringsT files;
		//StringsT dirs;
		//FU::findFilesDirs(LR"(D:\Projects\WxWidgets\YipPreview\Tryout)", files, dirs);
		//Logger::info(files, L"All files");
		//Logger::info(dirs, L"All directories");

		//files.clear();
		//FU::findMatchingFiles(LR"(D:\Projects\WxWidgets\YipPreview\Tryout)", files, L"*.mp4");
		//Logger::info(files, L"mp4 files");

		//files.clear();
		//FU::findMatchingFilesRex(LR"(D:\Projects\WxWidgets\YipPreview\Tryout)", files, L"a0[1-6].jpg");
		//Logger::info(files, L"regex a01.jpg to a06.jpg files");




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






