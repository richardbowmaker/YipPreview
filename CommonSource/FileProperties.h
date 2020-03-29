/*
 * FileProperties.h
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_FILEPROPERTIES_H_
#define COMMON_FILEPROPERTIES_H_

#include <map>
#include <memory>
#include <string>
#include <time.h>

#include "_Types.h"

class FileProperties
{
public:

	FileProperties() = default;
	~FileProperties() = default;

	static bool test();

	void setString(const std::wstring &property, const std::wstring &value);
	void setInt(const std::wstring &property, const int value);
	void setFloat(const std::wstring &property, const float value);
	void incCount(const std::wstring &property);
	void setDateTimeNow(const std::wstring &property);
	std::wstring getString(const std::wstring &property) const;
	int getInt(const std::wstring &property) const;
	float getFloat(const std::wstring &property) const;
	int getCount(const std::wstring &property) const;
	std::wstring toString() const;
	void fromString(const std::wstring &s);
	void clear();
	int getSize() const;
	void remove(const std::wstring property);
	void toLogger() const;

private:

	PropertiesT properties_;
};

#endif /* COMMON_FILEPROPERTIES_H_ */
