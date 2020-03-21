/*
 * GridTable.cpp
 *
 *  Created on: 13 Feb 2020
 *      Author: richard
 */

#include "GridTable.h"

#include "FileSet.h"
#include "FileSetManager.h"
#include "Logger.h"

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
	return 5;
}

wxString GridTable::GetValue(int row, int col)
{
	if (row >= FileSetManager::getNoOfFileSets() || row < 0 || col < 0 || col > 5)
	{
		Logger::error(L"GridTable::GetValue invalid row %d, col %d", row, col);
		return L"";
	}

	FileSetT fs = FileSetManager::getFileSet(row);
	
	switch (col)
	{
	case 0: return fs->getShortName();
	case 1: return fs->typesToString();
	case 2: return fs->getDurationStr();
	case 3: return fs->getLastTime();
	case 4: return fs->getTimes();
	}
	return L"";
}

void GridTable::SetValue(int row, int col, const wxString& value)
{
}

wxString GridTable::GetColLabelValue(int col)
{
	switch (col)
	{
	case 0: return L"File";
	case 1: return L"Type";
	case 2: return L"Duration";
	case 3: return L"Last";
	case 4: return L"Times";
	default: return L"";
	}
}

