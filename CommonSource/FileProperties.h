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

	void setString(const std::string &property, const std::string &value);
	void setInt(const std::string &property, const int value);
	void setFloat(const std::string &property, const float value);
	void incCount(const std::string &property);
	void setDateTimeNow(const std::string &property);
	std::string getString(const std::string &property) const;
	int getInt(const std::string &property) const;
	float getFloat(const std::string &property) const;
	int getCount(const std::string &property) const;
	std::string toString() const;
	void fromString(const std::string &s);
	void clear();
	int getSize() const;
	void remove(const std::string property);
	void toLogger() const;

private:

	PropertiesT properties_;
};

#endif /* COMMON_FILEPROPERTIES_H_ */
