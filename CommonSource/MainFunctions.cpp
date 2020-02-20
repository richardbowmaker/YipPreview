

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
#include <stdarg.h>
#include <stdio.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/mediactrl.h>
#include <iterator>
#include <wchar.h>
#include <thread>


#include "Constants.h"
#include "FileSet.h"
#include "GridTable.h"
#include "GridTableTest.h"
#include "Logger.h"
#include "Tryout.h"
#include "Utilities.h"
#include "ShellExecute.h"
#include "ImagePanel.h"
#include "MediaPreviewPlayer.h"
#include "FileSetManager.h"

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
	ID_MenuTools,
	ID_MenuToolsTryout,
	ID_MenuHelp,
	ID_MenuHelpAbout,

	// other functions, accelerators
	ID_PageUp,
	ID_PageDown
};

void MyFrame::menuOpenDispatch(wxMenuEvent& event, int menuId)
{
	// is there a row selected
	bool isSelected = (getSelectedRow() != -1);
	
	switch (menuId)
	{
	case ID_MenuFile:
		menus_[ID_MenuFileDelete]->Enable(isSelected);
		break;
	case ID_MenuView:
		menus_[ID_MenuViewPlay]->Enable(isSelected);
		break;
	case ID_MenuTools:
		break;
	case ID_MenuHelp:
		break;
	}
}

void MyFrame::menuSelectedDispatch(wxCommandEvent& event)
{
	FileSetT fs;
	int row = getSelectedRow();
	if (row != -1)
		fs = FileSetManager::getFileSet(row);

	switch (event.GetId())
	{
	case ID_MenuFileDelete:
		if (row != -1) deleteFile(event, row, *fs.get());
		break;
	case ID_MenuViewPlay:
		if (row != -1) play(event, row, *fs.get());
		break;
	case ID_MenuToolsTryout:
		tryout(event, row);
		break;
	case ID_MenuHelpAbout:
		break;
	case ID_PageUp:
		images_->pageUp();
		break;
	case ID_PageDown:
		images_->pageDown();
		break;
	}
}

void MyFrame::setupMenus()
{
	// file menu
	wxMenu* menuFile = new wxMenu;
	menuFile->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuOpenDispatch(e, ID_MenuFile); }, wxID_ANY);
	
	// file delete
	menus_[ID_MenuFileDelete] = 
		menuFile->Append(ID_MenuFileDelete, "Delete...\tCtrl-D", "Delete file");

	menuFile->AppendSeparator();

	// file exit
	menus_[ID_MenuFileExit] = menuFile->Append(wxID_EXIT);

	// view menu
	wxMenu* menuView = new wxMenu;
	menuView->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuOpenDispatch(e, ID_MenuView); }, wxID_ANY);

	// view play
	menus_[ID_MenuViewPlay] =
		menuView->Append(ID_MenuViewPlay, "Play\tF1", "Play file");

	// tools menu
	wxMenu* menuTools = new wxMenu;
	menuTools->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuOpenDispatch(e, ID_MenuTools); }, wxID_ANY);

	// tools tryout
	menus_[ID_MenuToolsTryout] =
		menuTools->Append(ID_MenuToolsTryout, "&TryOut...\tCtrl-T", "Hook for experimental code");

	// help menu
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) -> void { menuOpenDispatch(e, ID_MenuHelp); }, wxID_ANY);
	
	menus_[ID_MenuHelpAbout] = menuHelp->Append(wxID_ABOUT);

	// setup menu bar
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile,  "File");
	menuBar->Append(menuView,  "View");
	menuBar->Append(menuTools, "Tools");
	menuBar->Append(menuHelp,  "Help");
	SetMenuBar(menuBar);

	// bind the menu selected event to the dispatch function
	Bind(wxEVT_MENU, &MyFrame::menuSelectedDispatch, this, wxID_ANY);

	// accelerators
	wxAcceleratorEntry entries[11];
	entries[ 0].Set(wxACCEL_NORMAL, WXK_RETURN,			 ID_MenuViewPlay);
	entries[ 1].Set(wxACCEL_NORMAL, WXK_NUMPAD_PAGEUP,	 ID_PageUp);
	entries[ 2].Set(wxACCEL_NORMAL, (int)'Q',			 ID_PageUp);
	entries[ 3].Set(wxACCEL_NORMAL, (int)'W',			 ID_PageUp);
	entries[ 4].Set(wxACCEL_NORMAL, (int)'E',			 ID_PageUp);
	entries[ 5].Set(wxACCEL_NORMAL, (int)'R',			 ID_PageUp);
	entries[ 6].Set(wxACCEL_NORMAL, WXK_NUMPAD_PAGEDOWN, ID_PageDown);
	entries[ 7].Set(wxACCEL_NORMAL, (int)'Z',			 ID_PageDown);
	entries[ 8].Set(wxACCEL_NORMAL, (int)'X',			 ID_PageDown);
	entries[ 9].Set(wxACCEL_NORMAL, (int)'C',			 ID_PageDown);
	entries[10].Set(wxACCEL_NORMAL, (int)'V',			 ID_PageDown);

	wxAcceleratorTable accel(11, entries);
	SetAcceleratorTable(accel);

	
}

wxMenu *MyFrame::getGridPopupMenu()
{
	bool isSelected = (getSelectedRow() != -1);

	wxMenu *menu = new wxMenu();
	wxMenuItem *menuItem;
	menuItem = menu->Append(ID_MenuFileDelete, L"Delete ...");
	menuItem->Enable(isSelected);

	menuItem = menu->Append(ID_MenuViewPlay,   L"Play");
	menuItem->Enable(isSelected);

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

void MyFrame::play(wxCommandEvent& event, const int row, FileSet& fileset)
{
	Logger::info(L"Play %ls, %d", fileset.getId().c_str(), row);
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

	return;

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






