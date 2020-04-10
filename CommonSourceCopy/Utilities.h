/*
 * Utilities.h
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_

#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
	#include <sys/types.h>
	#include <unistd.h>
#endif

#include <mutex>
#include <wx/wx.h>

#include "_Types.h"
#include <fmt/core.h>
#include "Logger.h"

class US
{
public:

	virtual ~US();

	static long getThreadId();
	static long getProcessId();
	static long getMsCounter();
	static int pageDown(const int total, const int top, const int visible);
	static int pageUp(const int total, const int top, const int visible);
	static int getRand(const int min, const int max);
	static void delay(int ms);
	static std::string getWorkingDirectory();

	template<typename... Args>
	static int messageBox(
			const char* format, // the message, supports printf formatting
			const char* caption,
			const int style = wxOK | wxCENTRE, // see wxMessageDialog for style options
			wxWindow* parent = NULL,
			Args... args)
	{
		return messageBox_(fmt::format(format, args...).c_str(), caption, style, parent);
	}

	static std::string bytesToString(long long bytes);

private:

	US() = default;
	US(const US&) = default;
	US(US&&);

	static int messageBox_(const char* message, const char* caption, const int style, wxWindow* parent);

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
	static bool startsWith(const char *str, const char* prefix);
	static bool startsWith(const std::string str, const std::string prefix);
	static bool endsWith(const char* str, const char* suffix);
	static bool endsWith(const std::string str, const std::string suffix);
	static std::string doubleQuotes(const std::string &s);
	static std::string singleQuotes(const std::string &s);

private:

	SU() = default;
	SU(const SU&) = default;
	SU(SU&&);
};

class FU
{
public:

	virtual ~FU();

	static bool deleteFile(const std::string file);
	static bool fileExists(const std::string file);
	static bool copyFile(const std::string src, const std::string dest, const bool overwrite = true);
	static bool moveFile(const std::string src, const std::string dest, const bool overwrite = true);
	static long long getFileSize(const std::string file);

	struct DiskSpaceT
	{
		long long free;
		long long total;
	};
	static bool getVolumeFreeSpace(const std::string& volume, DiskSpaceT &space);

	static std::string getFileStem(const std::string path);
	static std::string getFileName(const std::string path);
	static std::string getPathNoExt(const std::string path);
	static std::string getPath(const std::string path);
	static std::string getExt(const std::string path);
	static std::string getPathSeparator();
	static bool findFiles(
			const std::string directory,
			StringCollT &files,
			const bool subdirs = false,
			const bool sort = false);
	static bool findMatchingFiles(
			const std::string directory,
			StringCollT &files,
			const std::string filter,
			const bool subdirs = false,
			const bool sort = false);
	static bool findFilesDirs(
			const std::string directory,
			StringCollT &files,
			StringCollT &dirs,
			const bool subdirs = false,
			const bool sort = false);
	static bool findMatchingFilesDirs(
			const std::string directory,
			StringCollT &files,
			const std::string filter,
			StringCollT &dirs,
			const bool subdirs = false,
			const bool sort = false);
	static bool findMatchingFilesRex(
			const std::string directory,
			StringCollT &files,
			const std::string regex,
			const bool subdirs = false,
			const bool sort = false);
	static bool findMatchingFilesDirsRex(
			const std::string directory,
			StringCollT &files,
			const std::string regex,
			StringCollT &dirs,
			const bool subdirs = false,
			const bool sort = false);
	static std::string pathToOs(const char *path);
	static std::string pathToOs(const std::string &path);
	static std::string abbreviateFilename(const std::string &file, const int max);
	static bool mkDir(const std::string dir);
	static bool rmDir(const std::string dir);

private:

	FU() = default;
	FU(const FU&) = default;
	FU(FU&&);

	static bool findFiles(
			const std::string directory,
			StringCollT *files,
			const std::string *filter,
			const std::string *regex,
			StringCollT *dirs,
			const bool subdirs,
			const bool sort);
};

class Duration
{
public:

	Duration();

	static bool test();

	void setMs(const long ms);
	long getMs() const;
	bool parse(const std::string &str);
	std::string toString() const;

	int hh_;
	int mm_;
	int ss_;
	int ms_;
};



#endif /* COMMON_UTILITIES_H_ */
