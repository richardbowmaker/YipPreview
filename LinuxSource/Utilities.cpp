/*
 * Utilities.cpp
 *
 *  Created on: 10 Apr 2020
 *      Author: richard
 */

#include "Utilities.h"

#include <algorithm>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <chrono>
#include <dirent.h>
#include <fmt/core.h>
#include <fnmatch.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <wx/wx.h>

#include "Constants.h"
#include "Logger.h"
#include "ShellExecute.h"

//--------------------------------------------------
// sudo
//--------------------------------------------------

std::mutex SudoMode::lock_;
bool SudoMode::active_ = false;
int SudoMode::refs_ = 0;
int SudoMode::uid_ = 0;

SudoMode::SudoMode() : got_(false)
{
	raise();
}

SudoMode::~SudoMode()
{
	lower();
}

void SudoMode::raise()
{
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
}

void SudoMode::lower()
{
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

		Logger::info("euid %d", geteuid());
	}
}

bool SudoMode::inSudoMode()
{
	return geteuid() == 0;
}

//--------------------------------------------------
// File utilities
//--------------------------------------------------

bool FU::deleteFile(const std::string file)
{
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
}

bool FU::fileExists(const std::string file)
{
	struct stat buffer;
	return stat(file.c_str(), &buffer) == 0;
}

bool FU::copyFile(const std::string src, const std::string dest, const bool overwrite /*= true*/)
{
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

	if (rename(src.c_str(), dest.c_str()) == 0)
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
	struct stat statbuf;
	if (stat(file.c_str(), &statbuf) == 0)
	{
		return static_cast<long long>(statbuf.st_size);
	}
	else
	{
		Logger::systemError("FU::getFileSize() stat() call failed for {}", file);
		return 0;
	}
}

bool FU::getVolumeFreeSpace(const std::string& volume, DiskSpaceT &space)
{
	struct statvfs stat;
	if (statvfs(volume.c_str(), &stat) == 0)
	{
		space.total = static_cast<long long>(stat.f_frsize)* static_cast<long long>(stat.f_blocks);
		space.free = static_cast<long long>(stat.f_bsize)* static_cast<long long>(stat.f_bfree);
		return true;
	}
	else
	{
		Logger::systemError("FU::getFreeSpace() statvfs() failed");
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
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(directory.c_str())) == NULL)
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
	return "/";
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

	// substitute windows absolute path with linux
	if (SU::startsWith(p.c_str(), sW))
		p = std::string(sL) + p.substr(strlen(sW));

	// convert path separator
	for (auto it = p.begin(); it != p.end(); ++it)
		if (*it == L'\\') *it = '/';

	if (SU::startsWith(p.c_str(), "/YipPreview"))
		p = std::string(R"(/media/nas_share/Top/Data/Projects/WxWidgets)") + p;
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
}

// requires sudo mode
bool FU::rmDir(const std::string dir)
{
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
}

//----------------------------------------------------------------------
// Audio utilities
//
// aplay -l		lists all audio devices
//
//
//----------------------------------------------------------------------

void AU::setVolume(const int vol)
{
	// code taken from https://gist.github.com/wolfg1969/3575700

	long min, max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	//const char *selem_name = "Master";

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, "Master");

	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

	snd_mixer_selem_set_playback_volume_all(elem, static_cast<long>(vol) * max / 100);

	snd_mixer_close(handle);
}






