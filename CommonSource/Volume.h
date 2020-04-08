/*
 * Volume.h
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_VOLUME_H_
#define COMMON_VOLUME_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "_Types.h"
#include "FileSet.h"

class Volume
{
public:

	Volume(const std::string file, const bool isMountable);
	virtual ~Volume();

	std::string getFile() const;
	std::string getMount() const;
	std::string getShortName() const;
	bool getIsMountable() const;
	bool getIsMounted() const;
	bool getIsSelected() const;
	void setIsSelected(const bool isSelected);
	std::string getPropertiesFile() const;
	std::string getFilesDirectory() const;
	FileSetCollT getFileSets() const;
	bool hasFileSets() const;
	void readProperties();
	void writeProperties();
	bool mount(const std::string &m, const std::string &password);
	bool unmount();
	void loadFiles();
	void clearFiles();
	std::string toString() const;
	void toLogger() const;
	std::tuple<bool, long long, long long> getFreeSpace() const; // (succeeded, total space, free space)
	void setIsDirty(const bool dirty);
	bool getIsdirty() const;
	void addFileSet(FileSetT &fs);

private:

	std::string file_;
	std::string mount_;
	std::string short_;
	bool isMountable_;
	bool isMounted_;
	bool isDirty_;
	bool isSelected_;
	FileSetCollT fileSets_;
};

#endif /* COMMON_VOLUME_H_ */
