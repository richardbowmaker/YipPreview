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

#include <map>

#include "_Types.h"
#include "ImagesBrowser.h"
#include "ImagesGrid.h"
#include "Volume.h"

class FileSet;
class GridTable;
class ImagesGrid;
class Logger;
class MediaPreviewPlayer;
class wxGridEvent;
class wxMenuItem;
class wxMenu;
class wxShellExecuteEvent;

class MyApp: public wxApp
{
public:

	MyApp();
    virtual bool OnInit();
};

class MyFrame: public wxFrame, ImagesBrowserServer, ImagesGridServer
{
public:

    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MyFrame() = default;

    static MyFrame& get();

    void refresh(const FileSet &fileset);

private:

    // ImagesBrowserData interface
	virtual int browserGetNoOfRows();
	virtual int browserGetNoOfCols();
	virtual int browserGetNoOfImages();
	virtual int browserGetSelected();
	virtual void browserSetSelected(const int selected);
	virtual std::wstring browserGetImage(const int n);
	virtual std::wstring browserGetVideo(const int n);
	virtual wxMenu *browserGetPopupMenu(const int item);

	// ImagesGridServer interface
	virtual wxMenu *gridGetPopupMenu(const int item);
	virtual wxGridTableBase *gridGetTable();
	virtual void gridSetSelected(const int selected);
	virtual void gridGotFocus();


    void onFocus(wxFocusEvent& event);

    Logger *setupLogger(wxPanel *panel);
    void toLogger();

    // menus
    void setupMenus();
    void menuSelectedDispatch(wxCommandEvent &event);
    void menuConfigure(wxMenuEvent &event, int menuId);
    wxMenu *getPopupMenu(const int item);


    void deleteFile(wxCommandEvent& event, const int row, FileSet &fileset);
    void play(wxCommandEvent &event, const int row, FileSet &fileset);
    void tryout(wxCommandEvent &event, const int row);
    void togglePreviewMode();
    void unitTests();

    FileSet& getSelectedFileSet() const;

    void pageUp();
    void pageDown();
	void cursorUp();
	void cursorDown();
	void cursorLeft();
	void cursorRight();

    void OnClose(wxCloseEvent &event);
    void OnThread(wxCommandEvent &event);
	void OnProcessCustom(wxCommandEvent &event);
	void OnShellExecute(wxShellExecuteEvent &event);

	ImagesGrid *grid_;
    GridTable *table_;

    // images browser
    ImagesBrowser *images_;
    int browserRows_;
    int browserCols_;

    std::map<int, wxMenuItem*> menus_;
    static MyFrame* this_;

    // temporarily here
    VolumeT volume_;
};

#endif /* COMMON_MAIN_H_ */
