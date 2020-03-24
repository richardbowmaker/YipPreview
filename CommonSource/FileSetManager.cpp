/*
 * FileSetManager.cpp
 *
 *  Created on: 14 Feb 2020
 *      Author: richard
 */

#include "FileSetManager.h"

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

bool FileSetManager::addFiles(VolumeT volume)
{
	return get().addFilesImpl(volume);
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

bool FileSetManager::addFilesImpl(VolumeT volume)
{
	// scan the volume's directory for files and add
	// them to the filesets_ collection
	StringCollT files;
	if (FU::findMatchingFiles(volume->getFilesDirectory(), files, L"*"))
	{
		for (auto f : files)
		{
			if (FileSet::isValidType(f))
			{
				// search for existing file set entry
				std::wstring id = FileSet::filenameToId(f);

				IdFileSetPairCollT::const_iterator it =
					std::find_if(fileSets_.begin(), fileSets_.end(),
							[id](const IdFileSetPairT &fs) { return fs.first.compare(id) == 0; });

				if (it != fileSets_.end())
				{
					it->second->set(f);
				}
				else
				{
					// create file set object and add to collection
					VolumeWRefT vref(volume);
					FileSetT fs = std::make_shared<FileSet>(vref, f);
					fileSets_.push_back(IdFileSetPairT(id, fs));

					// add file set to volume
					volume->addFileSet(fs);
				}
			}
		}
		volume->readProperties();
	}
	else
		Logger::warning(
				L"FileSetManager::addFilesImpl empty directory %ls",
				volume->getFilesDirectory().c_str());

	return true;
}

std::wstring FileSetManager::toStringImpl()
{
	std::wstring s;
	for (auto fs : fileSets_)
		s += fs.second->toString() + L"\n";
	return s;
}

int FileSetManager::getNoOfFileSetsImpl() const
{
	return static_cast<int>(fileSets_.size());
}

FileSetT FileSetManager::getFileSetImpl(const int n) const
{
	return fileSets_[n].second;
}

void FileSetManager::toLoggerImpl() const
{
	Logger::info(L"File Set Manager, %d", fileSets_.size());

	for (auto fs : fileSets_)
	{
		Logger::info(L"\tkey %ls", fs.first.c_str());
		fs.second->toLogger();
	}
}





