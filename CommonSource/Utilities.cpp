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
	#include <unistd.h>
	#include <sys/syscall.h>
#endif


long Utilities::GetThreadID()
{
#ifdef WINDOWS_BUILD
	return static_cast<long>(GetCurrentThreadId());
#elif LINUX_BUILD
	return static_cast<long>(syscall(SYS_gettid));
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
