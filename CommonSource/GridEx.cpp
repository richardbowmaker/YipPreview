
#include "GridEx.h"

#include "Logger.h"

GridEx::GridEx(wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos /*= wxDefaultPosition*/,
    const wxSize& size /*= wxDefaultSize*/,
    long style /*= wxWANTS_CHARS*/,
    const wxString& name /*= wxGridNameStr*/) :
    wxGrid(parent, id, pos, size, style, name)
{
}

void GridEx::refreshRowsAppended(const int noOfRows)
{
    wxGridTableMessage msg(
        GetTable(),
        wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
        noOfRows);
    ProcessTableMessage(msg);
    ForceRefresh();
}

void GridEx::refreshRowsDeleted(const int atRow, const int noOfRows)
{
    wxGridTableMessage msg(
        GetTable(),
        wxGRIDTABLE_NOTIFY_ROWS_DELETED,
        atRow, 
        noOfRows);
    ProcessTableMessage(msg);
    ForceRefresh();
}

void GridEx::refreshRowsInserted(const int atRow, const int noOfRows)
{
    wxGridTableMessage msg(
        GetTable(),
        wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
        atRow, 
        noOfRows);
    ProcessTableMessage(msg);
    ForceRefresh();
}

void GridEx::refresh()
{
    ForceRefresh();
}

int GridEx::getSelectedRow() const
{
    wxArrayInt rows = GetSelectedRows();
    if (rows.GetCount() == 1)
        return rows.Item(0);
    else
        return -1;
    return 0;
}

int GridEx::getTopRow() const
{
    int x, y;
    CalcUnscrolledPosition(0, 0, &x, &y);
    return YToRow(y);
}

// return no. of visible rows
int GridEx::getVisibleRows()
{
    wxPoint p = GetViewStart();
    wxSize  s = GetGridWindow()->GetClientSize();

    int rt = YToRow(p.y);
    int rb = YToRow(p.y + s.GetHeight());
    if (rb < 0)
        return GetNumberRows();
    else
        return rb - rt;
}

void GridEx::setScrollParams()
{
	if (!HasScrollbar(wxVERTICAL)) return;
	int r = GetNumberRows() + 1;
//	SetScrollbar(wxVERTICAL, 0, getVisibleRows(), r, true);


	wxSize s = GetClientSize();
	int rh = GetDefaultRowSize();
//	SetScrollbar(wxVERTICAL, 0,, r, true);
}


void GridEx::setTopRow(const int row)
{
	Logger::info(L"------------------------------");
	Logger::info(L"GetColLabelSize %d", GetColLabelSize());
	Logger::info(L"GetDefaultRowSize %d", GetDefaultRowSize());
	wxSize s;
	s = GetClientSize();
	Logger::info(L"GetClientSize %d %d", s.GetWidth(), s.GetHeight());

	wxPoint p;
	p = GetViewStart();
	Logger::info(L"GetViewStart %d %d", p.x, s.y);

	int x, y;
	GetVirtualSize(&x, &y);
	Logger::info(L"GetVirtualSize %d %d", x, y);



	Logger::info(L"GetScrollPos %d", GetScrollPos(wxVERTICAL));
	Logger::info(L"GetScrollRange %d", GetScrollRange(wxVERTICAL));
	Logger::info(L"GetScrollThumb %d", GetScrollThumb(wxVERTICAL));


	
/*
	Logger::info(L" %d", );
		

	
    int s = GetDefaultRowSize();
    int sx, sy;
    GetScrollPixelsPerUnit(&sx, &sy);
    int pos = (s * row + GetColLabelSize()) / sy;
    Scroll(0, pos);
*/
	
}




    
