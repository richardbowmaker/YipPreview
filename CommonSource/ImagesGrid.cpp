
#include "ImagesGrid.h"

#include "FileSetManager.h"
#include "Logger.h"
#include "Main.h"
#include "VolumeManager.h"

ImagesGrid::ImagesGrid(wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos /*= wxDefaultPosition*/,
    const wxSize& size /*= wxDefaultSize*/,
    long style /*= wxWANTS_CHARS*/,
    const wxString& name /*= wxGridNameStr*/) :
    wxGrid(parent, id, pos, size, style, name),
	iServer_(nullptr)
{
}

void ImagesGrid::initialise(ImagesGridServer *iServer)
{
	iServer_ = iServer;

	Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &ImagesGrid::eventDispatch, this, wxID_ANY);
	Bind(wxEVT_GRID_SELECT_CELL, &ImagesGrid::eventDispatch, this, wxID_ANY);
	Bind(wxEVT_SET_FOCUS, &ImagesGrid::onFocus, this, wxID_ANY);
	Bind(wxEVT_GRID_COL_SORT, &ImagesGrid::onColSort, this, wxID_ANY);

	SetColLabelSize(GetDefaultRowSize());
}

void ImagesGrid::uninitialise()
{
	Unbind(wxEVT_GRID_CELL_RIGHT_CLICK, &ImagesGrid::eventDispatch, this, wxID_ANY);
	Unbind(wxEVT_GRID_SELECT_CELL, &ImagesGrid::eventDispatch, this, wxID_ANY);
	Unbind(wxEVT_SET_FOCUS, &ImagesGrid::onFocus, this, wxID_ANY);
	Unbind(wxEVT_GRID_COL_SORT, &ImagesGrid::onColSort, this, wxID_ANY);

	SetTable(nullptr);
}

void ImagesGrid::populate()
{
	SetTable(iServer_->gridGetTable());
	SetSelectionMode(wxGrid::wxGridSelectRows);
	HideRowLabels();
	EnableEditing(false);
	SetColSize(0, 220);
	SetColSize(1, 140);
	SetColSize(2, 50);
	SetColSize(3, 50);
	SetColSize(4, 100);
	SetColSize(5, 150);
	SetColSize(6, 50);
	SetColSize(7, 75);
	SetColSize(8, 75);
}

void ImagesGrid::eventDispatch(wxGridEvent &event)
{
	int id = event.GetEventType();

	if (id == wxEVT_GRID_CELL_RIGHT_CLICK)
	{
		int row = YToRow(event.GetPosition().y - GetColLabelSize()) +
			getTopRow();

		if (row >= 0 && row < GetNumberRows())
		{
			SelectRow(row);
			iServer_->gridSetSelected(row);
			wxMenu* menu = iServer_->gridGetPopupMenu(row);
			PopupMenu(menu);
			delete menu;
		}
	}
	if (id == wxEVT_GRID_SELECT_CELL)
	{
		iServer_->gridSetSelected(getSelectedRow());
	}
}

void ImagesGrid::onFocus(wxFocusEvent& event)
{
	iServer_->gridGotFocus();
}

void ImagesGrid::onColSort(wxGridEvent &event)
{
	ColT col = static_cast<ColT>(event.GetCol());
	FileSetManager::sort(col);
	Main::get().refresh();
}

void ImagesGrid::refreshRowsAppended(const int noOfRows)
{
    wxGridTableMessage msg(
        GetTable(),
        wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
        noOfRows);
    ProcessTableMessage(msg);
    ForceRefresh();
}

void ImagesGrid::refreshRowsDeleted(const int atRow, const int noOfRows)
{
    wxGridTableMessage msg(
        GetTable(),
        wxGRIDTABLE_NOTIFY_ROWS_DELETED,
        atRow,
        noOfRows);
    ProcessTableMessage(msg);
    ForceRefresh();
}

void ImagesGrid::refreshRowsInserted(const int atRow, const int noOfRows)
{
    wxGridTableMessage msg(
        GetTable(),
        wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
        atRow,
        noOfRows);
    ProcessTableMessage(msg);
    ForceRefresh();
}

void ImagesGrid::refresh()
{
    ForceRefresh();
}

int ImagesGrid::getSelectedRow() const
{
    wxArrayInt rows = GetSelectedRows();
    if (rows.GetCount() == 1)
        return rows.Item(0);
    else
        return -1;
    return 0;
}

int ImagesGrid::getTopRow() const
{
    int x, y;
    CalcUnscrolledPosition(0, 0, &x, &y);
    return YToRow(y);
}

// return no. of visible rows
int ImagesGrid::getVisibleRows()
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







