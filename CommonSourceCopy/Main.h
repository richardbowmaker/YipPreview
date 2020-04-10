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
#include "ClipboardCapture.h"
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
class Logger;

class MyApp: public wxApp
{
public:

	MyApp();
    virtual bool OnInit();
};

class Main: public wxFrame, ImagesBrowserServer, ImagesGridServer
{
public:

    Main(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~Main() = default;

    static Main& get();

    void refresh(const FileSet &fileset);
    void refresh();
    void populateGui();
    void addFileSet(FileSetT &fs);

private:

    // ImagesBrowserData interface
	virtual int browserGetNoOfRows();
	virtual int browserGetNoOfCols();
	virtual int browserGetNoOfImages();
	virtual int browserGetSelected();
	virtual void browserSetSelected(const int selected);
	virtual std::string browserGetImage(const int n);
	virtual std::string browserGetVideo(const int n);
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
    void onMenuSelectedDispatch(wxCommandEvent &event);
    void onMenuConfigure(wxMenuEvent &event, int menuId);
    wxMenu *getPopupMenu(const int item);
    bool menuEnabled(const int menuId, const int item) const;


    void deleteFile(wxCommandEvent& event, const int row, FileSet &fileset);
    void play(wxCommandEvent &event, const int row, FileSet &fileset);
    void togglePreviewMode();
    void unitTests();
	void updateNoOfImages(const int delta);
	void importFile();
    void viewRefresh();
    void pageUp();
    void pageDown();
	void cursorUp();
	void cursorDown();
	void cursorLeft();
	void cursorRight();

    void onClose(wxCloseEvent &event);
 
	// the images grid and its data
	ImagesGrid *grid_;
    GridTable *table_;

    // images browser
    ImagesBrowser *images_;

    Logger* logger_;

	// all the menus mapped by id
    std::map<int, wxMenuItem*> menus_;

	// allow global access to MyFrame via
    static Main* this_;

    ClipboardCapture clipboardCapture_;

};

#endif /* COMMON_MAIN_H_ */
