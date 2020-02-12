/*
 * Utilities.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#include "Utilities.h"

#ifdef WINDOWS_BUILD
	#include <iostream>
	#include <regex>
	#include <stdio.h>
	#include <string.h>
	#include <utility>
	#include <windows.h>
#elif LINUX_BUILD
	#include <dirent.h>
	#include <fnmatch.h>
	#include <fstream>
	#include <iostream>
	#include <regex>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/syscall.h>
	#include <sys/stat.h>
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
	std::wstring wc(l + 1, L' ');
	std::mbstowcs( &wc[0], str, l);
	return wc;
}

std::string SU::wStrToStr(const wchar_t* str, int len /*= 0*/)
{
	std::size_t l = (len == 0 ? wcslen(str) : static_cast<std::size_t>(len));
	std::string sc(l + 1, L' ');
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

bool FU::deleteFile(const std::wstring file)
{
#ifdef WINDOWS_BUILD
	if (DeleteFile(file.c_str()))
	{
		Logger::info(L"file deleted: %ls", file.c_str());
		return true;
	}
	else
	{
		Logger::systemError(GetLastError(), L"delete file error : %ls", file.c_str());
		return false;
	}
#elif LINUX_BUILD
	if (remove(SU::wStrToStr(file).c_str()) == 0)
	{
		Logger::info(L"file deleted: %ls", file.c_str());
		return true;
	}
	else
	{
		Logger::systemError(errno, L"delete file error : %ls", file.c_str());
		return false;
	}
#endif
}

bool FU::fileExists(const std::wstring file)
{
#ifdef WINDOWS_BUILD
	DWORD res = GetFileAttributes(file.c_str());
	if (res == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		return false;
	else
		return true;
#elif LINUX_BUILD
	struct stat buffer;
	return stat(SU::wStrToStr(file).c_str(), &buffer) == 0;
#endif
}

bool FU::copyFile(const std::wstring src, const std::wstring dest, const bool overwrite /*= true*/)
{
#ifdef WINDOWS_BUILD
	if (CopyFile(src.c_str(), dest.c_str(), overwrite ? FALSE : TRUE))
	{
		Logger::info(L"file copied: %ls to % ls", src.c_str(), dest.c_str());
		return true;
	}
	else
	{
		Logger::systemError(GetLastError(), L"copy file error: %ls to % ls", src.c_str(), dest.c_str());
		return false;
	}
#elif LINUX_BUILD
	if (!fileExists(src))
	{
		Logger::error(L"copy file, no source file: %ls to % ls", src.c_str(), dest.c_str());
		return false;
	}
	if (!overwrite && FU::fileExists(dest))
	{
		Logger::error(L"copy file already exists error, no overwrite: %ls to % ls", src.c_str(), dest.c_str());
		return false;
	}
	std::ifstream srcf;
	std::ofstream dstf;
	srcf.open(SU::wStrToStr(src).c_str(), std::ios::in | std::ios::binary);
	dstf.open(SU::wStrToStr(dest).c_str(), std::ios::out | std::ios::binary);
	dstf << srcf.rdbuf();
	bool res = srcf && dstf;
	srcf.close();
	dstf.close();
	if (res)
		Logger::info(L"file copied: %ls to % ls", src.c_str(), dest.c_str());
	else
		Logger::systemError(errno, L"copy file error: %ls to % ls", src.c_str(), dest.c_str());
	return res;
#endif
}

bool FU::moveFile(const std::wstring src, const std::wstring dest, const bool overwrite /*= true*/)
{
	bool exists = FU::fileExists(dest);

	if (!overwrite && exists)
	{
		Logger::error(L"move file, already exists error, no overwrite: %ls to % ls", src.c_str(), dest.c_str());
		return false;
	}
	if (overwrite && exists)
		// if overwrite, delete destination first provided the source exists
		if (FU::fileExists(src)) FU::deleteFile(dest);

#ifdef WINDOWS_BUILD
	if (MoveFile(src.c_str(), dest.c_str()))
	{
		Logger::info(L"file moved: %ls to % ls", src.c_str(), dest.c_str());
		return true;
	}
	else
	{
		Logger::systemError(GetLastError(), L"move file error: %ls to % ls", src.c_str(), dest.c_str());
		return false;
	}
#elif LINUX_BUILD
	if (rename(SU::wStrToStr(src).c_str(), SU::wStrToStr(dest).c_str()) == 0)
	{
		Logger::info(L"file moved: %ls to % ls", src.c_str(), dest.c_str());
		return true;
	}
	else
	{
		Logger::systemError(errno, L"move file error: %ls to % ls", src.c_str(), dest.c_str());
		return false;
	}
#endif
}

bool FU::findFiles(
	const std::wstring directory,
	StringsT *files,
	const std::wstring *filter,
	const std::wstring *regex,
	StringsT *dirs,
	const bool sort)
{
#ifdef WINDOWS_BUILD

	std::wstring dir;
	if (filter == nullptr)
		dir = directory + std::wstring(L"\\*");
	else
		dir = directory + std::wstring(L"\\") + *filter;

	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(dir.c_str(), &data);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	std::regex rex;
	if (regex != nullptr)
		rex = std::regex(SU::wStrToStr(*regex).c_str());

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			if (files != nullptr)
			{
				bool match = true;

				if (regex != nullptr)
				{
					match = std::regex_match(SU::wStrToStr(data.cFileName), rex);
				}
				if (match)
					files->emplace_back(data.cFileName);
			}
		}
		else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (dirs != nullptr)
				dirs->emplace_back(data.cFileName);
		}
	} while (FindNextFile(hFind, &data));
	FindClose(hFind);

#elif LINUX_BUILD

	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(SU::wStrToStr(directory).c_str())) == NULL)
		return false;

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

#endif

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

bool FU::findFiles(
	const std::wstring directory,
	StringsT &files,
	const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, nullptr, nullptr, sort);
}

bool FU::findMatchingFiles(
		const std::wstring directory,
		StringsT &files,
		const std::wstring filter,
		const bool sort)
{
	return FU::findFiles(directory, &files, &filter, nullptr, nullptr, sort);
}

bool FU::findFilesDirs(
		const std::wstring directory,
		StringsT &files,
		StringsT &dirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, nullptr, &dirs, sort);
}

bool FU::findMatchingFilesDirs(
		const std::wstring directory,
		StringsT &files,
		const std::wstring filter,
		StringsT &dirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, &filter, nullptr, &dirs, sort);
}

bool FU::findMatchingFilesRex(
		const std::wstring directory,
		StringsT &files,
		const std::wstring regex,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, &regex, nullptr, sort);
}

bool FU::findMatchingFilesDirsRex(
		const std::wstring directory,
		StringsT &files,
		const std::wstring regex,
		StringsT &dirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, &regex, &dirs, sort);
}

