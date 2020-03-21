/*
 * Volume.cpp
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#include "Volume.h"

#include <fstream>
#include <stdio.h>

#include "_Types.h"
#include "Constants.h"
#include "Logger.h"
#include "Utilities.h"

Volume::Volume(std::wstring root) :
	root_(root),
	isMounted_(true)
{
}

Volume::~Volume()
{
}

std::wstring Volume::getRoot() const
{
	return root_;
}

bool Volume::getIsMounted() const
{
	return isMounted_;
}

std::wstring Volume::getPropertiesFile() const
{
	return root_ + Constants::propertiesCache;
}

std::wstring Volume::getFilesDirectory() const
{
	return root_ + Constants::filesDir;
}

void Volume::addFileSet(FileSetT &fileset)
{
	filesets_.push_back(fileset);
}

void Volume::readProperties()
{
	// read the properties cache into a map of file id v properties list string
	std::string pf = SU::wStrToStr(getPropertiesFile());
	std::ifstream pcache(pf);
	std::string sl;
	while (std::getline(pcache, sl))
	{
		std::wstring wl = SU::strToWStr(sl);
		int n = wl.find(L";");

		if (n != std::wstring::npos)
		{
			std::wstring fn = wl.substr(0, n);
			std::wstring id = FileSet::filenameToId(fn);
			std::wstring ps = wl.substr(n + 1);

			FileSetCollT::const_iterator it =
				std::find_if(filesets_.begin(), filesets_.end(),
						[id](const FileSetT &fs) { return fs->getId().compare(id) == 0; });

			if (it != filesets_.end())
				(*it)->properties().fromString(ps);
		}
	}
	pcache.close();
}

void Volume::writeProperties()
{
	if (!isMounted_) return;

	std::string pf = SU::wStrToStr(getPropertiesFile());
	std::ofstream pcache(pf);
	for (auto fs : filesets_)
	{
		if (fs->properties().getSize() > 0)
		{
			pcache  << SU::wStrToStr(fs->getId())
					<< ";"
					<< SU::wStrToStr(fs->properties().toString())
					<< std::endl;
		}
	}
	pcache.close();
}

void Volume::toLogger() const
{
	Logger::info(L"Volume");
	Logger::info(L"\troot %ls", root_.c_str());
	if (isMounted_)
		Logger::info(L"\tmounted");
	else
		Logger::info(L"\tnot mounted");

	for (auto fs : filesets_)
		fs->toLogger();
}





