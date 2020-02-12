

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

#include "FileSet.h"
#include "Logger.h"
#include "Tryout.h"
#include "Utilities.h"
#include "ShellExecute.h"
#include "ImagePanel.h"
#include "MediaPreviewPlayer.h"

enum MenuIDsT
{
	ID_Dummy = 0,
	ID_FileDelete,
	ID_FileImport,
	ID_ToolsTryout
};

void MyFrame::setupMenus()
{
	// file menu
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_FileDelete, "Delete...\tCtrl-D", "Delete items in a file set");
	Bind(wxEVT_MENU, &MyFrame::onFileDelete, this, ID_FileDelete);
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);
	Bind(wxEVT_MENU, &MyFrame::onFileExit, this, wxID_EXIT);

	// view menu
	wxMenu* menuView = new wxMenu;

	// tools menu
	wxMenu* menuTools = new wxMenu;
	menuTools->Append(ID_ToolsTryout, "&TryOut...\tCtrl-T", "Hook for experimental code");
	Bind(wxEVT_MENU, &MyFrame::onToolsTryout, this, ID_ToolsTryout);

	// help menu
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);
	Bind(wxEVT_MENU, &MyFrame::onHelpAbout, this, wxID_ABOUT);

	// setup menu bar
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "File");
	menuBar->Append(menuView, "View");
	menuBar->Append(menuTools, "Tools");
	menuBar->Append(menuHelp, "Help");
	SetMenuBar(menuBar);
}

//--------------------------------------------------------------------------
// menu handlers 
//--------------------------------------------------------------------------

void MyFrame::onFileExit(wxCommandEvent& event)
{
	Close(true);
}

void MyFrame::onFileDelete(wxCommandEvent& event)
{
	deleteFile(getSelectedFileSet());
}

void MyFrame::onToolsTryout(wxCommandEvent& event)
{
	tryout(getSelectedFileSet());
}

void MyFrame::onHelpAbout(wxCommandEvent& event)
{
	wxMessageBox("This is a wxWidgets' Hello world sample",
		"About Hello World", wxOK | wxICON_INFORMATION);
}

//--------------------------------------------------------------------------
// functions 
//--------------------------------------------------------------------------

void MyFrame::deleteFile(FileSet& fileSet)
{
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

void MyFrame::tryout(FileSet& fileSet)
{
	ShellExecute::shellAsyncGui(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)", 
		MyFrame::getMainFrame().GetEventHandler());
	return;

	//CreateThread(NULL, 0, &MyThread, NULL, 0, NULL);
	//Logger::info(L"thread started");
	//return;

#ifdef WINDOWS_BUILD
	player_->setFile(LR"(D:\Projects\WxWidgets\YipPreview\Tryout\f3.mp4)");
#elif LINUX_BUILD
	player_->setFile(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/f3.mp4)");
#endif
	player_->startPreview();
	//	player_->Load(LR"(D:\Projects\WxWidgets\YipPreview\Tryout\f3.mp4)");
	return;

	bool b;

	// linux copy file
	b = FU::copyFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	b = FU::copyFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)", false);
	b = FU::copyFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a12345.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a12345.jpg)");

	// linux file exists
	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1)");
	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a12345.jpg)");

	// linux delete file
	b = FU::deleteFile(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	b = FU::deleteFile(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");

	// linux move file
	b = FU::moveFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	b = FU::moveFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)", false);
	b = FU::moveFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	b = FU::moveFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a03.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	b = FU::moveFile(
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a03.jpg)",
		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout2/a03.jpg)");

	//b = FU::deleteFile(LR"(D:\IMAG0036_Copy.jpg)");
	//b = FU::deleteFile(LR"(D:\IMAG0036_Copy.jpg)");
//	bool b = FU::copyFile(LR"(D:\IMAG0036.jpg)", LR"(D:\IMAG0036_Copy.jpg)");
//	b = FU::copyFile(LR"(D:\IMAG0036.jpg)", LR"(D:\IMAG0036_Copy.jpg)", false);
	//b = FU::fileExists(LR"(D:\emails)");
	//b = FU::fileExists(LR"(D:\myimage.jpg)");
	//b = FU::fileExists(LR"(D:\xyz)");
	b = false;

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


	// linux


		//	files.clear();
	//	FU::FindMatchingFiles(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout)", files, L"a1*.jpg");
	//	Logger::info(files, L"a1*.jpg files");
	//

	//	StringsT files;
	//	FU::FindFiles(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout)", files);
	//	Logger::info(files, L"All files");
	//
	//	files.clear();
	//	FU::FindMatchingFiles(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout)", files, L"a1*.jpg");
	//	Logger::info(files, L"a1*.jpg files");
	//

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






