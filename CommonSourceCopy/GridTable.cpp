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
	return static_cast<int>(ColT::NoOfCols);
}

wxString GridTable::GetValue(int row, int col)
{
	FileSetT fs = FileSetManager::getFileSet(row);
	
	switch (static_cast<ColT>(col))
	{
	case ColT::Mount:      return fs->getVolume()->getMount();
	case ColT::Volume:     return fs->getVolume()->getShortName();
	case ColT::File:       return fs->getId();
	case ColT::Type:       return fs->typesToString();
	case ColT::Selected:   return fs->getIsSelected();
	case ColT::Duration:   return fs->getDurationStr();
	case ColT::LastTime:   return fs->getLastTime();
	case ColT::Times:      return fs->getTimes();
	case ColT::MaxVol:     return fs->getMaxVolStr();
	case ColT::AverageVol: return fs->getAverageVolStr();
	}
	return L"";
}

void GridTable::SetValue(int row, int col, const wxString& value)
{
}

wxString GridTable::GetColLabelValue(int col)
{
	switch (static_cast<ColT>(col))
	{
	case ColT::Volume: 		return L"Volume";
	case ColT::Mount: 		return L"Mount";
	case ColT::File: 		return L"File";
	case ColT::Type: 		return L"Type";
	case ColT::Selected: 	return L"Sel.";
	case ColT::Duration: 	return L"Duration";
	case ColT::LastTime: 	return L"Last";
	case ColT::Times: 		return L"Times";
	case ColT::MaxVol: 		return L"Max. Vol";
	case ColT::AverageVol: 	return L"Av. Vol";
	default: return L"";
	}
}

