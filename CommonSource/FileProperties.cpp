/*
 * FileProperties.cpp
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#include "FileProperties.h"

#include <cwchar>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "Logger.h"

void FileProperties::setString(const std::wstring& property,
		const std::wstring& value)
{
	if (value.size() == 0)
		// if property value not set then it need not be cached
		remove(property);
	else
		properties_[property] = value;
}

void FileProperties::setInt(const std::wstring& property, const int value)
{
	wchar_t buf[100];
	swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%d", value);
	setString(property, buf);
}

void FileProperties::setFloat(const std::wstring& property, const float value)
{
	wchar_t buf[100];
	swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%f", value);
	setString(property, buf);
}

void FileProperties::incCount(const std::wstring& property)
{
	int c = getInt(property);
	setInt(property, ++c);
}

void FileProperties::setDateTimeNow(const std::wstring& property)
{
	time_t rawtime;
	struct tm *timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);

	wchar_t buf[100];
	wcsftime(buf, sizeof(buf) / sizeof(wchar_t), L"%H:%M:%S %d/%m/%Y", timeinfo);
	setString(property, buf);
}

std::wstring FileProperties::getString(const std::wstring& property) const
{
	auto n = properties_.find(property);
	if (n != properties_.end())
	{
		return (*n).second;
	}
	else
		return L"";
}

int FileProperties::getInt(const std::wstring& property) const
{
	std::wstring s = getString(property);
	if (s.size() > 0)
	{
		wchar_t* p;
		return wcstol(s.c_str(), &p, 10);
	}
	else return 0;
}

float FileProperties::getFloat(const std::wstring& property) const
{
	std::wstring s = getString(property);
	if (s.size() > 0)
	{
		wchar_t* p;
		return wcstod(s.c_str(), &p);
	}
	else return 0.0f;
}

int FileProperties::getCount(const std::wstring& property) const
{
	return getInt(property);
}

std::wstring FileProperties::toString() const
{
// old - \Files\All\file20110102030031;times;2;lasttime;28/12/2019;maxvol;-2.5;duration;9:28
// new - a01;averagevol;2.20;duration;03:25:45.678;lasttime;15:52:59 28/03/2020;maxvol;1.10;selected;X;times;2

	if (properties_.size() == 0) return L"";

	std::wstring s;
	std::wstring d(L";");
	for (auto v : properties_)
		s += d + v.first + d + v.second;
	return s.substr(1);


//	std::wstring d(L";");
//	auto tocsv = [&d](const std::wstring &s, const PropertyT &p)
//		{ return s + d + p.first + d + p.second; };
//
//	// remove leading comma
//	return std::accumulate(properties_.begin(), properties_.end(),
//		std::wstring(L""),
//		tocsv).substr(1);
}

void FileProperties::fromString(const std::wstring& s)
{
	clear();
	wchar_t d(L';');
	std::size_t n1 = -1, n2 = 0, n3 = 0;
	std::wstring p, v;


	auto fixdate = [](std::wstring &p, std::wstring &v)
		{
			if (p.compare(L"lasttime") == 0)
			{
				if (v.size() == 20)
					v =  v.substr(0,19);
			}
		};


	while (true)
	{
		n2 = s.find(d, n1 + 1);
		if (n2 != std::wstring::npos)
		{
			n3 = s.find(d, n2 + 1);
			if (n3 == std::wstring::npos)
			{
				p = s.substr(n1 + 1, n2 - n1 - 1);
				v = s.substr(n2 + 1, s.size() - n2 - 1);
				fixdate(p, v);
				setString(p, v);
				break;
			}
			else
			{
				p = s.substr(n1 + 1, n2 - n1 - 1);
				v = s.substr(n2 + 1, n3 - n2 - 1);
				fixdate(p, v);
				setString(p, v);
				n1 = n3;
			}
		}
		else break;
	}
}

void FileProperties::clear()
{
	properties_.clear();
}

int FileProperties::getSize() const
{
	return properties_.size();
}

void FileProperties::remove(const std::wstring property)
{
	auto n = properties_.find(property);
	if (n != properties_.end()) properties_.erase(n);
}

void FileProperties::toLogger() const
{
	Logger::info(L"File Properties");
	for (auto p : properties_)
		Logger::info(L"\t%ls %ls", p.first.c_str(), p.second.c_str());
}

bool FileProperties::test()
{
	FileProperties fp;
	std::wstring s;
	bool result = true;

	fp.setString(L"p1", L"v1");
	fp.setInt(L"p2", 2);
	fp.incCount(L"p3");
	fp.incCount(L"p3");
	fp.incCount(L"p3");
//	fp.setDateTimeNow(L"p4");
	s = fp.toString();
	result &= Logger::test(s.compare(L"p1;v1;p2;2;p3;3") == 0, L"FileProperties::test() t1 failed");

	fp.clear();
	s = fp.toString();
	result &= Logger::test(s.compare(L"") == 0, L"FileProperties::test() t2 failed");

	fp.fromString(L"p1;v1;p2;10");
	s = fp.toString();
	result &= Logger::test(s.compare(L"p1;v1;p2;10") == 0, L"FileProperties::test() t3 failed");

	int c = fp.getSize();
	result &= Logger::test(c == 2, L"FileProperties::test() t4 failed");

	s = fp.getString(L"p1");
	result &= Logger::test(s.compare(L"v1") == 0, L"FileProperties::test() t5 failed");

	c = fp.getInt(L"p2");
	result &= Logger::test(c == 10, L"FileProperties::test() t6 failed");

	c = fp.getCount(L"p2");
	result &= Logger::test(c == 10, L"FileProperties::test() t7 failed");

	fp.remove(L"p1");
	c = fp.getSize();
	result &= Logger::test(c == 1, L"FileProperties::test() t8 failed");

	s = fp.toString();
	result &= Logger::test(s.compare(L"p2;10") == 0, L"FileProperties::test() t9 failed");

	fp.setString(L"p2", L"");
	c = fp.getSize();
	result &= Logger::test(c == 0, L"FileProperties::test() t10 failed");

	s = fp.toString();
	result &= Logger::test(s.compare(L"") == 0, L"FileProperties::test() t11 failed");

	fp.fromString(L"");
	s = fp.toString();
	result &= Logger::test(s.compare(L"") == 0, L"FileProperties::test() t12 failed");

	fp.fromString(L"p1;v1;p2");
	s = fp.toString();
	result &= Logger::test(s.compare(L"p1;v1") == 0, L"FileProperties::test() t13 failed");

	fp.fromString(L"p1;v1;p2;2;");
	s = fp.toString();
	Logger::test(s.compare(L"p1;v1;p2;2") == 0, L"FileProperties::test() t14 failed");

	fp.fromString(L"p1;v1;p2;;");
	s = fp.toString();
	result &= Logger::test(s.compare(L"p1;v1") == 0, L"FileProperties::test() t15 failed");

	fp.fromString(L"p1;v1;p2;");
	s = fp.toString();
	result &= Logger::test(s.compare(L"p1;v1") == 0, L"FileProperties::test() t16 failed");

	if (result)
		Logger::info(L"FileProperties::test() passed");

	return result;
}







