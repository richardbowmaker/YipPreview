/*
 * Constants.cpp
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#include "Constants.h"

#ifdef WINDOWS_BUILD
    std::wstring Constants::ffmpeg = std::wstring(LR"(D:\Projects\WxWidgets\YipPreview\ffmpeg.exe)");
    std::wstring Constants::ffmpegEnd = std::wstring(L" 2>&1");
#elif LINUX_BUILD
	std::wstring Constants::ffmpeg = std::wstring(LR"(/usr/bin/ffmpeg)");
    std::wstring Constants::ffmpegEnd = std::wstring(L" &2>1");
#endif

Constants::Constants()
{
	// TODO Auto-generated constructor stub

}

Constants::~Constants()
{
	// TODO Auto-generated destructor stub
}

