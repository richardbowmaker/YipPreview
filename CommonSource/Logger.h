#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <sys/types.h>
#include <wx/wx.h>

#include "Events.h"

// This class is a hybrid wxListBox control and static class for providing
// thread safe global static logging functions.
// There is only intended to be one instance of the logger in an application.

class wxLoggerEvent;

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


	static void setLevel(LevelT level);
	static LevelT getLevel();
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

	void OnLogger(wxLoggerEvent& event);

	static void log(const LevelT level, const wchar_t* format, va_list vl);
	static void append(const wchar_t* text);

	static LevelT level_;
	static Logger* this_;
	static long tid_;		// main GUI thread ID

	wxDECLARE_DYNAMIC_CLASS(Logger);
	wxDECLARE_EVENT_TABLE();
};

//----------------------------------------------------------------------------
// logger event, used to send log data from worker thread to logger list box

class wxLoggerEvent : public wxCommandEvent
{
public:
	wxLoggerEvent();
	wxLoggerEvent(const wxLoggerEvent &other);

	virtual wxEvent *Clone() const;

	int getLevel() const;
	void setLevel(int level);

	DECLARE_DYNAMIC_CLASS(wxLoggerEvent)

private:

	int level_;
};


typedef void (wxEvtHandler::*wxLoggerEventFunction)(wxLoggerEvent&);
#define wxLoggerEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxLoggerEventFunction, func)


wxDECLARE_EVENT(wxEVT_LOGGER_EVENT, wxLoggerEvent);

// for use in message maps
#define EVT_LOGGER_EVENT_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    	wxEVT_LOGGER_EVENT, id, wxID_ANY, \
		wxLoggerEventHandler(fn), \
        (wxObject *) NULL \
    ),



#endif


