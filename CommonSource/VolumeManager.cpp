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
#include "FileSetManager.h"
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

void VolumeManager::clear()
{
	get().clearImpl();
}

void VolumeManager::add(VolumeT& volume)
{
	get().addImpl(volume);
}

void VolumeManager::add(const std::string &file, const bool isMountable)
{
	get().addImpl(file, isMountable);
}

VolumeCollT &VolumeManager::getVolumes()
{
	return get().getVolumesImpl();
}

VolumeT VolumeManager::getVolume(const int n)
{
	return get().getVolumeImpl(n);
}

void VolumeManager::writeProperties()
{
	get().writePropertiesImpl();
}

bool VolumeManager::isMountInUse(const std::string &mount)
{
	return get().isMountInUseImpl(mount);
}

std::string VolumeManager::nextFreeMount()
{
	return get().nextFreeMountImpl();
}

bool VolumeManager::mountVolumes(const std::string &password)
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

FileSetCollT VolumeManager::getFileSets()
{
	return get().getFileSetsImpl();
}

VolumeT VolumeManager::findVolumeWithFreeSpace(const long long bytes)
{
	return get().findVolumeWithFreeSpaceImpl(bytes);
}

//---------------------------------------------

void VolumeManager::initialiseImpl()
{
#ifdef WINDOWS_BUILD
	// built a list of candidate mount points
	char l{'H'};
	for (int i = 0; i < 10; ++i)
		mounts_.push_back(std::string(1, l++) + std::string(R"(:)"));
#elif LINUX_BUILD
	for (int i = 0; i < 10; ++i)
	{
		char buf[200];
		snprintf(buf, sizeof(buf) / sizeof(char), R"(/media/volume%02d)", i + 1);
		mounts_.push_back(std::string(buf));
	}
#endif
}

void VolumeManager::uninitialiseImpl()
{
	volumes_.clear();
	mounts_.clear();
}

void VolumeManager::clearImpl()
{
	volumes_.clear();
}

void VolumeManager::addImpl(VolumeT& volume)
{
	// add volume to collection if not already in it
	auto n = std::find_if(volumes_.cbegin(), volumes_.cend(),
			[&volume](auto &v){return v->getFile().compare(volume->getFile()) == 0;});

	if (n == volumes_.cend())
		volumes_.push_back(volume);
}

void VolumeManager::addImpl(const std::string &file, const bool isMountable)
{
	// add volume to collection if not already in it
	auto n = std::find_if(volumes_.cbegin(), volumes_.cend(),
			[&file](auto &v){return v->getFile().compare(file) == 0;});

	if (n == volumes_.cend())
		volumes_.emplace_back(std::make_shared<Volume>(file, isMountable));
}

VolumeCollT &VolumeManager::getVolumesImpl()
{
	return volumes_;
}

VolumeT VolumeManager::getVolumeImpl(const int n)
{
	if (n >= 0 && n < static_cast<int>(volumes_.size()))
		return volumes_[n];
	else
	{
		Logger::error("VolumeManager::getVolumeImpl() no such volume %d", n);
		return nullptr;
	}
}

void VolumeManager::writePropertiesImpl() const
{
	for (auto v : volumes_)
	{
		if (v->getIsMountable() && v->getIsMounted())
			v->writeProperties();
		if (!v->getIsMountable() && v->getIsSelected())
			v->writeProperties();
	}
}

bool VolumeManager::isMountInUseImpl(const std::string &mount) const
{
	StringCollT files;
	StringCollT dirs;
	FU::findFilesDirs(mount, files, dirs);
	return (files.size() + dirs.size()) > 0;
}

std::string VolumeManager::nextFreeMountImpl() const
{
	for (auto m : mounts_)
	{
		if (!isMountInUseImpl(m)) return m;
	}
	return "";
}

bool VolumeManager::mountVolumesImpl(const std::string &password)
{
	bool result = true;

	for (auto v : volumes_)
	{
		if (v->getIsMountable())
		{
			if (v->getIsSelected() && !v->getIsMounted())
			{
				std::string m = nextFreeMountImpl();
				if (m.size()> 0)
				{
#ifdef LINUX_BUILD
					// in linux have to create a mount folder
					// if the mount already exists then this is unexpected,
					// use has to decide whether to use it or not
					if (FU::fileExists(m))
					{
						int r = Utilities::messageBox(
								"Mount folder \'{}\' already exists, do you want to use it ?", "Mount volume",
								wxYES_NO | wxCANCEL, &Main::get(), m);
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
#endif
					if (v->mount(m, password))
					{
						v->loadFiles();
						v->readProperties();
						result &= true;
					}
					else
						result &= false;
				}
				else
					result &= false;
			}
			if (!v->getIsSelected() && v->getIsMounted())
			{
				// unmount volume
				std::string m = v->getMount();
				v->writeProperties();
				v->clearFiles();
				result &= v->unmount();
#ifdef LINUX_BUILD
				// in linux remove the mount directory
				SudoMode sudo;
				if (!FU::rmDir(m)) result = false;
				sudo.lower();
#endif
			}
		}
		else
		{
			// non-mounting volumes
			if (v->getIsSelected() && !v->hasFileSets())
			{
				v->loadFiles();
				v->readProperties();
				result &= true;
			}
			if (!v->getIsSelected() && v->hasFileSets())
			{
				v->writeProperties();
				v->clearFiles();
				result &= true;
			}
		}
	}

	if (result)
		Logger::info("All volumes mounted OK");
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
			v->writeProperties();
			std::string m = v->getMount();
			result &= v->unmount();
#ifdef LINUX_BUILD
			// in linux remove the mount directory
			SudoMode sudo;
			if (!FU::rmDir(m)) result = false;
			sudo.lower();
#endif
		}
		if (!v->getIsMountable() && v->getIsSelected())
			v->writeProperties();
	}
	if (result)
		Logger::info("All volumes unmounted OK");
	return result;
}

bool VolumeManager::hasMountedVolumesImpl() const
{
	return std::any_of(volumes_.cbegin(), volumes_.cend(),
			[](auto &v){ return v->getIsMountable() && v->getIsMounted();});
}

FileSetCollT VolumeManager::getFileSetsImpl() const
{
	FileSetCollT fileSets;
	for (const auto v : volumes_)
	{
		FileSetCollT fs = v->getFileSets();
		fileSets.insert(fileSets.cbegin(), fs.cbegin(), fs.cend());
	}
	return fileSets;
}

VolumeT VolumeManager::findVolumeWithFreeSpaceImpl(const long long bytes) const
{
	for (auto v : volumes_)
	{
		auto [b, ts, fs] = v->getFreeSpace();
		if (b && fs > (Constants::minDiskFreeSpace + bytes)) return v;
	}
	return nullptr;
}

void VolumeManager::toLoggerImpl() const
{
	Logger::info("Volume Manager");
	Logger::info("mounts:");
	for (auto m : mounts_)
	{
		Logger::info(m.c_str());
		if (isMountInUseImpl(m))
			Logger::info("mount in use");
	}
	Logger::info("volumes:");
	for (auto v : volumes_)
		v->toLogger();
}


