/*
 * ShellExecute.h
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_SHELLEXECUTE_H_
#define COMMON_SHELLEXECUTE_H_

#include <string>

class ShellExecuteResult;

class ShellExecute
{
public:
	ShellExecute();
	virtual ~ShellExecute();

//	typedef void (*ShellExecuteEventHandler)(ShellExecuteResult &result);
	using ShellExecuteEventHandlerPtr = void (*)(ShellExecuteResult &result);

	static bool shellSync(const std::wstring &cmd, const int timeoutms = -1);
	static bool shellSync(const std::wstring &cmd, ShellExecuteResult &result, const int timeoutms = -1);
//	static bool shellAsync(const std::wstring &cmd, ShellExecuteEventHandler * handler, const int userId, const int timeoutms = -1);

	static bool shellAsync(
		const std::wstring &cmd,
		const int timeoutms,
		void* data,
		ShellExecuteEventHandlerPtr handler);

private:

	static bool shell_(
		const std::wstring &cmd,
		const int timeoutms,
		void* data,
		ShellExecuteResult &result);

	static void* ShellAsyncWait(void *ptr);

};

class ShellExecuteResult
{
public:
	ShellExecuteResult();
	ShellExecuteResult(const ShellExecuteResult &other);
	ShellExecuteResult(ShellExecuteResult &&other);
	virtual ~ShellExecuteResult();
	ShellExecuteResult& operator=(const ShellExecuteResult &other);

	std::wstring getCmd() const;
	int  getPid() const;
	int  getExitCode() const;
	bool getSuccess() const;
	int  getError() const;
	std::wstring getStdout() const;
	bool getTimedOut() const;
	void* getUserData() const;
	std::wstring toString() const;
	void killChildProcess();
	void clear();

	friend class ShellExecute;

private:

	std::wstring cmd_;
	int pid_;		// process id of child shell, 0 if destroyed
	int exitCode_;
	bool success_;	// executed successfully
	int error_;
	std::wstring stdout_;
	bool timedOut_;
	void* userData_;	// returned to client, asynch shell only
};

#endif /* COMMON_SHELLEXECUTE_H_ */
