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

	int check_{123456};

	FileSet();
	FileSet(const Volume *volume, const std::string file);
	virtual ~FileSet();

	void set(const std::string file);
	std::string getId() const;
	std::string getImage() const;
	void setImage(const std::string filename);
	std::string getVideo() const;
	void setVideo(const std::string filename);
	std::string getLink() const;
	void setLink(const std::string filename);
	std::string toString() const;
	bool hasImage() const;
	bool hasVideo() const;
	bool hasLink() const;
	std::string typesToString() const;
	std::string getShortName() const;
	const Volume* getVolume() const;
	void setDurationMs(const long ms);
	long getDurationMs() const;
	void setDurationStr(const std::string duration);
	std::string getDurationStr() const;
	std::string getLastTime() const;
	std::string getTimes() const;
	float getMaxVol() const;
	std::string getMaxVolStr() const;
	void setMaxVol(const float maxvol);
	float getAverageVol() const;
	std::string getAverageVolStr() const;
	void setAverageVol(const float averagevol);
	void setIsSelected(const bool selected);
	std::string getIsSelected() const;
	bool sort(const ColT col, FileSetT &other, const bool ascending);
	void toLogger();


	// properties
	FileProperties& properties();

	static bool isValidType(const std::string filename);
	static std::string filenameToId(const std::string filename);

private:

	enum TypeT { InvalidFile, ImageFile, LinkFile, VideoFile };
	static TypeT filenameToType(const std::string filename);

	bool setId(const std::string filename);

	std::string id_;
	std::string short_;
	std::string image_;
	std::string video_;
	std::string link_;

	FileProperties properties_;
	const Volume* volume_;
};


#endif // FILESET_H
