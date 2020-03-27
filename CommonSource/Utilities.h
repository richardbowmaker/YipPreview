/*
 * Utilities.h
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_

#include <mutex>
#include <sys/types.h>
#include <unistd.h>
#include <wx/wx.h>

#include "_Types.h"
#include "Logger.h"

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
	static void delay(int ms);

private:

	Utilities() = default;
	Utilities(const Utilities&) = default;
	Utilities(Utilities&&);

	static bool srand_;
};

// class for boosting program up to sudo mode
// only valid for linux programs started in sudo mode
// in windows or linux programs running at user level this class can be used but does nothing

/* SudoMode class supports an application that needs sudo mode for some parts of its code.
 * The application is launched in sudo mode and should immediately drops down to user level,
 * (by calling SudoMode::initialise(uid) at the start)
 * thereafter the application can acquire sudo mode on an as needed basis.
 * E.g. sample code;
 *
 * ... user level code ...
 * {
 * 		SudoMode sudo;	// raises to sudo mode
 * 		... sudo level code ...
 * }					// destructor lowers back to user level
 * ... user level code ...
 *
 * As the application is launched in sudo mode the user level id must be passed in
 * as a parameter to the program so that it can lower the privileges. This can be
 * done via a shell as follows;
 *
 * .
 * export LD_LIBRARY_PATH=<libraries>
 * export PATH=$PATH:<extra paths>
 *
 * # capture user id
 * MYUID=$(id -u)
 *
 * # run program in sudo mode, pass in user id
 * sudo PATH="$PATH" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" <program> -uid $MYUID
 * .
 *
 * The class is thread safe in that many threads can raise to sudo level,
 * a reference count will ensure that the level is lowered when the final
 * thread lowers to user level.
 * If one thread raises to sudo level then all threads are now in sudo level.
 */

class SudoMode
{
public:

	SudoMode();
	~SudoMode();
	static void initialise(const int uid);
	static bool inSudoMode();
	void raise();
	void lower();

private:

	static std::mutex lock_; // protects updates to reference count refs_
	static bool active_; // only true when the program is run under linux in sudo mode
	static int refs_; // nested ref count
	static int uid_;
	bool got_;	// true when this instance has a ref count
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
	static bool rmDir(const std::wstring dir);

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
