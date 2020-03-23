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
    std::wstring Constants::filesDir = std::wstring(LR"(\Files\All)");
    std::wstring Constants::propertiesCache = std::wstring(LR"(\PropertyCache.txt)");
#elif LINUX_BUILD
	std::wstring Constants::ffmpeg = std::wstring(LR"(/usr/bin/ffmpeg)");
    std::wstring Constants::ffmpegEnd = std::wstring(L" &2>1");
    std::wstring Constants::filesDir = std::wstring(LR"(/Files/All)");
    std::wstring Constants::propertiesCache = std::wstring(LR"(/PropertyCache.txt)");
#endif

std::wstring Constants::title = std::wstring(L"YipPreview");
std::wstring Constants::workingDir;
bool Constants::previewMode = false;
int Constants::imageBrowserSize = 2;
int Constants::imageBrowserSizeMin = 1;
int Constants::imageBrowserSizeMax = 4;

wxColour Constants::white;
wxColour Constants::blue;
wxColour Constants::grey;
wxColour Constants::lightBlue;
wxColour Constants::systemBackground;

Constants::Constants()
{
}

Constants::~Constants()
{
}

void Constants::initialise()
{
    wxColourDatabase colours;
    white = colours.Find(L"WHITE");
    blue = colours.Find(L"BLUE");
    grey = colours.Find(L"GREY");
    lightBlue = colours.Find(L"LIGHT BLUE");

    systemBackground = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
}
