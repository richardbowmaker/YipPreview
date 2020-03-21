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

	Volume(std::wstring root);
	virtual ~Volume();

	std::wstring getRoot() const;
	bool getIsMounted() const;
	std::wstring getPropertiesFile() const;
	std::wstring getFilesDirectory() const;
	void addFileSet(FileSetT &fileset);
	void readProperties();
	void writeProperties();
	void toLogger() const;

private:

	std::wstring root_;
	bool isMounted_;
	FileSetCollT filesets_;
};

#endif /* COMMON_VOLUME_H_ */
