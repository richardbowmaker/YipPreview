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
using FileSetCollT = std::vector<FileSetT>;

class Volume;
using VolumeT = std::shared_ptr<Volume>;
using VolumeCollT = std::vector<VolumeT>;

using StringStringT = std::pair<std::wstring, std::wstring>;
using MapStringStringT = std::map<std::wstring, std::wstring>;

class FileProperties;
using FilePropertiesT = std::shared_ptr<FileProperties>;

using PropertyT = std::pair<std::wstring, std::wstring>;
using PropertiesT = std::map<std::wstring, std::wstring>;

using StringCollT = std::vector<std::wstring>;

enum ColT
{
	FileCol = 0,
	TypeCol,
	SelectedCol,
	DurationCol,
	LastTimeCol,
	TimesCol,
	MaxVolCol,
	AverageVolCol

};


#endif /* COMMON_TYPES_H_ */
