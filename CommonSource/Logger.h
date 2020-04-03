#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <sys/types.h>
#include <wx/wx.h>

#include "_Types.h"
#include <fmt/core.h>

// This class is a hybrid wxListBox control and static class for providing
// thread safe global static logging functions.
// There is only intended to be one instance of the logger in an application.

class Logger : public wxListBox
{
public:

	enum LevelT
	{
		Error = 3,
		Warning = 2,
		Info = 1
	};


	Logger();
	Logger (wxWindow* parent, wxWindowID id);
	//virtual ~Logger();

	static void setLevel(const LevelT level);
	static void enableIdeOutput(const bool enable);
	static void enableTime(const bool enable);
	static void enableLineCount(const bool enable);

	static void clear();

	template<typename... Args>
	static void log(const LevelT level, const char* format, Args... args)
	{
		if (level >= level_) append(level, fmt::format(format, args...).c_str());
	};

	template<typename... Args>
	static void systemError(const int err, const char* format, Args... args)
	{
		char* errStr;
#ifdef WINDOWS_BUILD
		if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
			reinterpret_cast<LPWSTR>(&errStr),
			0,
			NULL)) return;
		std::string ws(errStr);
		LocalFree(errStr);
#elif LINUX_BUILD
		errStr = strerror(err);
#endif
		std::string es = fmt::format(" [{0}:{1}]", err, errStr);
		log(Error, (std::string(format) + es).c_str(), args...);
#ifdef WINDOWS_BUILD
		LocalFree(errStr);
#endif
	};

	template<typename... Args>
		static void error(const char* format, Args... args)
		{
			log(Error, format, args...);
		};
		
	template<typename... Args>
		static void warning(const char* format, Args... args)
		{
			if (level_ <= Warning)
				log(Warning, format, args...);
		};

	template<typename... Args>
		static void info(const char* format, Args... args)
		{
			if (level_ <= Info)
				log(Info, format, args...);
		};

	template<typename... Args>
	static void error(const StringCollT& strings, const char* format, Args... args)
	{
		log(error, format, args...);
		for (auto s : strings) append(Info, s.c_str());
	};

	template<typename... Args>
	static void warning(const StringCollT& strings, const char* format, Args... args)
	{
		if (level_ <= Warning)
		{
			log(warning, format, args...);
			for (auto s : strings) append(Info, s.c_str());
		}
	};
		
	template<typename... Args>
	static void info(const StringCollT& strings, const char* format, Args... args)
	{
		if (level_ <= Info)
		{
			log(Info, format, args...);
			for (auto s : strings) append(Info, s.c_str());
		}
	};

	template<typename... Args>
	static bool test(const bool result, const char* format, Args... args)
	{
		if (!result)
			log(Error, format, args...);
		return result;
	};

	//static void logSourceSystemError(const char* file, const int line, const int err, const char* format, ...); // err = errno
	//static void logSourceError(const char* file, const int line, const char* format, ...);
	//static void logSourceWarning(const char* file, const int line, const char* format, ...);
	//static void logSourceInfo(const char* file, const int line, const char* format, ...);

	//static void logBytes(const LevelT level, const uint8_t* bytes, const int len);
	//static void logBytes(const LevelT level, const std::string str);


private:

	void onLogger(wxCommandEvent& event);

	static void append(const LevelT level, const char* text);
	static void appendLb(const LevelT level, std::string text);

	static LevelT level_;
	static Logger* lbox_;
	static long tid_;		// main GUI thread ID
	static long lineNo_;
	static bool lcEnable_;
	static bool ideOut_;
	static long tzero_;
	static bool showTime_;

	wxDECLARE_DYNAMIC_CLASS(Logger);
};

#endif


