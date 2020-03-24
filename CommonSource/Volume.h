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

	std::wstring getMount() const;
	bool getIsMountable() const;
	bool getIsMounted() const;
	std::wstring getPropertiesFile() const;
	std::wstring getFilesDirectory() const;
	void addFileSet(FileSetT &fileset);
	void readProperties();
	void writeProperties();
	bool mount(std::wstring &mount);
	void unmount();

	void toLogger() const;

private:

	std::wstring file_;
	std::wstring mount_;
	bool isMountable_;
	bool isMounted_;
	bool isDirty_;
	bool isSelected_;
	FileSetCollT filesets_;
};

#endif /* COMMON_VOLUME_H_ */
