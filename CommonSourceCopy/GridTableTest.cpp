/*
 * GridTableTest.cpp
 *
 *  Created on: 16 Feb 2020
 *      Author: richard
 */

#include "GridTableTest.h"

#include <stdio.h>
#include <stdlib.h>

int GridTableTest::no_ = 0;

GridTableTest::GridTableTest()
{
	for (int i = 0; i < 10; ++i) append();
}

GridTableTest::~GridTableTest()
{
}

int GridTableTest::GetNumberRows()
{
	return vals_.size();
}

int GridTableTest::GetNumberCols()
{
	return 1;
}

wxString GridTableTest::GetValue(int row, int col)
{
	if (row >= 0 && row < GetNumberRows())
		return vals_[row];
	else
		return L"";
}

void GridTableTest::SetValue(int row, int col, const wxString& value)
{
	if (row >= 0 && row < GetNumberRows())
		vals_[row] = value;
}

wxString GridTableTest::GetColLabelValue(int col)
{
	return L"Num";
}

void GridTableTest::insertAt(const int row)
{
	vals_.insert(vals_.begin() + row, nextVal());
}

void GridTableTest::deleteAt(const int row)
{
	vals_.erase(vals_.begin() + row);
}

void GridTableTest::append()
{
	vals_.push_back(nextVal());
}

std::string GridTableTest::GridTableTest::nextVal() const
{
	char buf[20];
	snprintf(buf, sizeof(buf) / sizeof(char), "Val %d", no_++);
	return std::string(buf);
}

std::string GridTableTest::toString()
{
	std::string s;
	for (auto v : vals_) s += v + std::string(" ");
	return s;
}


