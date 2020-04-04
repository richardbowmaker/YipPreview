
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

FileSet::FileSet(const Volume *volume, const std::string file)
{
	volume_ = volume;
	set(file);
}

FileSet::~FileSet()
{
}

std::string FileSet::filenameToId(const std::string filename)
{
	return FU::getFileStem(filename);
}

FileSet::TypeT FileSet::filenameToType(const std::string filename)
{
	std::string ext = FU::getExt(filename);
	if (ext.compare("jpg") == 0 || ext.compare("jpeg") == 0)
		return ImageFile;
	else if (ext.compare("mp4") == 0 || ext.compare("avi") == 0)
		return VideoFile;
	else if (ext.compare("lnk") == 0)
		return LinkFile;
	else
		return InvalidFile;
}

bool FileSet::isValidType(const std::string filename)
{
	return filenameToType(filename) != InvalidFile;
}

void FileSet::set(const std::string filename)
{
	switch (filenameToType(filename))
	{
	case ImageFile: setImage(filename); return;
	case VideoFile: setVideo(filename); return;
	case LinkFile:  setLink(filename);  return;
	default:
		Logger::error("FileSet::setFilename, unhandled file type");
		break;
	}
}

std::string FileSet::getId() const
{
	return id_;
}

bool FileSet::setId(const std::string filename)
{
	std::string s = filenameToId(filename);
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
		Logger::error("FileSet::setId mismatch, id = {}, fn = {}", id_, filename);
		return false;
	}
}

const Volume* FileSet::getVolume() const
{
	return volume_;
}

std::string FileSet::getImage() const
{
	return image_;
}

void FileSet::setImage(const std::string filename)
{
	if (setId(filename)) image_ = filename;
}

std::string FileSet::getVideo() const
{
	return video_;
}

void FileSet::setVideo(const std::string filename)
{
	if (setId(filename)) video_ = filename;
}

std::string FileSet::getLink() const
{
	return link_;
}

void FileSet::setLink(const std::string filename)
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

std::string FileSet::typesToString() const
{
	std::string s;
	if (hasVideo()) s += "V";
	if (hasImage()) s += "I";
	if (hasLink())  s += "";
	return s;
}

std::string FileSet::getShortName() const
{
	return short_;
}

std::string FileSet::toString() const
{
	return std::string("ID = ") + id_ + std::string(", ") + typesToString();
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
//	auto volToString = [](FileProperties &ps, const std::string field)
//	{
//		std::string s;
//		char buf[1000];
//		if (ps.getString(field).size() > 0)
//		{
//			float f = ps.getFloat(field);
//			snprintf(buf, sizeof(buf) / sizeof(char), "%3.2f", std::abs(f));
//			s = std::string(buf);
//			if (f >= 0)
//				s = std::string(1, '>') + s;
//			else
//				s = std::string(1, '<') + s;
//		}
//		return s;
//	};
//
//	// time field to a sortable string
//	auto timeToString = [](const std::string &t) -> std::string
//	{
//		// 0123456789012345678
//		// 15:13:54 30/03/2020
//		std::stringstream s;
//		if (t.size() == 19)
//			s << t.substr(15, 4) << t.substr(12, 2) << t.substr(9, 2) << t.substr(0, 8);
//		else if (t.size() == 10)
//			s << t.substr(15, 4) << t.substr(12, 2) << t.substr(9, 2) << "00:00:00";
//		return s.str();
//	};
//
//	auto timesToString = [](const int ts) -> std::string
//	{
//		std::string s;
//		if (ts > 0)
//		{
//			char buf[1000];
//			snprintf(buf, sizeof(buf) / sizeof(char), "%05d", ts);
//			s = std::string(buf);
//		}
//		return s;
//   };

	// the sort fields
	std::string s1;
	std::string s2;
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
////		s1 = timesToString(properties_.getInt("times"));
////		s2 = timesToString(other->properties().getInt("times"));
////		break;
////	case ColT::MaxVol:
////		s1 = volToString(properties_, "maxvol");
////		s2 = volToString(other->properties(), "maxvol");
////		break;
////	case ColT::AverageVol:
////		s1 = volToString(properties_, "averagevol");
////		s2 = volToString(other->properties(), "averagevol");
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
	Logger::info("File Set");

	Logger::info("\tID {}",         id_);
	Logger::info("\tShortname {}",  short_);
	Logger::info("\tImage file {}", image_);
	Logger::info("\tVideo file {}", video_);
	Logger::info("\tLink file {}",  link_);
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
	properties_.setString("duration", d.toString());
}

long FileSet::getDurationMs() const
{
	Duration d;
	d.parse(properties_.getString("duration"));
	return d.getMs();
}

// (hh:)mm:ss(.nnn)
void FileSet::setDurationStr(const std::string duration)
{
	std::string s;
	if (duration.size() > 0)
	{
		Duration d;
		if (d.parse(duration)) s = d.toString();
	}
	properties_.setString("duration", s);
}

std::string FileSet::getDurationStr() const
{
	std::string p = properties_.getString("duration");
	if (p.size() > 0)
	{
		Duration d;
		if (d.parse(p)) p = d.toString();
	}
	return p;
}

std::string FileSet::getLastTime() const
{
	return properties_.getString("lasttime");
}

std::string FileSet::getTimes() const
{
	return properties_.getString("times");
}

float FileSet::getMaxVol() const
{
	return properties_.getFloat("maxvol");
}

std::string FileSet::getMaxVolStr() const
{
	return properties_.getString("maxvol");
}

void FileSet::setMaxVol(const float maxvol)
{
	char buf[100];
	snprintf(buf, sizeof(buf) / sizeof(char), "%.2f", maxvol);
	properties_.setString("maxvol", std::string(buf));
}

float FileSet::getAverageVol() const
{
	return properties_.getFloat("averagevol");
}

std::string FileSet::getAverageVolStr() const
{
	return properties_.getString("averagevol");
}

void FileSet::setAverageVol(const float averagevol)
{
	char buf[100];
	snprintf(buf, sizeof(buf) / sizeof(char), "%.2f", averagevol);
	properties_.setString("averagevol", std::string(buf));
}

void FileSet::setIsSelected(const bool selected)
{
	properties_.setString("selected", selected ? "X" : "");
}

std::string FileSet::getIsSelected() const
{
	return properties_.getString("selected");
}













