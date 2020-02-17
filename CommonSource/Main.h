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

class FileSet;
class Logger;
class MediaPreviewPlayer;
class wxGrid;
class wxShellExecuteEvent;
class GridTable;
class GridTableTest;
class wxMenuItem;

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

    void openMenuEvent(wxMenuEvent& event, int menuId);

    // grid functions
    void setupGrid(wxPanel* panel);
    void populateGrid();
    void refreshGridRowsAppended(const int noOfRows) const;
    void refreshGridRowsDeleted(const int atRow, const int noOfRows) const;
    void refreshGridRowsInserted(const int atRow, const int noOfRows) const;
    void refreshGrid() const;
    int getSelectedRow();

    Logger *setupLogger(wxPanel *panel);

    // menus
    void setupMenus();

    using MenuHandlerFuncT = void (MyFrame::*)(wxCommandEvent &, const int, FileSet&);
    void menuHandler(wxCommandEvent& event, MenuHandlerFuncT f);

    void deleteFile(wxCommandEvent& event, const int row, FileSet& fileset);
    void play(wxCommandEvent& event, const int row, FileSet& fileset);
    void tryout(wxCommandEvent& event, const int row, FileSet &fileset);

    FileSet& getSelectedFileSet() const;

    void OnClose(wxCloseEvent& event);
    void OnThread(wxCommandEvent& event);
	void OnProcessCustom(wxCommandEvent& event);
	void OnShellExecute(wxShellExecuteEvent& event);

	MediaPreviewPlayer *player_;
	wxGrid *grid_;
    GridTable *table_;

    std::map<int, wxMenuItem*> menus_;
    static MyFrame* this_;
};

#endif /* COMMON_MAIN_H_ */
