/*
 * Properties.cpp
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#include "Properties.h"

#include <cwchar>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "Logger.h"

void Properties::setString(const std::string& property,
		const std::string& value)
{
	if (value.size() == 0)
		// if property value not set then it need not be cached
		remove(property);
	else
		properties_[property] = value;
}

void Properties::setInt(const std::string& property, const int value)
{
	char buf[100];
	snprintf(buf, sizeof(buf) / sizeof(char), "%d", value);
	setString(property, buf);
}

void Properties::setFloat(const std::string& property, const float value)
{
	char buf[100];
	snprintf(buf, sizeof(buf) / sizeof(char), "%f", value);
	setString(property, buf);
}

void Properties::incCount(const std::string& property)
{
	int c = getInt(property);
	setInt(property, ++c);
}

void Properties::setDateTimeNow(const std::string& property)
{
	time_t rawtime;
	struct tm *timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);

	char buf[100];
	strftime(buf, sizeof(buf) / sizeof(char), "%H:%M:%S %d/%m/%Y", timeinfo);
	setString(property, buf);
}

std::string Properties::getString(const std::string& property) const
{
	auto n = properties_.find(property);
	if (n != properties_.end())
	{
		return (*n).second;
	}
	else
		return "";
}

int Properties::getInt(const std::string& property) const
{
	std::string s = getString(property);
	if (s.size() > 0)
		return atoi(s.c_str());
	else return 0;
}

float Properties::getFloat(const std::string& property) const
{
	std::string s = getString(property);
	if (s.size() > 0)
		return atof(s.c_str());
	else return 0.0f;
}

int Properties::getCount(const std::string& property) const
{
	return getInt(property);
}

std::string Properties::toString() const
{
// old - \Files\All\file20110102030031;times;2;lasttime;28/12/2019;maxvol;-2.5;duration;9:28
// new - a01;averagevol;2.20;duration;03:25:45.678;lasttime;15:52:59 28/03/2020;maxvol;1.10;selected;X;times;2

	if (properties_.size() == 0) return "";

	std::string s;
	std::string d(";");
	for (auto v : properties_)
		s += d + v.first + d + v.second;
	return s.substr(1);


//	std::string d(";");
//	auto tocsv = [&d](const std::string &s, const PropertyT &p)
//		{ return s + d + p.first + d + p.second; };
//
//	// remove leading comma
//	return std::accumulate(properties_.begin(), properties_.end(),
//		std::string(""),
//		tocsv).substr(1);
}

void Properties::fromString(const std::string& s)
{
	clear();
	char d(';');
	std::size_t n1 = -1, n2 = 0, n3 = 0;
	std::string p, v;


	auto fixdate = [](std::string &p, std::string &v)
		{
			if (p.compare("lasttime") == 0)
			{
				if (v.size() == 20)
					v =  v.substr(0,19);
			}
		};


	while (true)
	{
		n2 = s.find(d, n1 + 1);
		if (n2 != std::string::npos)
		{
			n3 = s.find(d, n2 + 1);
			if (n3 == std::string::npos)
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

void Properties::clear()
{
	properties_.clear();
}

int Properties::getSize() const
{
	return properties_.size();
}

void Properties::remove(const std::string property)
{
	auto n = properties_.find(property);
	if (n != properties_.end()) properties_.erase(n);
}

void Properties::toLogger() const
{
	Logger::info("File Properties");
	for (auto p : properties_)
		Logger::info("\t{} {}", p.first, p.second);
}

bool Properties::test()
{
	Properties fp;
	std::string s;
	bool result = true;

	fp.setString("p1", "v1");
	fp.setInt("p2", 2);
	fp.incCount("p3");
	fp.incCount("p3");
	fp.incCount("p3");
//	fp.setDateTimeNow("p4");
	s = fp.toString();
	result &= Logger::test(s.compare("p1;v1;p2;2;p3;3") == 0, "Properties::test() t1 failed");

	fp.clear();
	s = fp.toString();
	result &= Logger::test(s.compare("") == 0, "Properties::test() t2 failed");

	fp.fromString("p1;v1;p2;10");
	s = fp.toString();
	result &= Logger::test(s.compare("p1;v1;p2;10") == 0, "Properties::test() t3 failed");

	int c = fp.getSize();
	result &= Logger::test(c == 2, "Properties::test() t4 failed");

	s = fp.getString("p1");
	result &= Logger::test(s.compare("v1") == 0, "Properties::test() t5 failed");

	c = fp.getInt("p2");
	result &= Logger::test(c == 10, "Properties::test() t6 failed");

	c = fp.getCount("p2");
	result &= Logger::test(c == 10, "Properties::test() t7 failed");

	fp.remove("p1");
	c = fp.getSize();
	result &= Logger::test(c == 1, "Properties::test() t8 failed");

	s = fp.toString();
	result &= Logger::test(s.compare("p2;10") == 0, "Properties::test() t9 failed");

	fp.setString("p2", "");
	c = fp.getSize();
	result &= Logger::test(c == 0, "Properties::test() t10 failed");

	s = fp.toString();
	result &= Logger::test(s.compare("") == 0, "Properties::test() t11 failed");

	fp.fromString("");
	s = fp.toString();
	result &= Logger::test(s.compare("") == 0, "Properties::test() t12 failed");

	fp.fromString("p1;v1;p2");
	s = fp.toString();
	result &= Logger::test(s.compare("p1;v1") == 0, "Properties::test() t13 failed");

	fp.fromString("p1;v1;p2;2;");
	s = fp.toString();
	Logger::test(s.compare("p1;v1;p2;2") == 0, "Properties::test() t14 failed");

	fp.fromString("p1;v1;p2;;");
	s = fp.toString();
	result &= Logger::test(s.compare("p1;v1") == 0, "Properties::test() t15 failed");

	fp.fromString("p1;v1;p2;");
	s = fp.toString();
	result &= Logger::test(s.compare("p1;v1") == 0, "Properties::test() t16 failed");

	if (result)
		Logger::info("Properties::test() passed");

	return result;
}







