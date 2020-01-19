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

	static long GetThreadID();

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

	static std::wstring strToWStr(const char* str);		// string to wide string
	static std::string wStrToStr(const wchar_t* str);	// wide string to string

private:

	SU() = default;
	SU(const SU&) = default;
	SU(const SU&&);

};

#endif /* COMMON_UTILITIES_H_ */
