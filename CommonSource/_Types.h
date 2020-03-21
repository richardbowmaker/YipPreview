/*
 * _Types.h
 *
 *  Created on: 21 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <memory>
#include <vector>
#include <map>

class FileSet;
using FileSetT = std::shared_ptr<FileSet>;
using FileSetWRefT = std::weak_ptr<FileSet>;
using FileSetCollT = std::vector<FileSetT>;

class Volume;
using VolumeT = std::shared_ptr<Volume>;
using VolumeWRefT = std::weak_ptr<Volume>;
using VolumeCollT = std::vector<VolumeT>;

using StringStringT = std::pair<std::wstring, std::wstring>;
using MapStringStringT = std::map<std::wstring, std::wstring>;

class FileProperties;
using FilePropertiesT = std::shared_ptr<FileProperties>;

using PropertyT = std::pair<std::wstring, std::wstring>;
using PropertiesT = std::map<std::wstring, std::wstring>;



#endif /* COMMON_TYPES_H_ */
