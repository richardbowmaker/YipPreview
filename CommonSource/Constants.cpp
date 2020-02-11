/*
 * Constants.cpp
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#include "Constants.h"

#ifdef WINDOWS_BUILD
	std::wstring Constants::ffmpeg = std::wstring(L"ffmpeg");
#elif LINUX_BUILD
	std::wstring Constants::ffmpeg = std::wstring(LR"(/usr/bin/ffmpeg)");
#endif

Constants::Constants()
{
	// TODO Auto-generated constructor stub

}

Constants::~Constants()
{
	// TODO Auto-generated destructor stub
}

