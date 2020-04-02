
#include "FileSet.h"

#include <algorithm>
#include <cwchar>
#include <filesystem>
#include <regex>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "_Types.h"
#include "Logger.h"
#include "Utilities.h"


FileSet::FileSet() : volume_(nullptr)
{
}

FileSet::FileSet(const Volume *volume, const std::wstring file)
{
	volume_ = volume;
	set(file);
}

FileSet::~FileSet()
{
}

std::wstring FileSet::filenameToId(const std::wstring filename)
{
	return FU::getFileStem(filename);
}

FileSet::TypeT FileSet::filenameToType(const std::wstring filename)
{
	std::wstring ext = FU::getExt(filename);
	if (ext.compare(L"jpg") == 0 || ext.compare(L"jpeg") == 0)
		return ImageFile;
	else if (ext.compare(L"mp4") == 0 || ext.compare(L"avi") == 0)
		return VideoFile;
	else if (ext.compare(L"lnk") == 0)
		return LinkFile;
	else
		return InvalidFile;
}

bool FileSet::isValidType(const std::wstring filename)
{
	return filenameToType(filename) != InvalidFile;
}

void FileSet::set(const std::wstring filename)
{
	switch (filenameToType(filename))
	{
	case ImageFile: setImage(filename); return;
	case VideoFile: setVideo(filename); return;
	case LinkFile:  setLink(filename);  return;
	default:
		Logger::error(L"FileSet::setFilename, unhandled file type");
		break;
	}
}

std::wstring FileSet::getId() const
{
	return id_;
}

bool FileSet::setId(const std::wstring filename)
{
	std::wstring s = filenameToId(filename);
	if (id_.size() == 0)
	{
		// first time a file is set, capture the id and short name
		id_ = s;
		s = FU::getPathNoExt(filename);
		short_ = FU::abbreviateFilename(s, 30);
		return true;
	}
	if (s.compare(id_) == 0)
	{
		// file matches id ok
		return true;
	}
	else
	{
		Logger::error(L"FileSet::setId mismatch, id = %ls, fn = %ls", id_.c_str(), filename.c_str());
		return false;
	}
}

const Volume* FileSet::getVolume() const
{
	return volume_;
}

std::wstring FileSet::getImage() const
{
	return image_;
}

void FileSet::setImage(const std::wstring filename)
{
	if (setId(filename)) image_ = filename;
}

std::wstring FileSet::getVideo() const
{
	return video_;
}

void FileSet::setVideo(const std::wstring filename)
{
	if (setId(filename)) video_ = filename;
}

std::wstring FileSet::getLink() const
{
	return link_;
}

void FileSet::setLink(const std::wstring filename)
{
	if (setId(filename)) link_ = filename;
}

bool FileSet::hasImage() const
{
	return image_.size() > 0;
}

bool FileSet::hasVideo() const
{
	return video_.size() > 0;
}

bool FileSet::hasLink() const
{
	return link_.size() > 0;
}

std::wstring FileSet::typesToString() const
{
	std::wstring s;
	if (hasVideo()) s += L"V";
	if (hasImage()) s += L"I";
	if (hasLink())  s += L"L";
	return s;
}

std::wstring FileSet::getShortName() const
{
	return short_;
}

std::wstring FileSet::toString() const
{
	return std::wstring(L"ID = ") + id_ + std::wstring(L", ") + typesToString();
}

bool FileSet::sort(const ColT col, FileSetT &other, const bool ascending)
{
/* sort() is used by the grid column sort function.
 *
 * Column sorted by value, if two values are the same then sorted by the file id.
 * If the value is blank it will be displayed beneath those rows that do have a value
 * (i.e. to stop the values of interest being sorted pushed to the end of the grid)
 * Those rows with a blank in the column will always be sorted by ascending file id
 *
 *  if v1 = v2 return id1 < id2
 *  if v1 empty && !v2 empty return false
 *  if !v1 empty && v2 empty return true
 *
 * if ascending
 * 		if v1 < v2 then return true;
 * 		else if v2 < v1 return false;
 * 		else return id1 < id2
 *
 * if descending
 * 		if v1 > v2 then return true;
 * 		else if v1 < v2 return true;
 * 		else return id1 < id2
 *
 *
 */

//
//	// helper to format a volume float to sortable string
//	auto volToString = [](FileProperties &ps, const std::wstring field)
//	{
//		std::wstring s;
//		wchar_t buf[1000];
//		if (ps.getString(field).size() > 0)
//		{
//			float f = ps.getFloat(field);
//			swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%3.2f", std::abs(f));
//			s = std::wstring(buf);
//			if (f >= 0)
//				s = std::wstring(1, L'>') + s;
//			else
//				s = std::wstring(1, L'<') + s;
//		}
//		return s;
//	};
//
//	// time field to a sortable string
//	auto timeToString = [](const std::wstring &t) -> std::wstring
//	{
//		// 0123456789012345678
//		// 15:13:54 30/03/2020
//		std::wstringstream s;
//		if (t.size() == 19)
//			s << t.substr(15, 4) << t.substr(12, 2) << t.substr(9, 2) << t.substr(0, 8);
//		else if (t.size() == 10)
//			s << t.substr(15, 4) << t.substr(12, 2) << t.substr(9, 2) << L"00:00:00";
//		return s.str();
//	};
//
//	auto timesToString = [](const int ts) -> std::wstring
//	{
//		std::wstring s;
//		if (ts > 0)
//		{
//			wchar_t buf[1000];
//			swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%05d", ts);
//			s = std::wstring(buf);
//		}
//		return s;
//   };

	// the sort fields
	std::wstring s1;
	std::wstring s2;
	s1 = id_;
	s2 = other->getId();
	int n = s1.compare(s2);
	return n;


//	switch (static_cast<ColT>(col))
//	{
//	case ColT::Volume:
////		s1 = volume_->getMount();
////		s2 = other->getVolume()->getMount();
//		break;
//	case ColT::File:
//		s1 = id_;
//		s2 = other->getId();
//		break;
//	case ColT::Type:
//		s1 = typesToString();
//		s2 = other->typesToString();
//		break;
//	case ColT::Selected:
//		s1 = getIsSelected();
//		s2 = other->getIsSelected();
//		break;
//	case ColT::Duration:
//		s1 = getDurationStr();
//		s2 = other->getDurationStr();
//		break;
//	case ColT::LastTime:
////		s1 = timeToString(getLastTime());
////		s2 = timeToString(other->getLastTime());
//		s1 = getLastTime();
//		s2 = other->getLastTime();
//		break;
////	case ColT::Times:
////		s1 = timesToString(properties_.getInt(L"times"));
////		s2 = timesToString(other->properties().getInt(L"times"));
////		break;
////	case ColT::MaxVol:
////		s1 = volToString(properties_, L"maxvol");
////		s2 = volToString(other->properties(), L"maxvol");
////		break;
////	case ColT::AverageVol:
////		s1 = volToString(properties_, L"averagevol");
////		s2 = volToString(other->properties(), L"averagevol");
////		break;
//	default:
//		break;
//	}
//
//	// see header for explanation
//	int n = s1.compare(s2);
//	return n;

//	if (n == 0)
//	{
//		n = id_.compare(other->getId());
//		return n;
////		if (n == 0)
////		{
////			return (volume_->getMount().compare(other->getVolume()->getMount()) < 0);
////		}
////		else
////			return n;
//	}
//	if (s1.size() == 0) return false;
//	if (s2.size() == 0) return true;
//	if (ascending) return n < 0;
//	else return n > 0;
}

void FileSet::toLogger()
{
	Logger::info(L"File Set");

	Logger::info(L"\tID %ls",         id_.c_str());
	Logger::info(L"\tShortname %ls",  short_.c_str());
	Logger::info(L"\tImage file %ls", image_.c_str());
	Logger::info(L"\tVideo file %ls", video_.c_str());
	Logger::info(L"\tLink file %ls",  link_.c_str());
	properties_.toLogger();
}

//-------------------------------------------------------
// properties
//-------------------------------------------------------
// \Files\All\file20191106221222;times;11;lasttime;08/01/2020;volume;-8.3  -40.7;duration;5:27

FileProperties& FileSet::properties()
{
	return properties_;
}

void FileSet::setDurationMs(const long ms)
{
	Duration d;
	d.setMs(ms);
	properties_.setString(L"duration", d.toString());
}

long FileSet::getDurationMs() const
{
	Duration d;
	d.parse(properties_.getString(L"duration"));
	return d.getMs();
}

// (hh:)mm:ss(.nnn)
void FileSet::setDurationStr(const std::wstring duration)
{
	std::wstring s;
	if (duration.size() > 0)
	{
		Duration d;
		if (d.parse(duration)) s = d.toString();
	}
	properties_.setString(L"duration", s);
}

std::wstring FileSet::getDurationStr() const
{
	std::wstring p = properties_.getString(L"duration");
	if (p.size() > 0)
	{
		Duration d;
		if (d.parse(p)) p = d.toString();
	}
	return p;
}

std::wstring FileSet::getLastTime() const
{
	return properties_.getString(L"lasttime");
}

std::wstring FileSet::getTimes() const
{
	return properties_.getString(L"times");
}

float FileSet::getMaxVol() const
{
	return properties_.getFloat(L"maxvol");
}

std::wstring FileSet::getMaxVolStr() const
{
	return properties_.getString(L"maxvol");
}

void FileSet::setMaxVol(const float maxvol)
{
	wchar_t buf[100];
	swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%.2f", maxvol);
	properties_.setString(L"maxvol", std::wstring(buf));
}

float FileSet::getAverageVol() const
{
	return properties_.getFloat(L"averagevol");
}

std::wstring FileSet::getAverageVolStr() const
{
	return properties_.getString(L"averagevol");
}

void FileSet::setAverageVol(const float averagevol)
{
	wchar_t buf[100];
	swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%.2f", averagevol);
	properties_.setString(L"averagevol", std::wstring(buf));
}

void FileSet::setIsSelected(const bool selected)
{
	properties_.setString(L"selected", selected ? L"X" : L"");
}

std::wstring FileSet::getIsSelected() const
{
	return properties_.getString(L"selected");
}













