/*
 * Constants.cpp
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#include "Constants.h"

#ifdef WINDOWS_BUILD
    std::string Constants::ffmpeg			{R"(D:\Projects\WxWidgets\YipPreview\ffmpeg.exe)"};
    std::string Constants::ffmpegEnd		{" 2>&1"};
    std::string Constants::videoPlayer		{R"("C:\Program Files\VideoLAN\VLC\vlc.exe" -f )"};
    std::string Constants::filesDir			{R"(\Files\All)"};
    std::string Constants::propertiesCache	{R"(\PropertyCache.txt)"};
    std::string Constants::veracrypt		{R"("C:\Program Files\VeraCrypt\VeraCrypt.exe")"};
    std::string Constants::pathSeparator	{R"(\)"};
#elif LINUX_BUILD
	std::string Constants::ffmpeg			{R"(/usr/bin/ffmpeg)"};
    std::string Constants::ffmpegEnd		{" &2>1"};
	std::string Constants::videoPlayer		{R"(/usr/bin/vlc -f )"};
    std::string Constants::filesDir			{R"(/Files/All)"};
    std::string Constants::propertiesCache	{R"(/PropertyCache.txt)"};
    std::string Constants::veracrypt		{R"(/usr/bin/veracrypt)"};
    std::string Constants::pathSeparator	{R"(/)"};
#endif

std::string Constants::title{"YipPreview"};
std::string Constants::workingDir;
bool Constants::previewMode = false;
int Constants::imageBrowserSize = 2;
int Constants::imageBrowserSizeMin = 1;
int Constants::imageBrowserSizeMax = 4;

long long Constants::minDiskFreeSpace = 50 * 1024 * 1024;	// 50MB
std::string Constants::lastDirectory;

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
