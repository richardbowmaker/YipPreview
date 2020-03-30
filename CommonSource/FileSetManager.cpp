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
#include "Utilities.h"
#include "VolumeManager.h"

FileSetManager::FileSetManager()
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

	std::sort(fileSets_.begin(),
			  fileSets_.end(),
			  [](const FileSetT &f1, const FileSetT &f2)
			      { return f1->getId().compare(f2->getId()) < 0; });
}

void FileSetManager::toLoggerImpl() const
{
	Logger::info(L"File Set Manager, %d", fileSets_.size());

	for (auto fs : fileSets_)
		fs->toLogger();
}





