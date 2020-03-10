
#ifndef GRIDEX_H_
#define GRIDEX_H_


#include <wx/wx.h>
#include <wx/grid.h>

class GridEx : public wxGrid
{
public:

    GridEx(wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxWANTS_CHARS,
        const wxString& name = wxGridNameStr);

    // grid functions
    void refreshRowsAppended(const int noOfRows);
    void refreshRowsDeleted(const int atRow, const int noOfRows);
    void refreshRowsInserted(const int atRow, const int noOfRows);
    void refresh();
    int  getSelectedRow() const;
    int  getTopRow() const;
    int  getVisibleRows();
    void setTopRow(const int row);

};


#endif // GRIDEX_H
