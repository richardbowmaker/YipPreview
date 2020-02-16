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

class FileSet;
class Logger;
class MediaPreviewPlayer;
class wxGrid;
class wxShellExecuteEvent;
class GridTable;
class GridTableTest;

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

    // grid functions
    void setupGrid(wxPanel* panel);
    void populateGrid();
    void refreshGridRowsAppended(const int noOfRows) const;
    void refreshGridRowsDeleted(const int atRow, const int noOfRows) const;
    void refreshGridRowsInserted(const int atRow, const int noOfRows) const;
    void refreshGrid() const;

    Logger *setupLogger(wxPanel *panel);

    // menus
    void setupMenus();

    void deleteFile(FileSet& fileSet);
    void tryout(FileSet& fileSet);

    FileSet& getSelectedFileSet() const;

    void OnClose(wxCloseEvent& event);
    void OnThread(wxCommandEvent& event);
	void OnProcessCustom(wxCommandEvent& event);
	void OnShellExecute(wxShellExecuteEvent& event);

	MediaPreviewPlayer *player_;
	wxGrid *grid_;
    GridTable *table_;

    static MyFrame* this_;
};

#endif /* COMMON_MAIN_H_ */
