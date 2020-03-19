/*
 * Volume.cpp
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#include "Volume.h"

#include "Constants.h"
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
	return FU::pathToLocal(LR"(/YipPreview/Tryout/PropertyCache.txt)");
}

std::wstring Volume::getFilesDirectory() const
{
	return FU::pathToLocal(LR"(/YipPreview/Tryout)");
}


