/*
 * FileSetManager.cpp
 *
 *  Created on: 14 Feb 2020
 *      Author: richard
 */

#include "FileSetManager.h"

#include <algorithm>
#include <map>

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

void FileSetManager::sort(const ColT col)
{
	get().sortImpl(col);
}

void FileSetManager::toLogger()
{
	get().toLoggerImpl();
}

void FileSetManager::check()
{
	get().checkImpl();
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

ColT xcol;
bool xa;
int count;

bool mysort(FileSetT &f1, FileSetT &f2)
{
	count++;

	if (f1->check_ != 123456 || f2->check_ != 123456)
	{
		FileSetManager::check();
		int n = 0;
	}
	bool b = f1->sort(xcol, f2, xa);
	if (f1->check_ != 123456 || f2->check_ != 123456)
	{
		int n = 0;
	}
	return b;
}

void FileSetManager::sortImpl(const ColT col)
{
	return;

	count = 0;

	if (sortCol_ == col)
		sortAscend_ = !sortAscend_;
	else
	{
		sortCol_ = col;
		sortAscend_ = true;
	}

	Logger::info("FileSetManager::sortImpl() {} {}", col, std::string((sortAscend_ ? "ascending" : "descending")));


	checkImpl();
	xcol = col;
	xa = sortAscend_;

	std::sort(fileSets_.begin(), fileSets_.end(), mysort);
//		[this, col](FileSetT &f1, FileSetT &f2)
//		{
//			if (f1->check_ != 123456 || f2->check_ != 123456)
//			{
//				int n = 0;
//			}
//			bool b = f1->sort(col, f2, sortAscend_);
//			if (f1->check_ != 123456 || f2->check_ != 123456)
//			{
//				int n = 0;
//			}
//			return b;
//		});

	checkImpl();

	Main::get().refresh();
}

void FileSetManager::toLoggerImpl() const
{
	Logger::info("File Set Manager, %d", static_cast<int>(fileSets_.size()));

	for (auto fs : fileSets_)
		fs->toLogger();
}


void FileSetManager::checkImpl()
{
	for (auto fs : fileSets_)
		if (fs->check_ != 123456)
		{
			int nn = 0;
		}

}





