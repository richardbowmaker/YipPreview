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

#include "ImagesBrowser.h"

class FileSet;
class GridEx;
class GridTable;
class GridTableTest;
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

class MyFrame: public wxFrame, ImagesBrowserData
{
public:

    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MyFrame() = default;

    static MyFrame& getMainFrame();

    // ImagesBrowserData interface
    virtual int getNoOfRows();
    virtual int getNoOfCols();
    virtual int getNoOfImages();
    virtual int getSelected();
    virtual void setSelected(const int selected);
    virtual std::wstring getImage(const int n);

private:

    // grid functions
    void initialiseGrid(wxPanel* panel);
    void uninitialiseGrid();
    void populateGrid();
    void gridEventDispatch(wxGridEvent &event);

    void onFocus(wxFocusEvent& event);

    Logger *setupLogger(wxPanel *panel);

    // menus
    void setupMenus();
    void menuSelectedDispatch(wxCommandEvent &event);
    void menuOpenDispatch(wxMenuEvent &event, int menuId);
    wxMenu *getGridPopupMenu();

    void deleteFile(wxCommandEvent& event, const int row, FileSet& fileset);
    void play(wxCommandEvent& event, const int row, FileSet& fileset);
    void tryout(wxCommandEvent& event, const int row);

    FileSet& getSelectedFileSet() const;

    void pageUp();
    void pageDown();
	void cursorUp();
	void cursorDown();

    void OnClose(wxCloseEvent &event);
    void OnThread(wxCommandEvent &event);
	void OnProcessCustom(wxCommandEvent &event);
	void OnShellExecute(wxShellExecuteEvent &event);

	MediaPreviewPlayer *player_;
	GridEx *grid_;
    GridTable *table_;

    // images browser
    ImagesBrowser *images_;
    int browserRows_;
    int browserCols_;

    std::map<int, wxMenuItem*> menus_;
    static MyFrame* this_;
};

#endif /* COMMON_MAIN_H_ */
