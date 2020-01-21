/*
 * ShellExecute.h
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_SHELLEXECUTE_H_
#define COMMON_SHELLEXECUTE_H_

#include <string>

class ShellExecute
{
public:
	ShellExecute();
	virtual ~ShellExecute();

	static bool shell(const std::string &cmd, int &exitCode, std::string &stdout, const int timeoutms = -1);

private:


};

class ShellExecuteEvent
{
public:
	ShellExecuteEvent();
	ShellExecuteEvent(const ShellExecuteEvent &other);
	ShellExecuteEvent(ShellExecuteEvent &&other);
	virtual ~ShellExecuteEvent();
	ShellExecuteEvent& operator=(const ShellExecuteEvent &other);

	int  getError() const;
	int  getPid() const;
	std::string getStdout() const;

private:

	int pid_;
	int error_;
	std::string stdout_;
};

#endif /* COMMON_SHELLEXECUTE_H_ */
