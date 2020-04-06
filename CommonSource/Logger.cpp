#include "Logger.h"

#include <fmt/core.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <wx/wx.h>

#include "Utilities.h"

// static members
Logger::LevelT Logger::level_ = Logger::Error;
Logger* Logger::lbox_	= nullptr;
long Logger::tid_		= 0;
long Logger::lineNo_	= 0;
bool Logger::lcEnable_	= false;
bool Logger::ideOut_	= false;
long Logger::tzero_		= 0;
bool Logger::showTime_	= false;

constexpr int kBufferMax = 40000;
constexpr int kMaxLines = 1000;

wxDECLARE_EVENT(wxEVT_LOGGER_EVENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_LOGGER_EVENT, wxCommandEvent);

Logger::Logger() : wxListBox()
{
}

Logger::Logger(wxWindow* parent, wxWindowID id) :
	wxListBox(parent, id, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE)
{
	tid_ = Utilities::getThreadId();
	lbox_ = this;
	Bind(wxEVT_LOGGER_EVENT, &Logger::onLogger, this);
}

wxIMPLEMENT_DYNAMIC_CLASS(Logger, wxListBox);

void Logger::onLogger(wxCommandEvent& event)
{
	// add to message box
	wxListBox* lb = dynamic_cast<wxListBox*>(event.GetEventObject());
	if (lb != nullptr) 
		appendLb(
			static_cast<Logger::LevelT>(event.GetInt()), 
			std::string(event.GetString()));
	event.Skip();
}

void Logger::setLevel(const LevelT level)
{
	level_ = level;
}

void Logger::enableIdeOutput(const bool enable)
{
	ideOut_ = enable;
}

void Logger::enableTime(const bool enable)
{
	showTime_ = enable;
}
void Logger::clear()
{
	lbox_->Clear();
}

void Logger::enableLineCount(const bool enable)
{
	lcEnable_ = enable;
}

void Logger::append(const LevelT level, const char* text)
{

	// first call, set ticks at time zero
	long t = Utilities::getMsCounter();
	if (tzero_ == 0) tzero_ = t;

	std::stringstream es;

	// show time
	if (showTime_) es << fmt::format("{:>08}: ", t - tzero_);

	// show line count
	if (lcEnable_) es << fmt::format("{:>05}: ", lineNo_++);

	// show level
	switch (level)
	{
	case Error:		es << "Error   : "; break;
	case Warning:	es << "Warning : "; break;
	case Info:		es << "Info    : "; break;
	}

	es << text;

	if (ideOut_)
	{
// show message
#ifdef WINDOWS_BUILD
		OutputDebugStringA(es.str().c_str());
		OutputDebugStringA("\n");
#elif LINUX_BUILD
		std::cout << es.str();
#endif
	}

	appendLb(level, es.str());
}

void Logger::appendLb(const LevelT level, std::string text)
{
	if (lbox_ == nullptr) return;

	if (tid_ == Utilities::getThreadId())
	{
		// add to list box
		lbox_->Append(text);
		lbox_->Refresh();
		lbox_->Update();

		// limit the number of messages displayed
		while (lbox_->GetCount() > kMaxLines)
			lbox_->Delete(0);

		// make last line visible
		lbox_->SetSelection(lbox_->GetCount() - 1);
		lbox_->Deselect(lbox_->GetCount() - 1);
		lbox_->Refresh();
		lbox_->Update();
	}
	else
	{
		// post event to GUI thread
		wxCommandEvent evt;
		evt.SetString(wxString(text));
		evt.SetEventObject(lbox_);
		evt.SetInt(static_cast<int>(level));
		lbox_->GetEventHandler()->AddPendingEvent(evt);
	}
}

std::string Logger::systemErrorToString(const int err)
{
#ifdef WINDOWS_BUILD
	std::string es;
	char* errStr;
	if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
		reinterpret_cast<LPSTR>(&errStr),
		0,
		NULL))
	{
		es = errStr;
		LocalFree(errStr);
	}
#elif LINUX_BUILD
	std::string es(strerror(err));
#endif
	return es;
}

/*
void CLogger::LogSourceSystemError(const char* file, const int line, const int err, const char* format, ...)
{
	char buff[1024];
	snprintf(buff, sizeof(buff), " [%s: %d] {%s:%d}", strerror(err), err, file, line);
	va_list vl;
	va_start(vl, format);
	Log(Error, (std::string(format) + std::string(buff)).c_str(), vl);
}

void CLogger::LogSourceError(const char* file, const int line, const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	LogSource(file, line, Error, format, vl);
}

void CLogger::LogSourceWarning(const char* file, const int line, const char* format, ...)
{
	if (_level <= Warning)
	{
		va_list vl;
		va_start(vl, format);
		LogSource(file, line, Warning, format, vl);
	}
}

void CLogger::LogSourceInfo(const char* file, const int line, const char* format, ...)
{
	if (_level <= Info)
	{
		va_list vl;
		va_start(vl, format);
		LogSource(file, line, Info, format, vl);
	}
}

void CLogger::LogSource(const char* file, const int line, const LogT level, const char* format, va_list vl)
{
	if (level >= _level)
	{
		char buff[1024];
		snprintf(buff, sizeof(buff), " {%s:%d}", file, line);
		Log(level, (std::string(format) + std::string(buff)).c_str(), vl);
	}
}

void CLogger::LogBytes(const LogT level, const uint8_t* bytes, const int len)
{
	if (level >= _level)
	{
		const uint8_t* pb = bytes;
		char buff[100];
		char* pbuff = buff;
		char* pasc = buff + 52;
		int i = 0;
		int d;

		while (i < len)
		{
			if (i % 16 == 0) memset(buff, ' ', sizeof(buff));

			// hex
			d = (*pb / 16); *pbuff++ = d > 9 ? 'a' + d - 10 : '0' + d;
			d = (*pb % 16); *pbuff++ = d > 9 ? 'a' + d - 10 : '0' + d;
			++i;
			pbuff += (i % 4 == 0) ? 2 : 1;

			// ascii
			if (*pb >= 32 && *pb < 127)* pasc++ = *pb; else *pasc++ = '.';
			*pasc = 0;

			++pb;

			if (i % 16 == 0 || i == len)
			{
				_log.push_back(LogEntryT(level, std::string(buff)));
				if (_echo) std::cout << buff << std::endl;
				pbuff = buff;
				pasc = buff + 52;
			}
		}
	}
}

*/








