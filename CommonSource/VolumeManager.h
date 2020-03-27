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
	static void initialise();
	static void uninitialise();
	static void add(VolumeT &volume);
	static VolumeCollT getVolumes();
	static void toLogger();
	static void writeProperties();
	static bool isMountInUse(const std::wstring &mount);
	static std::wstring nextFreeMount();
	static bool mountVolumes(const std::wstring &password);
	static bool unmountVolumes();
	static bool hasMountedVolumes();

private:

	VolumeManager() = default;
	VolumeManager(const VolumeManager &) = default;

	void initialiseImpl();
	void uninitialiseImpl();
	void addImpl(VolumeT &volume);
	VolumeCollT getVolumesImpl();
	void toLoggerImpl() const;
	void writePropertiesImpl() const;
	bool isMountInUseImpl(const std::wstring &mount) const;
	std::wstring nextFreeMountImpl() const;
	bool mountVolumesImpl(const std::wstring &password);
	bool unmountVolumesImpl();
	bool hasMountedVolumesImpl() const;

	VolumeCollT volumes_;
	StringCollT mounts_;
};

#endif /* COMMON_VOLUMEMANAGER_H_ */
