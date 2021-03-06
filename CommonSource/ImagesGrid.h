/*
 * ImagesGrid.h
 *
 *  Created on: 22 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_IMAGESGRID_H_
#define COMMON_IMAGESGRID_H_

#include <string>
#include <wx/wx.h>
#include <wx/grid.h>

#include "_Types.h"
#include "FileSet.h"

class ImagesGridServer
{
public:

	virtual wxMenu *gridGetPopupMenu(const int item) = 0;
	virtual wxGridTableBase *gridGetTable() = 0;
	virtual void gridSetSelected(const int selected) = 0;
	virtual void gridGotFocus() = 0;
};

class ImagesGrid : public wxGrid
{
public:

	ImagesGrid(wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxWANTS_CHARS,
        const wxString& name = wxGridNameStr);

    void initialise(ImagesGridServer *iServer);
    void uninitialise();
    void populate();

    // grid functions
    void refreshRowsAppended(const int noOfRows);
    void refreshRowsDeleted(const int atRow, const int noOfRows);
    void refreshRowsInserted(const int atRow, const int noOfRows);
    void refresh();
    int  getSelectedRow() const;
    int  getTopRow() const;
    int  getVisibleRows();

private:

    void onFocus(wxFocusEvent& event);
    void onEventDispatch(wxGridEvent &event);
    void onColSort(wxGridEvent &event);
    void onMouseMove(wxMouseEvent &event);

    ImagesGridServer *iServer_;
    wxPoint prevMouse_;
};

#endif /* COMMON_IMAGESGRID_H_ */
