#ifndef FILESET_H
#define FILESET_H

#include <memory>
#include <string>

class FileSet;
using FileSetT = std::shared_ptr<FileSet>;

class FileSet
{
public:

	FileSet();
	virtual ~FileSet();

	void set(const std::wstring filename);
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
};


#endif // FILESET_H
