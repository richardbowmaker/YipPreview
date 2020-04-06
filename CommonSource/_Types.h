/*
 * _Types.h
 *
 *  Created on: 21 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

class FileSet;
using FileSetT = std::shared_ptr<FileSet>;
using FileSetCT = const std::shared_ptr<const FileSet>;
using FileSetCollT = std::vector<FileSetT>;
using FileSetCollCT = const std::vector<const FileSetCT>;

class Volume;
using VolumeT = std::shared_ptr<Volume>;
using VolumeCT = std::shared_ptr<const Volume>;
using VolumeCollT = std::vector<VolumeT>;

using StringStringT = std::pair<std::string, std::string>;
using MapStringStringT = std::map<std::string, std::string>;

class Properties;
using PropertiesT = std::shared_ptr<Properties>;

using PropertyT = std::pair<std::string, std::string>;
using PropertyCollT = std::map<std::string, std::string>;

using StringCollT = std::vector<std::string>;

enum class ColT
{
	File,
	Type,
	Selected,
	Duration,
	LastTime,
	Times,
	MaxVol,
	AverageVol,
	Volume,
	Mount,
	NoOfCols
};


#endif /* COMMON_TYPES_H_ */
