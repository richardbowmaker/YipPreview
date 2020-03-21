#ifndef FILESET_H
#define FILESET_H

#include <map>
#include <memory>
#include <string>
#include <time.h>
#include <vector>

#include "_Types.h"
#include "FileProperties.h"
#include "Volume.h"

class FileSet
{
public:

	FileSet();
	FileSet(const VolumeWRefT &volume, const std::wstring file);
	virtual ~FileSet();

	void set(const std::wstring file);
	std::wstring getId() const;
	std::wstring getImage() const;
	void setImage(const std::wstring filename);
	std::wstring getVideo() const;
	void setVideo(const std::wstring filename);
	std::wstring getLink() const;
	void setLink(const std::wstring filename);
	std::wstring toString() const;
	bool hasImage() const;
	bool hasVideo() const;
	bool hasLink() const;
	std::wstring typesToString() const;
	std::wstring getShortName() const;
	std::wstring getFileStem() const;
	void setDurationMs(const long ms);
	long getDurationMs() const;
	void setDurationStr(const std::wstring duration);
	std::wstring getDurationStr() const;
	std::wstring getLastTime() const;
	std::wstring getTimes() const;
	void toLogger();

	// properties
	FileProperties& properties();

	static bool isValidType(const std::wstring filename);
	static std::wstring filenameToId(const std::wstring filename);

private:

	enum TypeT { InvalidFile, ImageFile, LinkFile, VideoFile };
	static TypeT filenameToType(const std::wstring filename);

	bool setId(const std::wstring filename);

	std::wstring id_;
	std::wstring short_;
	std::wstring image_;
	std::wstring video_;
	std::wstring link_;

	FileProperties properties_;
	VolumeWRefT volume_;
};


#endif // FILESET_H
