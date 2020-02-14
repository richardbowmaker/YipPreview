
#include "FileSet.h"

#include <filesystem>

#include "Logger.h"
#include "Utilities.h"


FileSet::FileSet()
{
}

FileSet::~FileSet()
{
}

std::wstring FileSet::filenameToId(const std::wstring filename)
{
	return FU::getPathNoExt(filename);
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
	std::wstring ext = FU::getExt(filename);

	if (ext.compare(L"jpg") == 0 || ext.compare(L"jpeg") == 0 )
		setImage(filename);
	else if (ext.compare(L"mp4") == 0 || ext.compare(L"avi") == 0 )
		setVideo(filename);
	else if (ext.compare(L"lnk") == 0)
		setLink(filename);
	else
		Logger::error(L"FileSet::setFilename, unhandled file type");
}

std::wstring FileSet::getId() const
{
	return id_;
}

std::wstring FileSet::getImage() const
{
	return image_;
}

void FileSet::setImage(const std::wstring filename)
{
	std::wstring s = FU::getPathNoExt(filename);
	if (id_.size() == 0)
		id_ = s;
	else if (!s.compare(id_) == 0)
		Logger::error(L"FileSet::setImageFilename, id = %ls, fn = %ls", id_.c_str(), filename.c_str());
	image_ = filename;
}

std::wstring FileSet::getVideo() const
{
	return video_;
}

void FileSet::setVideo(const std::wstring filename)
{
	std::wstring s = FU::getPathNoExt(filename);
	if (id_.size() == 0)
		id_ = s;
	else if (!s.compare(id_) == 0)
		Logger::error(L"FileSet::setVideoFilename, id = %ls, fn = %ls", id_.c_str(), filename.c_str());
	video_ = filename;
}

std::wstring FileSet::getLink() const
{
	return link_;
}

void FileSet::setLink(const std::wstring filename)
{
	std::wstring s = FU::getPathNoExt(filename);
	if (id_.size() == 0)
		id_ = s;
	else if (!s.compare(id_) == 0)
		Logger::error(L"FileSet::setLinkFilename, id = %ls, fn = %ls", id_.c_str(), filename.c_str());
	link_ = filename;
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

std::wstring FileSet::toString() const
{
	return std::wstring(L"ID = ") + id_ + std::wstring(L", ") + typesToString();
}






