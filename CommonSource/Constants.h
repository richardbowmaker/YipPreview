/*
 * Constants.h
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#ifndef COMMON_CONSTANTS_H_
#define COMMON_CONSTANTS_H_

#include <string>
#include <wx/wx.h>

#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
#endif


class Constants
{
public:
	Constants();
	virtual ~Constants();

    static void initialise();

    static std::string ffmpeg;
    static std::string ffmpegEnd;
    static std::string title;
    static std::string workingDir;
    static std::string filesDir;
    static std::string propertiesCache;
    static bool previewMode;	// image browser preview mode on
	static int imageBrowserSize;  // no. of images in browser, vertically or horizontally (always the same)
	static int imageBrowserSizeMin;  
	static int imageBrowserSizeMax;
    static std::string veracrypt;
    static std::string pathSeparator;

    // colours
    static wxColour blue;
    static wxColour lightBlue;
    static wxColour grey;
    static wxColour white;
    static wxColour systemBackground;

};

#endif /* COMMON_CONSTANTS_H_ */
