/*
 * VolumeManager.h
 *
 *  Created on: 21 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_VOLUMEMANAGER_H_
#define COMMON_VOLUMEMANAGER_H_

#include "_Types.h"
#include "Volume.h"

class VolumeManager
{
public:
	virtual ~VolumeManager();

	static VolumeManager &get();
	static void uninitialise();
	static void add(VolumeT &volume);
	static VolumeCollT& getVolumes();
	static void toLogger();
	static void writeProperties();

private:

	VolumeManager() = default;
	VolumeManager(const VolumeManager &) = default;

	void uninitialiseImpl();
	void addImpl(VolumeT &volume);
	VolumeCollT& getVolumesImpl();
	void toLoggerImpl() const;
	void writePropertiesImpl() const;

	VolumeCollT volumes_;
};

#endif /* COMMON_VOLUMEMANAGER_H_ */
