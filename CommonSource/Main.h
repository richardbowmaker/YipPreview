/*
 * Main.h
 *
 *  Created on: 19 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_MAIN_H_
#define COMMON_MAIN_H_


#include <iostream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/mediactrl.h>

#include "Logger.h"
#include "ShellExecute.h"
#include "MediaPreviewPlayer.h"

class FileSet;

class MyApp: public wxApp
{
public:

	MyApp();

    virtual bool OnInit();
};

class MyFrame: public wxFrame
{
public:

    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    static MyFrame& getMainFrame();

private:

    // menus
    void setupMenus();
    void onFileDelete(wxCommandEvent& event);
    void onFileExit(wxCommandEvent& event);
    void onToolsTryout(wxCommandEvent& event);
    void onHelpAbout(wxCommandEvent& event);

    void deleteFile(FileSet& fileSet);
    void tryout(FileSet& fileSet);


    FileSet& getSelectedFileSet() const;

    void OnClose(wxCloseEvent& event);
    void OnThread(wxCommandEvent& event);
	void OnProcessCustom(wxCommandEvent& event);
	void OnShellExecuteAny(wxShellExecuteEvent& event);
	void OnShellExecute1(wxShellExecuteEvent& event);
	void OnShellExecute2(wxShellExecuteEvent& event);

	MediaPreviewPlayer *player_;

	void OnMediaPlay(wxMediaEvent& event);

    static MyFrame* this_;
};

#endif /* COMMON_MAIN_H_ */
