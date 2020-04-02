/*
 * Constants.cpp
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#include "Constants.h"

#ifdef WINDOWS_BUILD
    std::string Constants::ffmpeg{R"(D:\Projects\WxWidgets\YipPreview\ffmpeg.exe)"};
    std::string Constants::ffmpegEnd{" 2>&1"};
    std::string Constants::filesDir{R"(\Files\All)"};
    std::string Constants::propertiesCache{R"(\PropertyCache.txt)"};
    std::string Constants::veracrypt{R"("C:\Program Files\VeraCrypt\VeraCrypt.exe")"};
    std::string pathSeparator{R"(\)"};

#elif LINUX_BUILD
	std::string Constants::ffmpeg{R"(/usr/bin/ffmpeg)"};
    std::string Constants::ffmpegEnd{" &2>1"};
    std::string Constants::filesDir{R"(/Files/All)"};
    std::string Constants::propertiesCache{R"(/PropertyCache.txt)"};
    std::string Constants::veracrypt{R"(/usr/bin/veracrypt)"};
    std::string pathSeparator{R"(/)"};
#endif

std::string Constants::title{"YipPreview"};
std::string Constants::workingDir;
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
