/*
 * Utilities.h
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_

#include <wx/wx.h>

#include "_Types.h"

class Utilities
{
public:

	virtual ~Utilities();

	static long getThreadId();
	static long getProcessId();
	static long getMsCounter();
	static int pageDown(const int total, const int top, const int visible);
	static int pageUp(const int total, const int top, const int visible);
	static int getRand(const int min, const int max);
	static int messageBox(
			const wchar_t *format, // the message, supports printf formatting
			const wchar_t *caption,
			const int style = wxOK | wxCENTRE, // see wxMessageDialog for style options
			wxWindow * parent = NULL,
			...);

private:

	Utilities() = default;
	Utilities(const Utilities&) = default;
	Utilities(Utilities&&);

	static bool srand_;
};

// String Utilities
class SU
{
public:

	virtual ~SU();

	static std::wstring strToWStr(const char* str, int len = 0);		// string to wide string
	static std::string  wStrToStr(const wchar_t* str, int len = 0);	    // wide string to string
	static std::wstring strToWStr(const std::string str);
	static std::string  wStrToStr(const std::wstring str);
	static bool startsWith(const wchar_t *str, const wchar_t* prefix);
	static bool startsWith(const std::wstring str, const std::wstring prefix);

private:

	SU() = default;
	SU(const SU&) = default;
	SU(SU&&);
};

class FU
{
public:

	virtual ~FU();

	static bool deleteFile(const std::wstring file);
	static bool fileExists(const std::wstring file);
	static bool copyFile(const std::wstring src, const std::wstring dest, const bool overwrite = true);
	static bool moveFile(const std::wstring src, const std::wstring dest, const bool overwrite = true);
	static std::wstring getFileStem(const std::wstring path);
	static std::wstring getPathNoExt(const std::wstring path);
	static std::wstring getExt(const std::wstring path);
	static std::wstring getPathSeparator();
	static bool findFiles(
			const std::wstring directory,
			StringCollT &files,
			const bool sort = false);
	static bool findMatchingFiles(
			const std::wstring directory,
			StringCollT &files,
			const std::wstring filter,
			const bool sort = false);
	static bool findFilesDirs(
			const std::wstring directory,
			StringCollT &files,
			StringCollT &dirs,
			const bool sort = false);
	static bool findMatchingFilesDirs(
			const std::wstring directory,
			StringCollT &files,
			const std::wstring filter,
			StringCollT &dirs,
			const bool sort = false);
	static bool findMatchingFilesRex(
			const std::wstring directory,
			StringCollT &files,
			const std::wstring regex,
			const bool sort = false);
	static bool findMatchingFilesDirsRex(
			const std::wstring directory,
			StringCollT &files,
			const std::wstring regex,
			StringCollT &dirs,
			const bool sort = false);
	static std::wstring pathToLocal(const wchar_t *path);
	static std::wstring abbreviateFilename(const std::wstring &file, const int max);
	static bool mkDir(const std::wstring dir);

private:

	FU() = default;
	FU(const FU&) = default;
	FU(FU&&);

	static bool findFiles(
			const std::wstring directory,
			StringCollT *files,
			const std::wstring *filter,
			const std::wstring *regex,
			StringCollT *dirs,
			const bool sort);
};

class Duration
{
public:

	Duration();

	static bool test();

	void setMs(const long ms);
	long getMs() const;
	bool parse(const std::wstring &str);
	bool parse(const std::string &str);
	std::wstring toString() const;

	int hh_;
	int mm_;
	int ss_;
	int ms_;
};



#endif /* COMMON_UTILITIES_H_ */
