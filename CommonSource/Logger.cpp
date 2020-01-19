#include "Logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>

#include "Events.h"
#include "Utilities.h"

// static members
Logger::LevelT Logger::level_ = Logger::Error;
Logger* Logger::this_ = nullptr;
long Logger::tid_ = 0;

Logger::Logger() : wxListBox()
{
}

Logger::Logger(wxWindow* parent, wxWindowID id, LevelT level) :
	wxListBox (parent, id)
{
	tid_ = Utilities::GetThreadID();
	level_ = level;
	this_ = this;
}

wxIMPLEMENT_DYNAMIC_CLASS(Logger, wxListBox);

wxBEGIN_EVENT_TABLE(Logger, wxListBox)
	EVT_LOGGER_EVENT_COMMAND(wxID_ANY, Logger::OnLogger)
wxEND_EVENT_TABLE()

void Logger::OnLogger(wxLoggerEvent& event)
{
	// add to message box
	wxListBox* lb = dynamic_cast<wxListBox*>(event.GetEventObject());
	if (lb != nullptr) lb->Append(event.GetString().wc_str());
}

void Logger::setLevel(LevelT level)
{
	level_ = level;
}

void Logger::clear()
{
	this_->Clear();
}

void Logger::append(const wchar_t* text)
{
	if (this_ == nullptr) return;
	if (tid_ == Utilities::GetThreadID())
	{
		// in GUI thread, so update list box
		this_->Append(text);
	}
	else
	{
		// post event to GUI thread
		wxLoggerEvent evt;
		evt.SetString(wxString(text));
		evt.setLevel(22);
		evt.SetEventObject(this_);
		this_->GetEventHandler()->AddPendingEvent(evt);
	}
}

void Logger::log(const LevelT level, const wchar_t* format, va_list vl)
{
	wchar_t buff[1024];
	vswprintf(buff, sizeof(buff), format, vl);
	append(buff);
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
	wchar_t buff[1024];
#ifdef WINDOWS_BUILD
	char* errStr;
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
		(LPTSTR)& errStr,
		0,
		NULL)) return;
	swprintf(buff, sizeof(buff), L" [%ls: %d]", SU::strToWStr(errStr).c_str(), err);
	LocalFree(errStr);
#elif LINUX_BUILD
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


DEFINE_EVENT_TYPE(wxLOGGER_EVENT);
IMPLEMENT_DYNAMIC_CLASS(wxLoggerEvent, wxCommandEvent)


wxDEFINE_EVENT(wxEVT_LOGGER_EVENT, wxLoggerEvent);


wxLoggerEvent::wxLoggerEvent() :
	wxCommandEvent(wxEVT_LOGGER_EVENT, wxID_ANY),
	level_(0)
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

int wxLoggerEvent::getLevel() const
{
	return level_;
}

void wxLoggerEvent::setLevel(int level)
{
	level_ = level;
}




