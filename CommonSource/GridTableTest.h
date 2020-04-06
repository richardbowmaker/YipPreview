/*
 * GridTableTest.h
 *
 *  Created on: 16 Feb 2020
 *      Author: richard
 */

#ifndef COMMON_GRIDTABLETEST_H_
#define COMMON_GRIDTABLETEST_H_

#include <string>
#include <wx/grid.h>
#include <wx/wx.h>
#include <vector>

class GridTableTest : public wxGridTableBase
{
public:
	GridTableTest();
	virtual ~GridTableTest();

	virtual int GetNumberRows();
	virtual int GetNumberCols();

	virtual wxString GetValue(int row, int col);
	virtual void SetValue (int row, int col, const wxString &value);

	virtual wxString GetColLabelValue (int col);


	void insertAt(const int row);
	void deleteAt(const int row);
	void append();

	std::string toString();

private:

	std::string nextVal() const;

	static int no_;
	std::vector<std::string> vals_;

};

#endif /* COMMON_GRIDTABLETEST_H_ */
