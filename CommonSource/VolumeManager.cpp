/*
 * VolumeManager.cpp
 *
 *  Created on: 21 Mar 2020
 *      Author: richard
 */

#include "VolumeManager.h"

#include "Logger.h"

VolumeManager::~VolumeManager()
{
}

VolumeManager& VolumeManager::get()
{
	static VolumeManager instance;
	return instance;
}

void VolumeManager::uninitialise()
{
	get().uninitialiseImpl();
}

void VolumeManager::add(VolumeT& volume)
{
	get().addImpl(volume);
}

VolumeCollT& VolumeManager::getVolumes()
{
	return get().getVolumesImpl();
}

void VolumeManager::toLogger()
{
	get().toLoggerImpl();
}

void VolumeManager::writeProperties()
{
	get().writePropertiesImpl();
}

void VolumeManager::uninitialiseImpl()
{
	volumes_.clear();
}

void VolumeManager::addImpl(VolumeT& volume)
{
	volumes_.push_back(volume);
}

VolumeCollT& VolumeManager::getVolumesImpl()
{
	return volumes_;
}

void VolumeManager::toLoggerImpl() const
{
	Logger::info(L"Volume Manager");
	for (auto v : volumes_)
		v->toLogger();
}

void VolumeManager::writePropertiesImpl() const
{
	for (auto v : volumes_)
		v->writeProperties();
}



