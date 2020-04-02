/*
 * VolumeManager.h
 *
 *  Created on: 21 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_VOLUMEMANAGER_H_
#define COMMON_VOLUMEMANAGER_H_

#include <string>


#include "_Types.h"
#include "Volume.h"

class VolumeManager
{
public:
	virtual ~VolumeManager();

	static VolumeManager &get();
	static void initialise();
	static void uninitialise();
	static void clear();
	static void add(VolumeT &volume);
	static void add(const std::string &file, const bool isMountable);
	static VolumeCollT &getVolumes();
	static VolumeT getVolume(const int n);
	static void toLogger();
	static void writeProperties();
	static bool isMountInUse(const std::string &mount);
	static std::string nextFreeMount();
	static bool mountVolumes(const std::string &password);
	static bool unmountVolumes();
	static bool hasMountedVolumes();
	static FileSetCollT getFileSets();

private:

	VolumeManager() = default;
	VolumeManager(const VolumeManager &) = default;

	void initialiseImpl();
	void uninitialiseImpl();
	void clearImpl();
	void addImpl(VolumeT &volume);
	void addImpl(const std::string &file, const bool isMountable);
	VolumeCollT &getVolumesImpl();
	VolumeT getVolumeImpl(const int n);
	void toLoggerImpl() const;
	void writePropertiesImpl() const;
	bool isMountInUseImpl(const std::string &mount) const;
	std::string nextFreeMountImpl() const;
	bool mountVolumesImpl(const std::string &password);
	bool unmountVolumesImpl();
	bool hasMountedVolumesImpl() const;
	FileSetCollT getFileSetsImpl() const;


	VolumeCollT volumes_;
	StringCollT mounts_;
};

#endif /* COMMON_VOLUMEMANAGER_H_ */
