
#include "FileSet.h"

#include <filesystem>
#include <regex>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "Logger.h"
#include "Utilities.h"


FileSet::FileSet()
{
}

FileSet::FileSet(const VolumeWRefT &volume, const std::wstring file)
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

std::wstring FileSet::getFileStem() const
{
	return FU::getFileStem(id_);
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

void FileSet::toLogger()
{
	Logger::info(L"File Set");

	Logger::info(L"\tID %ls",         id_.c_str());
	Logger::info(L"\tShortname %ls",  short_.c_str());
	Logger::info(L"\tImage file %ls", image_.c_str());
	Logger::info(L"\tVideo file %ls",  video_.c_str());
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











