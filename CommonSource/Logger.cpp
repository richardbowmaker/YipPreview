#include "Logger.h"

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

wxDECLARE_EVENT(wxEVT_LOGGER_EVENT, wxLoggerEvent);
//wxDEFINE_EVENT(wxEVT_LOGGER_EVENT, wxLoggerEvent);

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

void Logger::onLogger(wxLoggerEvent& event)
{
	// add to message box
	wxListBox* lb = dynamic_cast<wxListBox*>(event.GetEventObject());
	if (lb != nullptr) append(event.getLevel(), event.GetString().wc_str());
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

void Logger::append(const LevelT level, const wchar_t* text)
{
	wchar_t buf[kBufferMax];

	// first call, set ticks at time zero
	long t = Utilities::getMsCounter();
	if (tzero_ == 0) tzero_ = t;

	if (ideOut_)
	{
		int n = 0;

		// show time
		if (showTime_)
			n = swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"***** %08ld: ", t - tzero_);

		// show line count
		if (lcEnable_)
			n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%05ld: ", lineNo_);

		// show level
		switch (level)
		{
		case Error:
			n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls: ", L"Error  ");
			break;
		case Warning:
			n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls: ", L"Warning");
			break;
		case Info:
			n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls: ", L"Info   ");
			break;
		}

		// show message
		swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls\n", text);
#ifdef WINDOWS_BUILD
		OutputDebugString(buf);
#elif LINUX_BUILD
		std::cout << SU::wStrToStr(buf);
#endif
	}

	if (lbox_ == nullptr) return;

	if (tid_ == Utilities::getThreadId())
	{
		// in GUI thread, so update list box
		std::wstring str(text), temp;
		std::wstringstream wss(str);
		while(std::getline(wss, temp, L'\n'))
		{
			int n = 0;

			// display tick count
			if (showTime_)
				n = swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%08ld: ", t - tzero_);

			// show line number
			if (lcEnable_)
				n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%05ld: ", lineNo_++);

			// show level
			switch (level)
			{
			case Error: 
				n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls: ", L"Error  ");
				break;
			case Warning:
				n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls: ", L"Warning");
				break;
			case Info:
				n += swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls: ", L"Info   ");
				break;
			}

			// show message
			swprintf(&buf[n], (sizeof(buf) / sizeof(wchar_t)) - n, L"%ls", temp.c_str());

			// add to list box
			lbox_->Append(buf);
			lbox_->Update();
		}

		// limit the number of messages displayed
		while (lbox_->GetCount() > kMaxLines)
			lbox_->Delete(0);

		// make last line visible
		lbox_->SetSelection(lbox_->GetCount() - 1);
		lbox_->Deselect(lbox_->GetCount() - 1);
		lbox_->Update();
	}
	else
	{
		// post event to GUI thread
		wxLoggerEvent evt;
		evt.SetString(wxString(text));
		evt.SetEventObject(lbox_);
		lbox_->GetEventHandler()->AddPendingEvent(evt);
	}
}

void Logger::log(const LevelT level, const wchar_t* format, va_list vl)
{
	wchar_t buff[kBufferMax];
	vswprintf(buff, sizeof(buff) / sizeof(wchar_t), format, vl);
	append(level, buff);
}

void Logger::log(const LevelT level, const wchar_t* format, ...)
{
	if (level >= level_)
	{
		va_list vl;
		va_start(vl, format);
		log(level, format, vl);
	}
}

void Logger::systemError(const int err, const wchar_t* format, ...)
{
#ifdef WINDOWS_BUILD
	wchar_t* errStr;
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
		reinterpret_cast<LPWSTR>(&errStr),
		0,
		NULL)) return;
	std::wstring ws(errStr);
	LocalFree(errStr);
	wchar_t buff[kBufferMax];
	swprintf(buff, sizeof(buff) / sizeof(wchar_t), L" [%ls: %d]", ws.substr(0, ws.size() - 2).c_str(), err);
#elif LINUX_BUILD
	wchar_t buff[kBufferMax];
	swprintf(buff, sizeof(buff), L" [%ls: %d]", SU::strToWStr(strerror(err)).c_str(), err);
#endif
	va_list vl;
	va_start(vl, format);
	log(Error, (std::wstring(format) + std::wstring(buff)).c_str(), vl);
}

void Logger::error(const wchar_t* format, ...)
{
	va_list vl;
	va_start(vl, format);
	log(Error, format, vl);
}

void Logger::warning(const wchar_t* format, ...)
{
	if (level_ <= Warning)
	{
		va_list vl;
		va_start(vl, format);
		log(Warning, format, vl);
	}
}

void Logger::info(const wchar_t* format, ...)
{
	if (level_ <= Info)
	{
		va_list vl;
		va_start(vl, format);
		log(Info, format, vl);
	}
}

void Logger::error(const StringCollT &strings, const wchar_t* format, ...)
{
	va_list vl;
	va_start(vl, format);
	log(Error, format, vl);
	for (auto s : strings) append(Error, s.c_str());
}

void Logger::warning(const StringCollT &strings, const wchar_t* format, ...)
{
	if (level_ <= Warning)
	{
		va_list vl;
		va_start(vl, format);
		log(Warning, format, vl);
		for (auto s : strings) append(Warning, s.c_str());
	}
}

void Logger::info(const StringCollT &strings, const wchar_t* format, ...)
{
	if (level_ <= Info)
	{
		va_list vl;
		va_start(vl, format);
		log(Info, format, vl);
		for (auto s : strings) append(Info, s.c_str());
	}
}

bool Logger::test(const bool result, const wchar_t* format, ...)
{
	if (!result)
	{
		va_list vl;
		va_start(vl, format);
		log(Error, format, vl);
	}
	return result;
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


IMPLEMENT_DYNAMIC_CLASS(wxLoggerEvent, wxCommandEvent)

wxDEFINE_EVENT(wxEVT_LOGGER_EVENT, wxLoggerEvent);

wxLoggerEvent::wxLoggerEvent() :
	wxCommandEvent(wxEVT_LOGGER_EVENT, wxID_ANY),
	level_(Logger::Info)
{
}

wxLoggerEvent::wxLoggerEvent(const wxLoggerEvent &other) :
	wxCommandEvent(other)
{
	level_ = other.level_;
}

wxEvent *wxLoggerEvent::Clone() const
{
	return new wxLoggerEvent(*this);
};

Logger::LevelT wxLoggerEvent::getLevel() const
{
	return level_;
}

void wxLoggerEvent::setLevel(const Logger::LevelT level)
{
	level_ = level;
}




