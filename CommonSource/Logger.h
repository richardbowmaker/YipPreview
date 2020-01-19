#ifndef LOGGER_H
#define LOGGER_H

#include <wx/wx.h>
#include <string>

#include "Events.h"


class LoggerListBox : public wxListBox
{
	public:
		DECLARE_DYNAMIC_CLASS(LoggerListBox);

		LoggerListBox () {}
		LoggerListBox (wxWindow* parent, wxWindowID id)
			: wxListBox (parent, id) {}

		void OnLogger(wxLoggerEvent& event);

		wxDECLARE_EVENT_TABLE();
};


class Logger
{
public:

	virtual ~Logger();

	enum LevelT
	{
		Error = 3,
		Warning = 2,
		Info = 1
	};

	static void initialise(wxListBox* listBox, wxEvtHandler* event, LevelT level = LevelT::Error);
	static void setLevel(LevelT level);
	static Logger::LevelT getLevel();

	static void test();

	static void clear();
	static void log(const LevelT level, const wchar_t* format, ...);
	static void systemError(const int err, const wchar_t* format, ...); // err = errno
	static void error(const wchar_t* format, ...);
	static void warning(const wchar_t* format, ...);
	static void info(const wchar_t* format, ...);

	//static void logSourceSystemError(const char* file, const int line, const int err, const char* format, ...); // err = errno
	//static void logSourceError(const char* file, const int line, const char* format, ...);
	//static void logSourceWarning(const char* file, const int line, const char* format, ...);
	//static void logSourceInfo(const char* file, const int line, const char* format, ...);

	//static void logBytes(const LevelT level, const uint8_t* bytes, const int len);
	//static void logBytes(const LevelT level, const std::string str);


private:

	Logger();
	Logger(const Logger& other);
	Logger(const Logger&& other);

	static void log(const LevelT level, const wchar_t* format, va_list vl);
	static void append(const wchar_t* text);

	static LevelT level_;
	static wxListBox* listBox_;
	static wxEvtHandler* event_;


};


#endif


