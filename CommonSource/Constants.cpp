/*
 * Constants.cpp
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#include "Constants.h"

#ifdef WINDOWS_BUILD
    std::wstring Constants::ffmpeg{LR"(D:\Projects\WxWidgets\YipPreview\ffmpeg.exe)"};
    std::wstring Constants::ffmpegEnd{L" 2>&1"};
    std::wstring Constants::filesDir{LR"(\Files\All)"};
    std::wstring Constants::propertiesCache{LR"(\PropertyCache.txt)"};
    std::wstring Constants::veracrypt{LR"("C:\Program Files\VeraCrypt\VeraCrypt.exe")"};
    std::wstring pathSeparator{LR"(\)"};

#elif LINUX_BUILD
	std::wstring Constants::ffmpeg{LR"(/usr/bin/ffmpeg)"};
    std::wstring Constants::ffmpegEnd{L" &2>1"};
    std::wstring Constants::filesDir{LR"(/Files/All)"};
    std::wstring Constants::propertiesCache{LR"(/PropertyCache.txt)"};
    std::wstring Constants::veracrypt{LR"(/usr/bin/veracrypt)"};
    std::wstring pathSeparator{LR"(/)"};
#endif

std::wstring Constants::title{L"YipPreview"};
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
