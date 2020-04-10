/*
 * Utilities.cpp
 *
 *  Windows Version

 */

#include "Utilities.h"

#include <algorithm>
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <regex>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <thread>
#include <time.h>
#include <utility>
#include <windows.h>
#include <wx/wx.h>

#include "Constants.h"
#include "Logger.h"
#include "Main.h"
#include "ShellExecute.h"


//--------------------------------------------------
// File utilities
//--------------------------------------------------

bool FU::deleteFile(const std::string file)
{
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
}

bool FU::fileExists(const std::string file)
{
	DWORD res = GetFileAttributesA(file.c_str());
	if (res == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		return false;
	else
		return true;
}

bool FU::copyFile(const std::string src, const std::string dest, const bool overwrite /*= true*/)
{
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
}

long long FU::getFileSize(const std::string file)
{
	WIN32_FILE_ATTRIBUTE_DATA attr;
	if (GetFileAttributesExA(file.c_str(), GetFileExInfoStandard, &attr))
	{
		return (static_cast<long long>(attr.nFileSizeHigh) * static_cast<long long>(LONG_MAX)) +
			static_cast<long long>(attr.nFileSizeLow);
	}
	else
	{
		Logger::systemError("FU::getFileSize() GetFileAttributesExA() call failed for {}", file);
		return 0;
	}
}

bool FU::getVolumeFreeSpace(const std::string& volume, DiskSpaceT &space)
{
	DWORD sectorsPerCluster;
	DWORD bytesPerSector;
	DWORD numberOfFreeClusters;
	DWORD totalNumberOfClusters;

	if (GetDiskFreeSpaceA(
		volume.c_str(),
		&sectorsPerCluster,
		&bytesPerSector,
		&numberOfFreeClusters,
		&totalNumberOfClusters))
	{
		long long bytesPerCluster = static_cast<long long>(sectorsPerCluster)* static_cast<long long>(bytesPerSector);
		space.total = static_cast<long long>(totalNumberOfClusters)* bytesPerCluster;
		space.free = static_cast<long long>(numberOfFreeClusters)* bytesPerCluster;
		return true;
	}
	else
	{
		Logger::systemError("FU::getFreeSpace() GetDiskFreeSpaceA() failed");
		return false;
	}
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
	std::string dir = directory + Constants::pathSeparator + std::string("*");
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
				else if (filter != nullptr)
					match = (PathMatchSpecExA(data.cFileName, filter->c_str(), PMSF_NORMAL) == 0);

				if (match)
					files->emplace_back(directory + std::string("\\") + std::string(data.cFileName));
			}
		}
		else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (dirs != nullptr || subdirs)
			{
				std::string d{data.cFileName};
				if (d.compare(".") != 0 && d.compare("..") != 0)
				{
					if (dirs != nullptr) dirs->emplace_back(d);
					if (subdirs)
					{
						if (!FU::findFiles(directory + Constants::pathSeparator + d,
							files, filter, regex, dirs, subdirs, false))
							return false;
					}
				}
			}
		}
	} while (FindNextFileA(hFind, &data));
	FindClose(hFind);

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

std::string FU::getPath(const std::string path)
{
	std::size_t n = path.find_last_of(FU::getPathSeparator());

	if (n == std::string::npos)
		return "";
	else
		return path.substr(0, n);
}

std::string FU::getFileName(const std::string path)
{
	std::size_t n = path.find_last_of(FU::getPathSeparator());

	if (n == std::string::npos )
		return path;
	else
		return path.substr(n + 1, path.size() - n - 1);
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
	return "\\";

}

std::string FU::pathToOs(const std::string& path)
{
	return pathToOs(path.c_str());
}

// convert file path to window or linux build version
std::string FU::pathToOs(const char* path)
{
	constexpr const char* sW = R"(D:\)";
	constexpr const char* sL = R"(/media/nas_share/Top/Data/)";
	std::string p(path);

	// substitute linux absolute path with windows
	if (SU::startsWith(p.c_str(), sL))
		p = std::string(sW) + p.substr(strlen(sL));

	// convert path separator
	for (auto it = p.begin(); it != p.end(); ++it)
		if (*it == L'/') *it = '\\';

	// convert relative path to absolute
	if (SU::startsWith(p.c_str(), R"(\YipPreview)"))
		p = std::string(R"(D:\Projects\WxWidgets)") + p;

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
}

// requires sudo mode
bool FU::rmDir(const std::string dir)
{
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
}


//--------------------------------------------------
// Audio utilities
//--------------------------------------------------

void AU::setVolume(const int vol) // 0 - 100
{
	// sets master pc volume to about 10%

	HWND hwin = Main::get().GetHWND();

	SendMessage(hwin, WM_APPCOMMAND, reinterpret_cast<WPARAM>(hwin), APPCOMMAND_VOLUME_MUTE);

	for (int i = 0; i < 50; ++i)
		SendMessageW(hwin, WM_APPCOMMAND, reinterpret_cast<WPARAM>(hwin), APPCOMMAND_VOLUME_DOWN);

	for (int i = 0; i < vol / 2; ++i)
		SendMessageW(hwin, WM_APPCOMMAND, reinterpret_cast<WPARAM>(hwin), APPCOMMAND_VOLUME_UP);

}
