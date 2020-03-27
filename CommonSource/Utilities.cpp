/*
 * Utilities.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#include "Utilities.h"

#ifdef WINDOWS_BUILD
	#include <algorithm>
	#include <iostream>
	#include <regex>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string>
	#include <string.h>
	#include <time.h>
	#include <utility>
	#include <windows.h>
#elif LINUX_BUILD
	#include <algorithm>
	#include <dirent.h>
	#include <fnmatch.h>
	#include <fstream>
	#include <iostream>
	#include <regex>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string>
	#include <string.h>
	#include <sys/syscall.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <time.h>
	#include <unistd.h>
	#include <utility>
#endif

#include <chrono>
#include <thread>
#include <wx/wx.h>


#include "Constants.h"
#include "Logger.h"
#include "ShellExecute.h"

bool Utilities::srand_{false};

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

int Utilities::pageDown(const int total, const int top, const int visible)
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

int Utilities::pageUp(const int total, const int top, const int visible)
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


int Utilities::getRand(const int min, const int max)
{
	if (!srand_)
	{
		srand((unsigned)time(NULL));
		srand_ = true;
	}
	return min + (rand() % (max - min + 1));
}

int Utilities::messageBox(
		const wchar_t *format,
		const wchar_t *caption,
		const int style 	   /*= wxOK | wxCENTRE*/,
		wxWindow * parent 	   /*= NULL*/,
		...)
{
	// format message
	va_list vl;
	va_start(vl, format);
	wchar_t buf[4000];
	vswprintf(buf, sizeof(buf) / sizeof(wchar_t), format, vl);

	std::wstring c = Constants::title + std::wstring(L" - ") + std::wstring(caption);
	return wxMessageBox(buf, c.c_str(), style,  parent);

}

void Utilities::delay(int ms)
{
	std::this_thread::sleep_for (std::chrono::milliseconds(ms));
}


//--------------------------------------------------
// String utilities
//--------------------------------------------------

std::mutex SudoMode::lock_;
bool SudoMode::active_ = false;
int SudoMode::refs_ = 0;
int SudoMode::uid_ = 0;

SudoMode::SudoMode() : got_(false)
{
#if LINUX_BUILD
	raise();
#endif
}

SudoMode::~SudoMode()
{
#if LINUX_BUILD
	lower();
#endif
}

void SudoMode::raise()
{
#if LINUX_BUILD
	if (!active_) return;
	if (got_) return;

	const std::lock_guard<std::mutex> lock(lock_);
	if (refs_ == 0)
	{
		int r = seteuid(0);
		if (r == 0)
		{
			Logger::info(L"Sudo mode entered OK");
			refs_++;
			got_ = true;
		}
		else
			Logger::systemError(errno, L"Sudo mode enter error");
	}
	else
	{
		refs_++;
		got_ = true;
		Logger::info(L"Sudo mode raise, refs %d", refs_);
	}
#endif
}

void SudoMode::lower()
{
#if LINUX_BUILD
	if (!active_) return;
	if (!got_) return;

	const std::lock_guard<std::mutex> lock(lock_);
	if (refs_ == 1)
	{
		int r = seteuid(uid_);
		if (r == 0)
		{
			Logger::info(L"Sudo mode exited OK");
			refs_ = 0;
			got_ = false;
		}
		else
			Logger::systemError(errno, L"Sudo mode lower error");
	}
	else if (refs_ > 1)
	{
		refs_--;
		Logger::info(L"Sudo mode lower, refs %d", refs_);
	}
#endif
}

// must be called at program start whilst still in sudo mode
// if program is run in user mode, then the sudo mode class does nothing
void SudoMode::initialise(const int uid)
{
	if (inSudoMode())
	{
		// we are in sudo mode, lower privileges
		active_ = true;
		uid_ = uid;
		seteuid(uid);
	}
}

bool SudoMode::inSudoMode()
{
	return geteuid() == 0;
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

bool SU::startsWith(const wchar_t* str, const wchar_t* prefix)
{
	while (*str != 0 && *prefix != 0)
		if (*str++ != *prefix++) return false;
	return *prefix == 0;
}

bool SU::startsWith(const std::wstring str, const std::wstring prefix)
{
	return startsWith(str.c_str(), prefix.c_str());
}

//--------------------------------------------------
// File utilities
//--------------------------------------------------

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
	StringCollT *files,
	const std::wstring *filter,
	const std::wstring *regex,
	StringCollT *dirs,
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
					files->emplace_back(directory + std::wstring(L"\\") + std::wstring(data.cFileName));
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
	{
		Logger::systemError(errno, L"FU::findFiles opendir");
		return false;
	}

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
				{
					files->push_back(directory + std::wstring(L"/") + SU::strToWStr(ent->d_name));
				}
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
	StringCollT &files,
	const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, nullptr, nullptr, sort);
}

bool FU::findMatchingFiles(
		const std::wstring directory,
		StringCollT &files,
		const std::wstring filter,
		const bool sort)
{
	return FU::findFiles(directory, &files, &filter, nullptr, nullptr, sort);
}

bool FU::findFilesDirs(
		const std::wstring directory,
		StringCollT &files,
		StringCollT &dirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, nullptr, &dirs, sort);
}

bool FU::findMatchingFilesDirs(
		const std::wstring directory,
		StringCollT &files,
		const std::wstring filter,
		StringCollT &dirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, &filter, nullptr, &dirs, sort);
}

bool FU::findMatchingFilesRex(
		const std::wstring directory,
		StringCollT &files,
		const std::wstring regex,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, &regex, nullptr, sort);
}

bool FU::findMatchingFilesDirsRex(
		const std::wstring directory,
		StringCollT &files,
		const std::wstring regex,
		StringCollT &dirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, &regex, &dirs, sort);
}

std::wstring FU::getFileStem(const std::wstring path)
{
	std::size_t m = path.find_last_of(FU::getPathSeparator());
	std::size_t n = path.find_last_of(L".");

	if (m == std::string::npos) m = 0; else ++m; // m points after separator
	if (n == std::string::npos || n < m) n = path.size();

	return path.substr(m, n - m);
}

std::wstring FU::getPathNoExt(const std::wstring path)
{
	std::size_t m = path.find_last_of(FU::getPathSeparator());
	std::size_t n = path.find_last_of(L".");

	if (m == std::string::npos) m = 0;
	if (n == std::string::npos || n < m)
		return path;
	else
		return path.substr(0, n);
}

std::wstring FU::getExt(const std::wstring path)
{
	std::size_t m = path.find_last_of(FU::getPathSeparator());
	std::size_t n = path.find_last_of(L".");

	if (m == std::string::npos) m = 0;
	if (n == std::string::npos || n < m)
		return L"";
	else
		return path.substr(n + 1, path.size() - n - 1);
}

std::wstring FU::getPathSeparator()
{
#ifdef WINDOWS_BUILD
	return L"\\";
#elif LINUX_BUILD
	return L"/";
#endif

}

// convert file path to window or linux build version
std::wstring FU::pathToLocal(const wchar_t* path)
{
	constexpr const wchar_t* sW = LR"(D:\)";
	constexpr const wchar_t* sL = LR"(/media/nas_share/Top/Data/)";
	std::wstring p(path);

#ifdef WINDOWS_BUILD
	// substitute linux absolute path with windows
	if (SU::startsWith(p.c_str(), sL))
		p = std::wstring(sW) + p.substr(wcslen(sL));
#elif LINUX_BUILD
	// substitute windows absolute path with linux
	if (SU::startsWith(p.c_str(), sW))
		p = std::wstring(sL) + p.substr(wcslen(sW));
#endif

	// convert path separator
	for (auto it = p.begin(); it != p.end(); ++it)
	{
#ifdef WINDOWS_BUILD
		if (*it == L'/') *it = '\\';
#elif LINUX_BUILD
		if (*it == L'\\') *it = '/';
#endif
	}

#ifdef WINDOWS_BUILD
	// convert relative path to absolute
	if (SU::startsWith(p.c_str(), LR"(\YipPreview)"))
		p = std::wstring(LR"(D:\Projects\WxWidgets)") + p;
#elif LINUX_BUILD
	if (SU::startsWith(p.c_str(), L"/YipPreview"))
		p = std::wstring(LR"(/media/nas_share/Top/Data/Projects/WxWidgets)") + p;
#endif

	return p;
}

// abbreviate filename to /abc/de ... xyz/filename
std::wstring FU::abbreviateFilename(const std::wstring &file, const int max)
{
	// return whole thing if less than max characters
	int len = file.size();
	if (len <= max) return file;

	// if no path in file name show the whole file id
	std::size_t m = file.find_last_of(FU::getPathSeparator());
	if (m == std::string::npos) return file;

	// length of filename part
	int l = file.size() - m;
	if (l > max)
	{
		// if file name part is greater than max characters
		// show the whole file name
		return file.substr(m + 1);
	}

	// chars less file name and elipsis part
	int n = max - l - 3;
	return file.substr(0, (n/2) + (n%2)) + std::wstring(L"...") + file.substr(m - (n/2));
}

// requires sudo mode
bool FU::mkDir(const std::wstring dir)
{
#ifdef WINDOWS_BUILD
	TODO
#elif LINUX_BUILD

	int status = mkdir(SU::wStrToStr(dir).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

	if (status == 0)
	{
		Logger::info(L"FU::mkDir() created directory %ls", dir.c_str());
		return true;
	}
	else
	{
		Logger::systemError(errno, L"FU::mkDir() error creating directory %ls", dir.c_str());
		return false;
	}
#endif
}

// requires sudo mode
bool FU::rmDir(const std::wstring dir)
{
#ifdef WINDOWS_BUILD
	TODO
#elif LINUX_BUILD

	int status = rmdir(SU::wStrToStr(dir).c_str());

	if (status == 0)
	{
		Logger::info(L"FU::rmDir() removed directory %ls", dir.c_str());
		return true;
	}
	else
	{
		Logger::systemError(errno, L"FU::rmDir() error removing directory %ls", dir.c_str());
		return false;
	}
#endif
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

bool Duration::parse(const std::wstring &str)
{
	return parse(SU::wStrToStr(str));
}

bool Duration::parse(const std::string &str)
{
    setMs(0);
    if (str.size() == 0) return true;
    const std::regex rex(R"(((\d{1,2}):)?(\d{1,2}):(\d{1,2})(\.(\d{1,3}(\s)?))?)");
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
    	Logger::error(L"Duration::parse() invalid duration string: %ls",
    			SU::strToWStr(str).c_str());
    	return false;
    }
}

std::wstring Duration::toString() const
{
	wchar_t buf[100];
	swprintf(buf, sizeof(buf) / sizeof(wchar_t),
			L"%02d:%02d:%02d.%03d", hh_, mm_, ss_, ms_);
	return std::wstring(buf);
}

bool Duration::test()
{
	Duration d;
	std::wstring s;
	bool b;
	bool result = true;

	s = d.toString();
	result &= Logger::test(s.compare(L"00:00:00.000") == 0, L"Duration::test() t1 failed");

	d.setMs( ((((12 * 60) + 34) * 60) + 56) * 1000 + 789 );
	s = d.toString();
	result &= Logger::test(s.compare(L"12:34:56.789") == 0, L"Duration::test() t2 failed");

	b = d.parse(L"12:34");
	result &= Logger::test(b, L"Duration::test() t3 failed");
	result &= Logger::test((d.hh_ == 0 ), L"Duration::test() t4 failed");
	result &= Logger::test((d.mm_ == 12), L"Duration::test() t5 failed");
	result &= Logger::test((d.ss_ == 34), L"Duration::test() t6 failed");
	result &= Logger::test((d.ms_ == 0 ), L"Duration::test() t7 failed");

	b = d.parse(L"12:34:56");
	result &= Logger::test(b, L"Duration::test() t8 failed");
	result &= Logger::test((d.hh_ == 12), L"Duration::test() t9 failed");
	result &= Logger::test((d.mm_ == 34), L"Duration::test() t10 failed");
	result &= Logger::test((d.ss_ == 56), L"Duration::test() t11 failed");
	result &= Logger::test((d.ms_ == 0 ), L"Duration::test() t12 failed");

	b = d.parse(L"12:34:56.789");
	result &= Logger::test(b, L"Duration::test() t13 failed");
	result &= Logger::test((d.hh_ == 12 ), L"Duration::test() t14 failed");
	result &= Logger::test((d.mm_ == 34 ), L"Duration::test() t15 failed");
	result &= Logger::test((d.ss_ == 56 ), L"Duration::test() t16 failed");
	result &= Logger::test((d.ms_ == 789), L"Duration::test() t17 failed");

	b = d.parse(L"1:2:3.4");
	result &= Logger::test(b, L"Duration::test() t18 failed");
	result &= Logger::test((d.hh_ == 1), L"Duration::test() t19 failed");
	result &= Logger::test((d.mm_ == 2), L"Duration::test() t20 failed");
	result &= Logger::test((d.ss_ == 3), L"Duration::test() t21 failed");
	result &= Logger::test((d.ms_ == 4), L"Duration::test() t22 failed");

	b = d.parse(L"12:34:56.78 ");
	result &= Logger::test(b, L"Duration::test() t23 failed");
	result &= Logger::test((d.hh_ == 12), L"Duration::test() t24 failed");
	result &= Logger::test((d.mm_ == 34), L"Duration::test() t25 failed");
	result &= Logger::test((d.ss_ == 56), L"Duration::test() t26 failed");
	result &= Logger::test((d.ms_ == 78), L"Duration::test() t27 failed");

	b = d.parse(L"x:y:z");
	result &= Logger::test(!b, L"Duration::test() t28 failed");

	if (result) Logger::info(L"Duration::test() all tests passed");

	return result;
}



//bool SU::parseDuration(const std::wstring &str, DurationT &duration)
//{
//	return parseDuration(SU::wStrToStr(str), duration);
//}
//
//bool SU::parseDuration(const std::string &str, DurationT &duration)
//{
//    memset(reinterpret_cast<void *>(&duration), 0, sizeof(duration));
//    const std::regex rex(R"(((\d{1,2}):)?(\d{1,2}):(\d{1,2})(\.(\d{1,3}))?)");
//    std::smatch m;
//    if (std::regex_search(str, m, rex))
//    {
//    	duration.hh = atoi(m[1].str().c_str());
//    	duration.mm = atoi(m[3].str().c_str());
//    	duration.ss = atoi(m[4].str().c_str());
//    	duration.ms = atoi(m[6].str().c_str());
//    	return true;
//    }
//    else
//    	return false;
//}
//
//std::wstring SU::durationToString(const DurationT &duration)
//{
//	wchar_t buf[100];
//	swprintf(buf, sizeof(buf) / sizeof(wchar_t),
//			L"%02d:%02d:%02d.%03d",
//			duration.hh,
//			duration.mm,
//			duration.ss,
//			duration.ms);
//	return std::wstring(buf);
//}
//
//std::wstring SU::durationToString(const long &durationMs)
//{
//	DurationT d;
//	long ld = durationMs;
//	d.ms = ld % 1000; ld /= 1000;
//	d.ss = ld % 60; ld /= 60;
//	d.mm = ld % 60; ld /= 60;
//	d.hh = ld % 60; ld /= 60;
//	return durationToString(d);
//}
//



