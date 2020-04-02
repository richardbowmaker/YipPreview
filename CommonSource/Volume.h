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
#include <vector>

#include "_Types.h"
#include "FileSet.h"

class Volume
{
public:

	Volume(const std::wstring file, const bool isMountable);
	virtual ~Volume();

	std::wstring getFile() const;
	std::wstring getMount() const;
	std::wstring getShortName() const;
	bool getIsMountable() const;
	bool getIsMounted() const;
	bool getIsSelected() const;
	void setIsSelected(const bool isSelected);
	std::wstring getPropertiesFile() const;
	std::wstring getFilesDirectory() const;
	FileSetCollT getFileSets() const;
	bool hasFileSets() const;
	void readProperties();
	void writeProperties();
	bool mount(const std::wstring &m, const std::wstring &password);
	bool unmount();
	void loadFiles();
	void clearFiles();
	std::wstring toString() const;
	void toLogger() const;

private:

	std::wstring file_;
	std::wstring mount_;
	std::wstring short_;
	bool isMountable_;
	bool isMounted_;
	bool isDirty_;
	bool isSelected_;
	FileSetCollT fileSets_;
};

#endif /* COMMON_VOLUME_H_ */
