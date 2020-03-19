/*
 * FileSetManager.cpp
 *
 *  Created on: 14 Feb 2020
 *      Author: richard
 */

#include "FileSetManager.h"

#include <fstream>

#include "FileSet.h"
#include "Logger.h"
#include "Utilities.h"

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

bool FileSetManager::addFiles(const VolumeT volume)
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

void FileSetManager::initialiseImpl()
{
}

void FileSetManager::uninitialiseImpl()
{
	fileSets_.clear();
}

bool FileSetManager::addFilesImpl(const VolumeT volume)
{
	StringsT files;
	if (FU::findMatchingFiles(volume->getFilesDirectory(), files, L"*"))
	{
		for (auto f : files)
		{
			if (FileSet::isValidType(f))
			{
				// search for existing fileset entry
				std::wstring id = FileSet::filenameToId(f);

				FileSetsT::const_iterator it = std::find_if(fileSets_.begin(), fileSets_.end(),
					[id](IdFileSetPairT& fs) { return fs.first.compare(id) == 0; });

				if (it != fileSets_.end())
				{
					it->second->set(f);
				}
				else
				{
					FileSetT fs = std::make_shared<FileSet>();
					fs->set(f);
					fileSets_.push_back(IdFileSetPairT(id, fs));
				}
			}
		}

		// read the properties cache
		std::string pf = SU::wStrToStr(volume->getPropertiesFile());
		std::ifstream infile(pf);
		std::string sl;
		while (std::getline(infile, sl))
		{
			std::wstring wl = SU::strToWStr(sl);
			int n = wl.find(L";");

			if (n != std::wstring::npos)
			{
				std::wstring fn = wl.substr(0, n);
				std::wstring ps = wl.substr(n + 1);
				int nn = 0;
			}
		}
		infile.close();
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


