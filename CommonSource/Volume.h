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

class Volume;

using VolumeT = std::shared_ptr<Volume>;

class Volume
{
public:

	Volume(std::wstring root);
	virtual ~Volume();

	std::wstring getRoot() const;
	bool getIsMounted() const;
	std::wstring getPropertiesFile() const;
	std::wstring getFilesDirectory() const;

private:

	std::wstring root_;
	bool isMounted_;

};

#endif /* COMMON_VOLUME_H_ */
