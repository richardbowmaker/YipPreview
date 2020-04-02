

#ifdef WINDOWS_BUILD
	#include <cstdio>
	#include <iostream>
	#include <memory>
	#include <stdexcept>
	#include <string>
	#include <array>
	#include <utility>
	#include <string.h>
	#include <errno.h>
	#include <sstream>
	#include <fcntl.h>
	#include <time.h>
	#include <future>
	#include <atomic>
	#include <string>
	#include <thread>
	#include <wx/wx.h>
	#include <wx/thread.h>
	#include <windows.h> 
	#include <tchar.h>
	#include <stdio.h> 
	#include <strsafe.h>
	#include <tchar.h>
#elif LINUX_BUILD
	#include <sys/wait.h>
	#include <sys/types.h>
	#include <pthread.h>
	#include <cstdio>
	#include <iostream>
	#include <memory>
	#include <stdexcept>
	#include <string>
	#include <array>
	#include <utility>
	#include <string.h>
	#include <errno.h>
	#include <sstream>
	#include <fcntl.h>
	#include <time.h>
	#include <future>
	#include <atomic>
	#include <sstream>
	#include <string>
	#include <thread>
	#include <unistd.h>
	#include <wx/wx.h>
	#include <wx/thread.h>
#endif


#include "_Types.h"
#include "FileSetManager.h"
#include "Logger.h"
#include "Tryout.h"
#include "Utilities.h"
#include "ShellExecute.h"
#include "Volume.h"
#include "VolumeManager.h"

// STD STRING 1


//template<typename T>
//std::string toString(T t) { return " unknown "; }
//
//template<>
//std::string toString(std::string t) { return " string "; }
//
//template<>
//std::string toString(int t) { return " integer "; }
//
//
//template<typename T, typename... Args>
//std::string toString(T first, Args... args)
//{
//	return toString(first) + toString(args...);
//}


#include <fmt/core.h>




template<typename T>
std::string toString(T t) { return "??"; }

template<>
std::string toString(const std::string &t) { return t; }

template<>
std::string toString(const int t)
{
	char buf[100];
	sprintf(buf, "%d", t);
	return std::string(buf);
}

std::string toString(const char* s)
{
	return std::string(s);
}

template<typename T>
void logss(std::stringstream &ss, const std::string &format, T v)
{
	ss << toString(v);
}

template<typename T, typename... Args>
void logss(std::stringstream &ss, const std::string &format, T v, Args... args)
{
	ss << toString(v);
	logss(ss, format, args...);
}

template<typename... Args>
std::string log(const std::string &format, Args... args)
{
	std::stringstream ss;
	logss(ss, format, args...);
	return ss.str();
}


void TryOut::tryout(const FileSetT fs)
{
//	std::string s = toString(3, 2, std::string("123"), 0.4f);

//	std::string s = log("", 3, 4, 0.4f, "34");
//
//	fmt::format(format_str, args)
//
//
//	std::string message = fmt::wformat(L"The answer is {}", 42);
//
//


	return;
}



void TryOut::AsyncShell(wxEvtHandler *handler)
{

	ShellExecuteResult result;


	//	ShellExecute::shellAsync(LR"(D:\Projects\WxWidgets\YipPreview\StdErrOutWin\Debug\StdErrOutWin.exe)", ShelExecuteHandler, 1);
	//	ShellExecute::shellAsync(LR"(D:\_Ricks\c#\ZiPreview\Executable\ffmpeg.exe)", ShelExecuteHandler, 2);

//	ShellExecute::shellSync(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)", result);
//	Logger::info(L"Shell result %ls", result.toString().c_str());
//
//	ShellExecute::shellAsync(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)", ShelExecuteHandler, 3);
//	ShellExecute::shellAsync(LR"(D:\Projects\WxWidgets\YipPreview\StdErrOutWin\Debug\StdErrOutWin.exe)", ShelExecuteHandler, 4);
//	ShellExecute::shellAsyncGui(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)", handler);
//
//	return;

	/////////////////////////

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell launched: firefox");
//	ShellExecute::shell(L"/usr/bin/firefox");
//	ShellExecute::shell(L"/bin/notepadqq");

	Logger::info(L"----------------------------------");
	Logger::info(L"Shell launched: StdOutErr");
	ShellExecute::shellSync(L"/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/StdOutErr/Debug/StdOutErr", result);
	Logger::info(result.toString().c_str());


// /bin/ffmpeg -i /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/f3.mp4 -af "volumedetect" -vn -sn -dn -f null NUL &2>1
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: ffmpeg -i /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/f3.mp4 -af \"volumedetect\" -vn -sn -dn -f null NUL");
//	ShellExecute::shellSync(L"/bin/ffmpeg -i /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/f3.mp4 -af \"volumedetect\" -vn -sn -dn -f null NUL &2>1", result, 10000);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: ffmpegbat");
//	ShellExecute::shellSync(L"/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/ffmpegbat &2>1", result, 2000);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: test.bat");
//	ShellExecute::shellSync(L"/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/test.bat", result);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellSync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", result, 1000);
//	Logger::info(result.toString().c_str());
//
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: /bin/nonexistentprogram");
//	ShellExecute::shellSync(L"/bin/nonexistentprogram", result, 1000);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: /bin/notepadqq");
//	ShellExecute::shellSync(L"/bin/notepadqq", result, 2000);
//	Logger::info(result.toString().c_str());
//	result.killChildProcess();

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async launched: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al, no event handler");
//	ShellExecute::shellAsync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async launched: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellAsync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", &ShelExecuteHandler, 200, nullptr, 10000);

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async launched: /bin/nonexistentprogram");
//    ShellExecute::shellAsync(L"/bin/nonexistentprogram", &ShelExecuteHandler, 201, nullptr, 10000);
//
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async launched: /bin/notepadqq");
//    ShellExecute::shellAsync(L"/bin/notepadqq", &ShelExecuteHandler, 202, nullptr, 5000);


// GUI callbacks

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched any: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al, no event handler");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched any: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", handler, wxID_ANY, 3001);

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched 1: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", handler, 1, 3002);

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched 2: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", handler, 2, 3003);

	//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched: /bin/nonexistentprogram");
//	ShellExecute::shellAsyncGui(L"/bin/nonexistentprogram", 5000, 301, nullptr, handler);
//
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched: /bin/notepadqq");
//	ShellExecute::shellAsyncGui(L"/bin/notepadqq", 5000, 302, nullptr, handler);
}



