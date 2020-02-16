/*
 * GridTable.h
 *
 *  Created on: 13 Feb 2020
 *      Author: richard
 */

#ifndef COMMON_GRIDTABLE_H_
#define COMMON_GRIDTABLE_H_

#include <string>
#include <wx/grid.h>
#include <wx/wx.h>

class GridTable : public wxGridTableBase
{
public:
	GridTable();
	virtual ~GridTable();

	virtual int GetNumberRows();
	virtual int GetNumberCols();

	virtual wxString GetValue(int row, int col);
	virtual void SetValue (int row, int col, const wxString &value);

	virtual wxString GetColLabelValue (int col);

	void initialise();

};

#endif /* COMMON_GRIDTABLE_H_ */
