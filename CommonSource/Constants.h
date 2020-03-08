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

class Constants
{
public:
	Constants();
	virtual ~Constants();

    static void initialise();

    static std::wstring ffmpeg;
    static std::wstring ffmpegEnd;
    static std::wstring title;

    // colours
    static wxColour blue;
    static wxColour lightBlue;
    static wxColour grey;
    static wxColour white;
    static wxColour systemBackground;

};

#endif /* COMMON_CONSTANTS_H_ */
