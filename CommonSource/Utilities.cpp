/*
 * Utilities.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#include "Utilities.h"

#include <stdio.h>
#include <string.h>
#include <utility>

#ifdef WINDOWS_BUILD
#include <windows.h>
#elif LINUX_BUILD
	#include <sys/syscall.h>
	#include <time.h>
	#include <unistd.h>
#endif

long Utilities::getThreadId()
{
#ifdef WINDOWS_BUILD
	return static_cast<long>(GetCurrentThreadId());
#elif LINUX_BUILD
	return static_cast<long>(syscall(SYS_gettid));
#endif
}

long Utilities::getProcessId()
{
#ifdef WINDOWS_BUILD
	return static_cast<long>(GetCurrentThreadId());
#elif LINUX_BUILD
	return static_cast<long>(getpid());
#endif
}

long Utilities::getMsCounter()
{
#ifdef WINDOWS_BUILD
	return ??;
#elif LINUX_BUILD
	timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) != -1)
		return ts.tv_sec * 1000 +  ts.tv_nsec / 1000000;
	else
		return 0;
	return static_cast<long>(getpid());
#endif

}

std::wstring SU::strToWStr(const char* str)
{
	std::size_t len = strlen(str);
	std::wstring wc(len, L' ');
	std::mbstowcs( &wc[0], str, len );
	return wc;
}

std::string SU::wStrToStr(const wchar_t* str)
{
	std::size_t len = wcslen(str);
	std::string sc(len, L' ');
	std::wcstombs( &sc[0], str, len );
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

