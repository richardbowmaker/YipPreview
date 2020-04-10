/*
 * Utilities.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#include "Utilities.h"

#include <algorithm>
#include <chrono>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <thread>
#include <time.h>
#include <utility>
#include <wx/wx.h>

#ifdef WINDOWS_BUILD
	#include <shlwapi.h>
	#include <windows.h>
#elif LINUX_BUILD
	#include <sys/syscall.h>
#endif

#include "Constants.h"
#include "Logger.h"
#include "ShellExecute.h"

bool US::srand_{false};

long US::getThreadId()
{
#ifdef WINDOWS_BUILD
	return static_cast<long>(GetCurrentThreadId());
#elif LINUX_BUILD
	return static_cast<long>(syscall(SYS_gettid));
#endif
}

long US::getProcessId()
{
#ifdef WINDOWS_BUILD
	return static_cast<long>(GetCurrentProcessId());
#elif LINUX_BUILD
	return static_cast<long>(getpid());
#endif
}

long US::getMsCounter()
{
#ifdef WINDOWS_BUILD
	return static_cast<long>(GetTickCount64());
#elif LINUX_BUILD
	timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) != -1)
		return ts.tv_sec * 1000 +  ts.tv_nsec / 1000000;
	else
		return 0;
	return static_cast<long>(getpid());
#endif
}

int US::pageDown(const int total, const int top, const int visible)
{
	int t = 0;
	if (top >= total - visible) t = 0;
	else
	{
		t = top + visible;
		if (t > total - visible) t = total - visible;
	}
	return t;
}

int US::pageUp(const int total, const int top, const int visible)
{
	int t = 0;
	if (top == 0) t = total - visible;
	else
	{
		t = top - visible;
		if (t < 0) t = 0;
	}
	return t;
}

int US::getRand(const int min, const int max)
{
	if (!srand_)
	{
		srand((unsigned)time(NULL));
		srand_ = true;
	}
	return min + (rand() % (max - min + 1));
}

int US::messageBox_(const char* message, const char* caption, const int style, wxWindow* parent)
{
	std::string c;
	if (caption != nullptr)
		c = Constants::title + std::string(" - ") + std::string(caption);
	return wxMessageBox(message, c.c_str(), style, parent);
}

std::string US::bytesToString(long long bytes)
{
	std::string s = "";

	long long kbs = bytes / 1024;
	long long mbs = kbs / 1024;
	long long gbs = mbs / 1024;

	if (gbs > 0)
		s = fmt::format("{}.{}GB", gbs, (mbs % 1024) / 102);
	else if (mbs > 0)
		s = fmt::format("{}.{}MB", mbs, (kbs % 1024) / 102);
	else if (kbs > 0)
		s = fmt::format("{}.{}kb", kbs, (bytes % 1024) / 102);
	else
		s = fmt::format("{} bytes", bytes);

	s += fmt::format(" ({})", bytes);

	return s;
}

void US::delay(int ms)
{
#ifdef WINDOWS_BUILD
	// std::this_thread::sleep_for (std::chrono::milliseconds(ms));
	Sleep(ms);
#elif LINUX_BUILD
	usleep(ms * 1000);
#endif
}

std::string US::getWorkingDirectory()
{
#ifdef WINDOWS_BUILD

	HMODULE hModule = GetModuleHandle(nullptr);
	if (hModule)
	{
		char path[MAX_PATH];
		GetModuleFileNameA(hModule, path, sizeof(path));
		std::string wd = std::string(path);
		return FU::getPath(wd); // strip of executable filename
	}
	else
	{
		Logger::systemError("US::getWorkingDirectory() GetModuleFileNameA() failed");
		return "";
	}

#elif LINUX_BUILD
	char path[PATH_MAX];
	if (getcwd(path, sizeof(path)) != nullptr)
		return std::string(path);
	else
	{
		Logger::systemError("US::getWorkingDirectory() getcwd() failed");
		return "";
	}
#endif
}


//--------------------------------------------------
// String utilities
//--------------------------------------------------

std::wstring SU::strToWStr(const char* str, int len /*= 0*/)
{
	std::size_t l = (len == 0 ? strlen(str) : static_cast<std::size_t>(len));
	std::wstring wc(l, 0);
	std::mbstowcs( &wc[0], str, l);
	return wc;
}

std::string SU::wStrToStr(const wchar_t* str, int len /*= 0*/)
{
	std::size_t l = (len == 0 ? wcslen(str) : static_cast<std::size_t>(len));
	std::string sc(l, 0);
	std::wcstombs(&sc[0], str, l);
	return sc;
}

std::wstring SU::strToWStr(const std::string str)
{
	return strToWStr(str.c_str());
}

std::string SU::wStrToStr(const std::wstring str)
{
	return wStrToStr(str.c_str());
}

bool SU::startsWith(const char* str, const char* prefix)
{
	while (*str != 0 && *prefix != 0)
		if (*str++ != *prefix++) return false;
	return *prefix == 0;
}

bool SU::startsWith(const std::string str, const std::string prefix)
{
	return startsWith(str.c_str(), prefix.c_str());
}

bool SU::endsWith(const char* str, const char* suffix)
{
	int sl = strlen(str);
	int el = strlen(suffix);

	if (el == 0) return true;

	const char* ps = str + sl;
	const char* pe = suffix + el;

	while (ps != str && pe != suffix)
		if (*(--ps) != *(--pe)) return false;
	return pe == suffix;
}

bool SU::endsWith(const std::string str, const std::string suffix)
{
	return endsWith(str.c_str(), suffix.c_str());
}

std::string SU::doubleQuotes(const std::string &s)
{
	return std::string("\"") + s + std::string("\"");
}

std::string SU::singleQuotes(const std::string &s)
{
	return std::string("'") + s + std::string("'");
}


//---------------------------------------------
// Duration
//---------------------------------------------

Duration::Duration() :
		hh_(0),
		mm_(0),
		ss_(0),
		ms_(0)
{
}

void Duration::setMs(const long ms)
{
	long d = ms;
	ms_ = d % 1000; d /= 1000;
	ss_ = d % 60;   d /= 60;
	mm_ = d % 60;   d /= 60;
	hh_ = d % 60;   d /= 60;
}

long Duration::getMs() const
{
	return ((((hh_ * 60) + mm_) * 60) + ss_) * 1000 + ms_;
}

bool Duration::parse(const std::string &str)
{
    setMs(0);
    if (str.size() == 0) return true;
    std::regex rex(R"(((\d{1,2}):)?(\d{1,2}):(\d{1,2})(\.(\d{1,3}(\s)?))?)");
    std::smatch m;
    if (std::regex_search(str, m, rex))
    {
    	hh_ = atoi(m[1].str().c_str());
    	mm_ = atoi(m[3].str().c_str());
    	ss_ = atoi(m[4].str().c_str());
    	ms_ = atoi(m[6].str().c_str());
    	return true;
    }
    else
    {
    	Logger::error("Duration::parse() invalid duration string: {}", str);
    	return false;
    }
}

std::string Duration::toString() const
{
	char buf[100];
	snprintf(buf, sizeof(buf) / sizeof(char),
			"%02d:%02d:%02d.%03d", hh_, mm_, ss_, ms_);
	return std::string(buf);
}

bool Duration::test()
{
	Duration d;
	std::string s;
	bool b;
	bool result = true;

	s = d.toString();
	result &= Logger::test(s.compare("00:00:00.000") == 0, "Duration::test() t1 failed");

	d.setMs( ((((12 * 60) + 34) * 60) + 56) * 1000 + 789 );
	s = d.toString();
	result &= Logger::test(s.compare("12:34:56.789") == 0, "Duration::test() t2 failed");

	b = d.parse("12:34");
	result &= Logger::test(b, "Duration::test() t3 failed");
	result &= Logger::test((d.hh_ == 0 ), "Duration::test() t4 failed");
	result &= Logger::test((d.mm_ == 12), "Duration::test() t5 failed");
	result &= Logger::test((d.ss_ == 34), "Duration::test() t6 failed");
	result &= Logger::test((d.ms_ == 0 ), "Duration::test() t7 failed");

	b = d.parse("12:34:56");
	result &= Logger::test(b, "Duration::test() t8 failed");
	result &= Logger::test((d.hh_ == 12), "Duration::test() t9 failed");
	result &= Logger::test((d.mm_ == 34), "Duration::test() t10 failed");
	result &= Logger::test((d.ss_ == 56), "Duration::test() t11 failed");
	result &= Logger::test((d.ms_ == 0 ), "Duration::test() t12 failed");

	b = d.parse("12:34:56.789");
	result &= Logger::test(b, "Duration::test() t13 failed");
	result &= Logger::test((d.hh_ == 12 ), "Duration::test() t14 failed");
	result &= Logger::test((d.mm_ == 34 ), "Duration::test() t15 failed");
	result &= Logger::test((d.ss_ == 56 ), "Duration::test() t16 failed");
	result &= Logger::test((d.ms_ == 789), "Duration::test() t17 failed");

	b = d.parse("1:2:3.4");
	result &= Logger::test(b, "Duration::test() t18 failed");
	result &= Logger::test((d.hh_ == 1), "Duration::test() t19 failed");
	result &= Logger::test((d.mm_ == 2), "Duration::test() t20 failed");
	result &= Logger::test((d.ss_ == 3), "Duration::test() t21 failed");
	result &= Logger::test((d.ms_ == 4), "Duration::test() t22 failed");

	b = d.parse("12:34:56.78 ");
	result &= Logger::test(b, "Duration::test() t23 failed");
	result &= Logger::test((d.hh_ == 12), "Duration::test() t24 failed");
	result &= Logger::test((d.mm_ == 34), "Duration::test() t25 failed");
	result &= Logger::test((d.ss_ == 56), "Duration::test() t26 failed");
	result &= Logger::test((d.ms_ == 78), "Duration::test() t27 failed");

	b = d.parse("x:y:z");
	result &= Logger::test(!b, "Duration::test() t28 failed");

	if (result) Logger::info("Duration::test() all tests passed");

	return result;
}



