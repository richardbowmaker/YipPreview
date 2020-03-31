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
	sortCol_(FileCol),
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

std::wstring FileSetManager::toString()
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

std::wstring FileSetManager::toStringImpl()
{
	std::wstring s;
	for (auto fs : fileSets_)
		s += fs->toString() + L"\n";
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
	sortImpl(FileCol);
	sortAscend_ = false; // so that after a refresh clicking the file column keeps ascending order
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

	Logger::info(L"FileSetManager::sortImpl() %d %ls", col, (sortAscend_ ? L"ascending" : L"descending"));

	std::sort(fileSets_.begin(), fileSets_.end(),
		[this, col](FileSetT &f1, FileSetT &f2) { return f1->sort(col, f2, sortAscend_); });

	Main::get().refresh();
}

void FileSetManager::toLoggerImpl() const
{
	Logger::info(L"File Set Manager, %d", fileSets_.size());

	for (auto fs : fileSets_)
		fs->toLogger();
}





