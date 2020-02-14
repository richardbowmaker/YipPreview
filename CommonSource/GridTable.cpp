/*
 * GridTable.cpp
 *
 *  Created on: 13 Feb 2020
 *      Author: richard
 */

#include "GridTable.h"

#include "FileSet.h"
#include "FileSetManager.h"

GridTable::GridTable()
{
}

GridTable::~GridTable()
{
}

void GridTable::initialise()
{
	wxGridCellAttrProvider *ap = new wxGridCellAttrProvider();

	// all columns are read only
	wxGridCellAttr*  a = new wxGridCellAttr();
	a->SetReadOnly(true);
	for (int c = 0; c < GetNumberCols(); c++)
		ap->SetColAttr(a, c);

	SetAttrProvider(ap);
}

int GridTable::GetNumberRows()
{
	return FileSetManager::getNoOfFileSets();
}

int GridTable::GetNumberCols()
{
	return 2;
}

wxString GridTable::GetValue(int row, int col)
{
	FileSetT fs = FileSetManager::getFileSet(row);
	
	switch (col)
	{
	case 0: return fs->getId();
	case 1: return fs->typesToString();
	}
	
}

void GridTable::SetValue(int row, int col, const wxString& value)
{
}

wxString GridTable::GetColLabelValue(int col)
{
	switch (col)
	{
	case 0: return L"File";
	case 1: return L"Other";
	}
}

