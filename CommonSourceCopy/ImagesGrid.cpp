
#include "ImagesGrid.h"

#include "_Types.h"
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
	iServer_(nullptr),
	prevMouse_(wxPoint(-1, -1))
{
}

void ImagesGrid::initialise(ImagesGridServer *iServer)
{
	iServer_ = iServer;

	Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &ImagesGrid::onEventDispatch, this, wxID_ANY);
	Bind(wxEVT_GRID_SELECT_CELL, 	  &ImagesGrid::onEventDispatch, this, wxID_ANY);
	Bind(wxEVT_SET_FOCUS, 			  &ImagesGrid::onFocus, 		this, wxID_ANY);
	Bind(wxEVT_GRID_COL_SORT, 		  &ImagesGrid::onColSort, 		this, wxID_ANY);
	Bind(wxEVT_MOTION, 				  &ImagesGrid::onMouseMove, 	this, wxID_ANY);

	SetTable(iServer_->gridGetTable());
	HideRowLabels();
	SetColLabelSize(GetDefaultRowSize());
	EnableEditing(false);
}

void ImagesGrid::uninitialise()
{
	Unbind(wxEVT_GRID_CELL_RIGHT_CLICK, &ImagesGrid::onEventDispatch, 	this, wxID_ANY);
	Unbind(wxEVT_GRID_SELECT_CELL, 	  	&ImagesGrid::onEventDispatch, 	this, wxID_ANY);
	Unbind(wxEVT_SET_FOCUS, 			&ImagesGrid::onFocus, 			this, wxID_ANY);
	Unbind(wxEVT_GRID_COL_SORT, 		&ImagesGrid::onColSort, 		this, wxID_ANY);
	Unbind(wxEVT_MOTION, 				&ImagesGrid::onMouseMove, 		this, wxID_ANY);

	SetTable(nullptr);
}

void ImagesGrid::populate()
{
	SetTable(iServer_->gridGetTable());
	SetSelectionMode(wxGrid::wxGridSelectRows);
	SetColSize(static_cast<int>(ColT::Volume),    220);
	SetColSize(static_cast<int>(ColT::Mount),     220);
	SetColSize(static_cast<int>(ColT::File),      140);
	SetColSize(static_cast<int>(ColT::Type),       50);
	SetColSize(static_cast<int>(ColT::Selected),   50);
	SetColSize(static_cast<int>(ColT::Duration),  100);
	SetColSize(static_cast<int>(ColT::LastTime),  150);
	SetColSize(static_cast<int>(ColT::Times),	   50);
	SetColSize(static_cast<int>(ColT::MaxVol),     75);
	SetColSize(static_cast<int>(ColT::AverageVol), 75);
}

void ImagesGrid::onEventDispatch(wxGridEvent &event)
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
		event.Skip();
	}
	if (id == wxEVT_GRID_SELECT_CELL)
	{
		iServer_->gridSetSelected(getSelectedRow());
		event.Skip();
	}
}

void ImagesGrid::onFocus(wxFocusEvent& event)
{
	iServer_->gridGotFocus();
	event.Skip();
}

void ImagesGrid::onColSort(wxGridEvent &event)
{
	ColT col = static_cast<ColT>(event.GetCol());
	FileSetManager::sort(col);
	Main::get().refresh();
	event.Skip();
}

void ImagesGrid::onMouseMove(wxMouseEvent &event)
{
	if (prevMouse_.x != -1 && prevMouse_ == event.GetPosition())
	{
		// mouse hovering
		wxPoint p = CalcUnscrolledPosition(prevMouse_);
		int row = YToRow(p.y);
		FileSetT fs = FileSetManager::getFileSet(row);
		std::string tt = fs->getToolTip();
		Logger::info("Tooltip row {}: {}", row, tt);

	}
	prevMouse_ = event.GetPosition();
	event.Skip();
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







