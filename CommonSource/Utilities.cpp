/*
 * Utilities.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#include "Utilities.h"

#ifdef WINDOWS_BUILD
	#include <stdio.h>
	#include <string.h>
	#include <utility>
	#include <windows.h>
#elif LINUX_BUILD
	#include <dirent.h>
	#include <fnmatch.h>
	#include <regex>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/syscall.h>
	#include <time.h>
	#include <unistd.h>
	#include <utility>
#endif

#include "Logger.h"

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
	return static_cast<long>(GetCurrentProcessId());
#elif LINUX_BUILD
	return static_cast<long>(getpid());
#endif
}

long Utilities::getMsCounter()
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

std::wstring SU::strToWStr(const char* str, int len /*= 0*/)
{
	std::size_t l = (len == 0 ? strlen(str) : static_cast<std::size_t>(len));
	std::wstring wc(l, L' ');
	std::mbstowcs( &wc[0], str, l);
	return wc;
}

std::string SU::wStrToStr(const wchar_t* str, int len /*= 0*/)
{
	std::size_t l = (len == 0 ? wcslen(str) : static_cast<std::size_t>(len));
	std::string sc(l, L' ');
	std::wcstombs( &sc[0], str, l);
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


bool FU::FindFiles(
	const std::wstring directory,
	StringsT *files,
	const std::wstring *filter,
	const std::wstring *regex,
	StringsT *dirs,
	const bool sort)
{
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(SU::wStrToStr(directory).c_str())) != NULL)
	{
		std::regex rex;
		if (regex != nullptr)
			rex = std::regex(SU::wStrToStr(*regex).c_str());

		std::string fstr;
		if (filter != nullptr)
			fstr = SU::wStrToStr(*filter);

		while ((ent = readdir(dir)) != NULL)
		{
			switch (ent->d_type)
			{
			case DT_REG:
			{
				if (files != nullptr)
				{
					bool match = true;

					if (regex != nullptr)
					{
						match = std::regex_match(std::string(ent->d_name), rex);
					}
					else if (filter != nullptr)
					{
						match = (fnmatch(fstr.c_str(), ent->d_name,
								FNM_NOESCAPE | FNM_PATHNAME | FNM_PERIOD) == 0);
					}

					if (match)
						files->push_back(SU::strToWStr(ent->d_name));
				}
				break;
			}
			case DT_LNK:
				break;
			case DT_DIR:
			{
				if (dirs != nullptr)
				{
					int len = strlen(ent->d_name);
					for (int i = 0; i < len; ++i)
					{
						if (ent->d_name[i] != '.')
						{
							dirs->push_back(SU::strToWStr(ent->d_name));
							break;
						}
					}
				}
				break;
			}
			}
		}
		closedir(dir);

		if (sort)
		{
			// sort the files and directories
			if (files != nullptr)
			{
				std::sort(files->begin(), files->end(),
						[](std::wstring &f1, std::wstring &f2)
						{ return f1.compare(f2) < 0; });
			}

			if (dirs != nullptr)
			{
				std::sort(dirs->begin(), dirs->end(),
						[](std::wstring &d1, std::wstring &d2)
						{ return d1.compare(d2) < 0; });
			}
		}
		return true;
	}
	else
		return false;
}

bool FU::FindFiles(
	const std::wstring directory,
	StringsT &files,
	const bool sort)
{
	return FU::FindFiles(directory, &files, nullptr, nullptr, nullptr, sort);
}

bool FU::FindMatchingFiles(
		const std::wstring directory,
		StringsT &files,
		const std::wstring filter,
		const bool sort)
{
	return FU::FindFiles(directory, &files, &filter, nullptr, nullptr, sort);
}

bool FU::FindFilesDirs(
		const std::wstring directory,
		StringsT &files,
		StringsT &dirs,
		const bool sort)
{
	return FU::FindFiles(directory, &files, nullptr, nullptr, &dirs, sort);
}

bool FU::FindMatchingFilesDirs(
		const std::wstring directory,
		StringsT &files,
		const std::wstring filter,
		StringsT &dirs,
		const bool sort)
{
	return FU::FindFiles(directory, &files, &filter, nullptr, &dirs, sort);
}

bool FU::FindMatchingFilesRex(
		const std::wstring directory,
		StringsT &files,
		const std::wstring regex,
		const bool sort)
{
	return FU::FindFiles(directory, &files, nullptr, &regex, nullptr, sort);
}

bool FU::FindMatchingFilesDirsRex(
		const std::wstring directory,
		StringsT &files,
		const std::wstring regex,
		StringsT &dirs,
		const bool sort)
{
	return FU::FindFiles(directory, &files, nullptr, &regex, &dirs, sort);
}

