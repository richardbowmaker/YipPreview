#include "Logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>

#include "Utilities.h"

// static members
Logger::LevelT Logger::level_ = Logger::Error;
wxListBox* Logger::listBox_ = nullptr;

void Logger::setLevel(LevelT level)
{
	level_ = level;
}

void Logger::initialise(wxListBox* listBox, LevelT level)
{
	level_ = level;
	listBox_ = listBox;
}

void Logger::clear()
{
	listBox_->Clear();
}

void Logger::append(const wchar_t* text)
{
	if (listBox_ == nullptr) return;
	listBox_->Append(text);
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


