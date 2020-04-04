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

int Utilities::messageBox_(const char* message, const char* caption, const int style, wxWindow* parent)
{
	std::string c;
	if (caption != nullptr)
		c = Constants::title + std::string(" - ") + std::string(caption);
	return wxMessageBox(message, c.c_str(), style, parent);
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
			Logger::info("Sudo mode entered OK");
			refs_++;
			got_ = true;
		}
		else
			Logger::systemError("Sudo mode enter error");
	}
	else
	{
		refs_++;
		got_ = true;
		Logger::info("Sudo mode raise, refs %d", refs_);
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
			Logger::info("Sudo mode exited OK");
			refs_ = 0;
			got_ = false;
		}
		else
			Logger::systemError("Sudo mode lower error");
	}
	else if (refs_ > 1)
	{
		refs_--;
		Logger::info("Sudo mode lower, refs %d", refs_);
	}
#endif
}

// must be called at program start whilst still in sudo mode
// if program is run in user mode, then the sudo mode class does nothing
void SudoMode::initialise(const int uid)
{
#if LINUX_BUILD
	if (inSudoMode())
	{
		// we are in sudo mode, lower privileges
		active_ = true;
		uid_ = uid;
		seteuid(uid);

		Logger::info("euid %d", geteuid());

	}
#endif
}

bool SudoMode::inSudoMode()
{
#ifdef WINDOWS_BUILD
	return true;
#elif LINUX_BUILD
	return geteuid() == 0;
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

//--------------------------------------------------
// File utilities
//--------------------------------------------------

bool FU::deleteFile(const std::string file)
{
#ifdef WINDOWS_BUILD
	if (DeleteFileA(file.c_str()))
	{
		Logger::info("file deleted: {}", file);
		return true;
	}
	else
	{
		Logger::systemError("delete file error : {}", file);
		return false;
	}
#elif LINUX_BUILD
	if (remove(file.c_str()) == 0)
	{
		Logger::info("file deleted: {}", file);
		return true;
	}
	else
	{
		Logger::systemError("delete file error : {}", file);
		return false;
	}
#endif
}

bool FU::fileExists(const std::string file)
{
#ifdef WINDOWS_BUILD
	DWORD res = GetFileAttributesA(file.c_str());
	if (res == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		return false;
	else
		return true;
#elif LINUX_BUILD
	struct stat buffer;
	return stat(file.c_str(), &buffer) == 0;
#endif
}

bool FU::copyFile(const std::string src, const std::string dest, const bool overwrite /*= true*/)
{
#ifdef WINDOWS_BUILD
	if (CopyFileA(src.c_str(), dest.c_str(), overwrite ? FALSE : TRUE))
	{
		Logger::info("file copied: {} to {}", src, dest);
		return true;
	}
	else
	{
		Logger::systemError("copy file error: {} to {}", src, dest);
		return false;
	}
#elif LINUX_BUILD
	if (!fileExists(src))
	{
		Logger::error("copy file, no source file: {} to {}", src, dest);
		return false;
	}
	if (!overwrite && FU::fileExists(dest))
	{
		Logger::error("copy file already exists error, no overwrite: {} to {}", src, dest);
		return false;
	}
	std::ifstream srcf;
	std::ofstream dstf;
	srcf.open(src.c_str(), std::ios::in | std::ios::binary);
	dstf.open(dest.c_str(), std::ios::out | std::ios::binary);
	dstf << srcf.rdbuf();
	bool res = srcf && dstf;
	srcf.close();
	dstf.close();
	if (res)
		Logger::info("file copied: {} to {}", src, dest);
	else
		Logger::systemError("copy file error: {} to {}", src, dest);
	return res;
#endif
}

bool FU::moveFile(const std::string src, const std::string dest, const bool overwrite /*= true*/)
{
	bool exists = FU::fileExists(dest);

	if (!overwrite && exists)
	{
		Logger::error("move file, already exists error, no overwrite: {} to {}", src, dest);
		return false;
	}
	if (overwrite && exists)
		// if overwrite, delete destination first provided the source exists
		if (FU::fileExists(src)) FU::deleteFile(dest);

#ifdef WINDOWS_BUILD
	if (MoveFileA(src.c_str(), dest.c_str()))
	{
		Logger::info("file moved: {} to {}", src, dest);
		return true;
	}
	else
	{
		Logger::systemError("move file error: {} to {}", src, dest);
		return false;
	}
#elif LINUX_BUILD
	if (rename(src, dest) == 0)
	{
		Logger::info("file moved: {} to {}", src, dest);
		return true;
	}
	else
	{
		Logger::systemError("move file error: {} to {}", src, dest);
		return false;
	}
#endif
}

bool FU::findFiles(
	const std::string directory,
	StringCollT *files,
	const std::string *filter,
	const std::string *regex,
	StringCollT *dirs,
	const bool subdirs,
	const bool sort)
{
#ifdef WINDOWS_BUILD

	std::string dir;
	if (filter == nullptr)
		dir = directory + std::string("\\*");
	else
		dir = directory + std::string("\\") + *filter;

	WIN32_FIND_DATAA data;
	HANDLE hFind = FindFirstFileA(dir.c_str(), &data);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	std::regex rex;
	if (regex != nullptr)
		rex = std::regex(*regex);

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			if (files != nullptr)
			{
				bool match = true;

				if (regex != nullptr)
					match = std::regex_match(data.cFileName, rex);
				if (match)
					files->emplace_back(directory + std::string("\\") + std::string(data.cFileName));
			}
		}
		else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (dirs != nullptr || subdirs)
			{
				std::string d{data.cFileName};
				if (dirs != nullptr) dirs->emplace_back(d);
				if (subdirs)
				{
					if (!FU::findFiles(directory + std::string(R"(/)") + d,
							files, filter, regex, dirs, subdirs, false))
						return false;
				}
			}
		}
	} while (FindNextFileA(hFind, &data));
	FindClose(hFind);

#elif LINUX_BUILD

	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(directory)) == NULL)
	{
		Logger::systemError("FU::findFiles opendir");
		return false;
	}

	std::regex rex;
	if (regex != nullptr)
		rex = std::regex(*regex);

	std::string fstr;
	if (filter != nullptr)
		fstr = *filter;

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
					files->push_back(directory + std::string("/") + std::string(ent->d_name));
				}
			}
			break;
		}
		case DT_LNK:
			break;
		case DT_DIR:
		{
			if (dirs != nullptr || subdirs)
			{
				std::string d{ent->d_name};
				if (d.compare(".") != 0 && d.compare("..") != 0)
				{
					if (dirs != nullptr) dirs->emplace_back(d);
					if (subdirs)
					{
						if (!FU::findFiles(directory + std::string(R"(/)") + d,
								files, filter, regex, dirs, subdirs, false))
							return false;
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
					[](std::string &f1, std::string &f2)
					{ return f1.compare(f2) < 0; });
		}

		if (dirs != nullptr)
		{
			std::sort(dirs->begin(), dirs->end(),
					[](std::string &d1, std::string &d2)
					{ return d1.compare(d2) < 0; });
		}
	}
	return true;
}

bool FU::findFiles(
	const std::string directory,
	StringCollT &files,
	const bool subdirs,
	const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, nullptr, nullptr, subdirs, sort);
}

bool FU::findMatchingFiles(
		const std::string directory,
		StringCollT &files,
		const std::string filter,
		const bool subdirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, &filter, nullptr, nullptr, subdirs, sort);
}

bool FU::findFilesDirs(
		const std::string directory,
		StringCollT &files,
		StringCollT &dirs,
		const bool subdirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, nullptr, &dirs, subdirs, sort);
}

bool FU::findMatchingFilesDirs(
		const std::string directory,
		StringCollT &files,
		const std::string filter,
		StringCollT &dirs,
		const bool subdirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, &filter, nullptr, &dirs, subdirs, sort);
}

bool FU::findMatchingFilesRex(
		const std::string directory,
		StringCollT &files,
		const std::string regex,
		const bool subdirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, &regex, nullptr, subdirs, sort);
}

bool FU::findMatchingFilesDirsRex(
		const std::string directory,
		StringCollT &files,
		const std::string regex,
		StringCollT &dirs,
		const bool subdirs,
		const bool sort)
{
	return FU::findFiles(directory, &files, nullptr, &regex, &dirs, subdirs, sort);
}

std::string FU::getFileStem(const std::string path)
{
	std::size_t m = path.find_last_of(FU::getPathSeparator());
	std::size_t n = path.find_last_of(".");

	if (m == std::string::npos) m = 0; else ++m; // m points after separator
	if (n == std::string::npos || n < m) n = path.size();

	return path.substr(m, n - m);
}

std::string FU::getPathNoExt(const std::string path)
{
	std::size_t m = path.find_last_of(FU::getPathSeparator());
	std::size_t n = path.find_last_of(".");

	if (m == std::string::npos) m = 0;
	if (n == std::string::npos || n < m)
		return path;
	else
		return path.substr(0, n);
}

std::string FU::getExt(const std::string path)
{
	std::size_t m = path.find_last_of(FU::getPathSeparator());
	std::size_t n = path.find_last_of(".");

	if (m == std::string::npos) m = 0;
	if (n == std::string::npos || n < m)
		return "";
	else
		return path.substr(n + 1, path.size() - n - 1);
}

std::string FU::getPathSeparator()
{
#ifdef WINDOWS_BUILD
	return "\\";
#elif LINUX_BUILD
	return "/";
#endif

}

// convert file path to window or linux build version
std::string FU::pathToLocal(const char* path)
{
	constexpr const char* sW = R"(D:\)";
	constexpr const char* sL = R"(/media/nas_share/Top/Data/)";
	std::string p(path);

#ifdef WINDOWS_BUILD
	// substitute linux absolute path with windows
	if (SU::startsWith(p.c_str(), sL))
		p = std::string(sW) + p.substr(strlen(sL));
#elif LINUX_BUILD
	// substitute windows absolute path with linux
	if (SU::startsWith(p.c_str(), sW))
		p = std::string(sL) + p.substr(strlen(sW));
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
	if (SU::startsWith(p.c_str(), R"(\YipPreview)"))
		p = std::string(R"(D:\Projects\WxWidgets)") + p;
#elif LINUX_BUILD
	if (SU::startsWith(p.c_str(), "/YipPreview"))
		p = std::string(R"(/media/nas_share/Top/Data/Projects/WxWidgets)") + p;
#endif

	return p;
}

// abbreviate filename to /abc/de ... xyz/filename
std::string FU::abbreviateFilename(const std::string &file, const int max)
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
	return file.substr(0, (n/2) + (n%2)) + std::string("...") + file.substr(m - (n/2));
}

// requires sudo mode
bool FU::mkDir(const std::string dir)
{
#ifdef WINDOWS_BUILD
	if (CreateDirectoryA(dir.c_str(), NULL))
	{
		Logger::info("FU::mkDir() created directory {}", dir);
		return true;
	}
	else
	{
		DWORD err = GetLastError();
		Logger::systemError("FU::mkDir() error creating directory {}", dir);
		return false;
	}
#elif LINUX_BUILD
	int status = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	if (status == 0)
	{
		Logger::info("FU::mkDir() created directory {}", dir);
		return true;
	}
	else
	{
		Logger::systemError("FU::mkDir() error creating directory {}", dir);
		return false;
	}
#endif
}

// requires sudo mode
bool FU::rmDir(const std::string dir)
{
#ifdef WINDOWS_BUILD
	if (RemoveDirectoryA(dir.c_str()))
	{
		Logger::info("FU::mkDir() removed directory {}", dir);
		return true;
	}
	else
	{
		DWORD err = GetLastError();
		Logger::systemError("FU::mkDir() error removing directory {}", dir);
		return false;
	}
#elif LINUX_BUILD

	int status = rmdir(dir.c_str());

	if (status == 0)
	{
		Logger::info("FU::rmDir() removed directory {}", dir);
		return true;
	}
	else
	{
		Logger::systemError("FU::rmDir() error removing directory {}", dir);
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



