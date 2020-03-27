/*
 * VolumeManager.cpp
 *
 *  Created on: 21 Mar 2020
 *      Author: richard
 */

#include "VolumeManager.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include "Constants.h"
#include "Logger.h"
#include "Main.h"
#include "Utilities.h"

VolumeManager::~VolumeManager()
{
}

VolumeManager& VolumeManager::get()
{
	static VolumeManager instance;
	return instance;
}

void VolumeManager::initialise()
{
	get().initialiseImpl();
}

void VolumeManager::uninitialise()
{
	get().uninitialiseImpl();
}

void VolumeManager::add(VolumeT& volume)
{
	get().addImpl(volume);
}

VolumeCollT VolumeManager::getVolumes()
{
	return get().getVolumesImpl();
}

void VolumeManager::writeProperties()
{
	get().writePropertiesImpl();
}

bool VolumeManager::isMountInUse(const std::wstring &mount)
{
	return get().isMountInUseImpl(mount);
}

std::wstring VolumeManager::nextFreeMount()
{
	return get().nextFreeMountImpl();
}

bool VolumeManager::mountVolumes(const std::wstring &password)
{
	return get().mountVolumesImpl(password);
}

bool VolumeManager::unmountVolumes()
{
	return get().unmountVolumesImpl();
}

void VolumeManager::toLogger()
{
	get().toLoggerImpl();
}

bool VolumeManager::hasMountedVolumes()
{
	return get().hasMountedVolumesImpl();
}

//---------------------------------------------

void VolumeManager::initialiseImpl()
{
	// built a list of candidate mount points
	for (int i = 0; i < 10; ++i)
	{
#ifdef WINDOWS_BUILD
		wchar_t l{'H' + i};
		mounts_.push_back(std::wstring(l, 1) + std::wstring(LR"(:)"));
#elif LINUX_BUILD
		wchar_t buf[200];
		swprintf(buf, sizeof(buf) / sizeof(wchar_t), LR"(/media/volume%02d)", i + 1);
		mounts_.push_back(std::wstring(buf));
#endif
	}
}

void VolumeManager::uninitialiseImpl()
{
	volumes_.clear();
	mounts_.clear();
}

void VolumeManager::addImpl(VolumeT& volume)
{
	volumes_.push_back(volume);
}

VolumeCollT VolumeManager::getVolumesImpl()
{
	return volumes_;
}

void VolumeManager::writePropertiesImpl() const
{
	for (auto v : volumes_)
		v->writeProperties();
}

bool VolumeManager::isMountInUseImpl(const std::wstring &mount) const
{
	StringCollT files;
	StringCollT dirs;
	FU::findFilesDirs(mount, files, dirs);
	return (files.size() + dirs.size()) > 0;
}

std::wstring VolumeManager::nextFreeMountImpl() const
{
	for (auto m : mounts_)
	{
		if (!isMountInUseImpl(m)) return m;
	}
	return L"";
}

bool VolumeManager::mountVolumesImpl(const std::wstring &password)
{
	bool result = true;

	for (auto v : volumes_)
	{
		if (v->getIsMountable() && !v->getIsMounted())
		{
			std::wstring m = nextFreeMountImpl();
			if (m.size()> 0)
			{
				// if the mount already exists then this is unexpected,
				// use has to decide whether to use it or not
				if (FU::fileExists(m))
				{
					int r = Utilities::messageBox(
							L"Mount folder \'%ls\' already exists, do you want to use it ?", L"Mount volume",
							wxYES_NO | wxCANCEL, &MyFrame::get(), m.c_str());

					if (r == wxCANCEL) return false;
					if (r == wxNO) continue;
				}
				else
				{
					// create the mount directory, if fails to create move
					SudoMode sudo;
					if (!FU::mkDir(m))
					{
						result = false;
						continue;
					}
				}
				result &= (v->mount(m, password));
			}
		}
	}
	if (result)
		Logger::info(L"All volumes mounted OK");
	return result;
}

bool VolumeManager::unmountVolumesImpl()
{
	bool result = true;
	for (auto v : volumes_)
	{
		if (v->getIsMountable() && v->getIsMounted())
		{
			// unmount volume
			std::wstring m = v->getMount();
			result &= v->unmount();

			SudoMode sudo;
			if (!FU::rmDir(m)) result = false;
			sudo.lower();
		}
	}
	if (result)
		Logger::info(L"All volumes unmounted OK");
	return result;
}

bool VolumeManager::hasMountedVolumesImpl() const
{
	return std::any_of(volumes_.cbegin(), volumes_.cend(),
			[](auto &v){ return v->getIsMountable() && v->getIsMounted();});
}

void VolumeManager::toLoggerImpl() const
{
	Logger::info(L"Volume Manager");
	Logger::info(L"mounts:");
	for (auto m : mounts_)
	{
		Logger::info(m.c_str());
		if (isMountInUseImpl(m))
			Logger::info(L"mount in use");
	}
	Logger::info(L"volumes:");
	for (auto v : volumes_)
		v->toLogger();
}


