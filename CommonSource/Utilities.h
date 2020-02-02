/*
 * Utilities.h
 *
 *  Created on: 18 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_

#include <string>

class Utilities
{
public:

	virtual ~Utilities();

	static long getThreadId();
	static long getProcessId();
	static long getMsCounter();

private:

	Utilities() = default;
	Utilities(const Utilities&) = default;
	Utilities(const Utilities&&);
};

// String Utilities
class SU
{
public:

	virtual ~SU();

	static std::wstring strToWStr(const char* str, int len = 0);		// string to wide string
	static std::string wStrToStr(const wchar_t* str, int len = 0);	    // wide string to string
	static std::wstring strToWStr(const std::string str);
	static std::string wStrToStr(const std::wstring str);

private:

	SU() = default;
	SU(const SU&) = default;
	SU(const SU&&);

};

#endif /* COMMON_UTILITIES_H_ */
