/*
 * FileSetManager.cpp
 *
 *  Created on: 14 Feb 2020
 *      Author: richard
 */

#include "FileSetManager.h"

#include <algorithm>
#include <ctime>
#include <fmt/core.h>
#include <iostream>
#include <map>
#include <string>

#include "_Types.h"
#include "FileSet.h"
#include "Logger.h"
#include "Main.h"
#include "Utilities.h"
#include "VolumeManager.h"

FileSetManager::FileSetManager() :
	sortCol_(ColT::File),
	sortAscend_(true)
{
}

FileSetManager::~FileSetManager()
{
}

void FileSetManager::initialise()
{
	get().initialiseImpl();
}

void FileSetManager::uninitialise()
{
	get().uninitialiseImpl();
}

FileSetManager& FileSetManager::get()
{
	static FileSetManager instance;
	return instance;
}

int FileSetManager::getNoOfFileSets()
{
	return get().getNoOfFileSetsImpl();
}

FileSetT FileSetManager::getFileSet(const int n)
{
	return get().getFileSetImpl(n);
}

std::string FileSetManager::toString()
{
	return get().toStringImpl();
}

void FileSetManager::setFileSets(const FileSetCollT &fileSets)
{
	get().setFileSetsImpl(fileSets);
}

const FileSetCollT FileSetManager::getFileSets()
{
	return get().getFileSetsImpl();
}

void FileSetManager::sort(const ColT col)
{
	get().sortImpl(col);
}

std::string FileSetManager::getNextId()
{
	return get().getNextIdImpl();
}

void FileSetManager::addFileSet(FileSetT &fs)
{
	get().addFileSetImpl(fs);
}

void FileSetManager::removeFileSet(FileSetT& fs)
{
	get().removeFileSetImpl(fs);
}

void FileSetManager::toLogger()
{
	get().toLoggerImpl();
}

//----------------------------------------------
// implementation
//----------------------------------------------

void FileSetManager::initialiseImpl()
{
}

void FileSetManager::uninitialiseImpl()
{
	fileSets_.clear();
}

std::string FileSetManager::toStringImpl()
{
	std::string s;
	for (auto fs : fileSets_)
		s += fs->toString() + "\n";
	return s;
}

int FileSetManager::getNoOfFileSetsImpl() const
{
	return static_cast<int>(fileSets_.size());
}

FileSetT FileSetManager::getFileSetImpl(const int n) const
{
	return fileSets_[n];
}

void FileSetManager::setFileSetsImpl(const FileSetCollT &fileSets)
{
	// new set of file sets, they must be sorted
	fileSets_ = fileSets;
	sortAscend_ = false;
	sortImpl(ColT::File);
	sortAscend_ = false; // so that after a refresh clicking the file column keeps ascending order
}

const FileSetCollT FileSetManager::getFileSetsImpl() const
{
	return fileSets_;
}

void FileSetManager::sortImpl(const ColT col)
{
	if (sortCol_ == col)
		sortAscend_ = !sortAscend_;
	else
	{
		sortCol_ = col;
		sortAscend_ = true;
	}

	Logger::info("FileSetManager::sortImpl() {} {}", col, std::string((sortAscend_ ? "ascending" : "descending")));

	std::sort(fileSets_.begin(), fileSets_.end(),
		[this, col](FileSetT& f1, FileSetT& f2) {return f1->sort(col, f2, sortAscend_); });

	Main::get().refresh();
}

// return unique file id in fYYYYMMDDHHMMSS format
std::string FileSetManager::getNextIdImpl()
{
    while (true)
    {
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
		std::string id = fmt::format("file{:04}{:02}{:02}{:02}{:02}{:02}",
				now->tm_year + 1900,
				now->tm_mon,
				now->tm_mday,
				now->tm_hour,
				now->tm_min,
				now->tm_sec);

		// ensure this id has not been used before
		if (id.compare(lastId_) != 0)
		{
			lastId_ = id;
			return id;
		}
		US::delay(500);
    }
}

void FileSetManager::addFileSetImpl(FileSetT &fs)
{
	fileSets_.push_back(fs);
}

void FileSetManager::removeFileSetImpl(FileSetT& fs)
{
	FileSetCollT::const_iterator n = std::find_if(
		fileSets_.cbegin(), fileSets_.cend(),
		[fs](const FileSetT& fs1) { return fs->getId().compare(fs1->getId()) == 0; });

	if (n != fileSets_.cend())
		fileSets_.erase(n);
}

void FileSetManager::toLoggerImpl() const
{
	Logger::info("File Set Manager, %d", static_cast<int>(fileSets_.size()));

	for (auto fs : fileSets_)
		fs->toLogger();
}



