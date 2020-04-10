/*
 * ShellExecute.cpp
 *
 *  Windows version
 *
 */

#include "ShellExecute.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cwchar>
#include <errno.h>
#include <fcntl.h>
#include <memory>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <utility>
#include <wchar.h>

#include "Logger.h"
#include "Utilities.h"

struct ShellThreadData
{
public:

	ShellThreadData();

	ShellExecuteResult 	result_;
	int 				timeoutms_;
	wxEvtHandler* wxHandler_;
	int					wxid_;
	ShellExecuteEventHandlerT handler_;
	HANDLE				completed_;
};

bool shellWin_(ShellThreadData& data);
DWORD WINAPI shellWinThread1_(void* pdata);
DWORD WINAPI shellWinThread2_(void* pdata);

bool ShellExecute::shellFile(const std::string& verb, const std::string& cmd)
{
	Logger::info("ShellExecute::shellFile {} {}", verb, cmd);

	HINSTANCE n = ShellExecuteA(
		NULL,
		verb.c_str(),
		cmd.c_str(),
		NULL,
		NULL,
		SW_MAXIMIZE);

	if (n > reinterpret_cast<HINSTANCE>(32))
		return true;
	else
	{
		Logger::systemError("ShellExecute::shell() CreateProcessA() failed, {} {} ", verb, cmd);
		return false;
	}
}

// shell
bool ShellExecute::shell(const std::string &cmd)
{
	Logger::info("ShellExecute::shell {}", cmd);

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOA siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// CreateProcess may modify the command line
	constexpr int cmdMaxLen = 2000;
	char cmd_[cmdMaxLen * sizeof(char)];
	ZeroMemory(cmd_, cmdMaxLen * sizeof(char));

	// Create the child process. 
	bSuccess = CreateProcessA(
		nullptr,
		cmd_,				// command line 
		&saAttr,			// process security attributes 
		nullptr,			// primary thread security attributes 
		TRUE,				// handles are inherited 
		CREATE_NO_WINDOW,   // creation flags 
		nullptr,			// use parent's environment 
		nullptr,			// use parent's current directory 
		&siStartInfo, 
		&piProcInfo); 

	if (bSuccess)
		return true;
	else
	{
		Logger::systemError("ShellExecute::shell() CreateProcessA() failed, {} ", cmd);
		return false;
	}
}

// Synchronous shell execute
bool ShellExecute::shellSync(
		const std::string &cmd,
		const int timeoutms /*= -1*/)
{
	Logger::info("ShellExecute::shellSync {}, timeout = {}", cmd, timeoutms);
	ShellThreadData data;
	data.result_.cmd_ = cmd;
	data.timeoutms_ = timeoutms;
	return shellWin_(data);
}

// Synchronous shell execute that returns the result data
bool ShellExecute::shellSync(
	const std::string& cmd,
	ShellExecuteResult& result,
	const int timeoutms /*= -1 */)
{
	Logger::info("ShellExecute::shellSync {}, timeout = {}", cmd, timeoutms);
	ShellThreadData data;
	data.result_.cmd_ = cmd;
	data.timeoutms_ = timeoutms;
	bool res;
	res = shellWin_(data);
	result = std::move(data.result_);
	return res;
}

// Asynch shell execute, notification is via user supplied handler on worker thread
bool ShellExecute::shellAsync(
	const std::string& cmd,
	ShellExecuteEventHandlerT handler,
	const int userId,
	void* userData,
	const int timeoutms)
{
	Logger::info("ShellExecute::shellAsync {}, timeout = {}", cmd, timeoutms);
	ShellThreadData *data = new ShellThreadData;
	data->result_.cmd_ = cmd;
	data->result_.userId_ = userId;
	data->result_.userData_ = userData;
	data->timeoutms_ = timeoutms;
	data->handler_ = handler;

	DWORD tid;
	HANDLE hThread = CreateThread(
		NULL,
		0,
		&shellWinThread2_,
		reinterpret_cast<void*>(data),
		0,
		&tid);
	return true;
}

// Asynch shell execute, notification is via wxWidgets event handler on GUI thread
bool ShellExecute::shellAsyncGui(
	const std::string& cmd,
	wxEvtHandler* wxHandler,
	const int wxid,
	const int userId,
	void* userData,
	const int timeoutms)
{
	Logger::info("ShellExecute::shellAsyncGui {}, timeout = {}", cmd, timeoutms);
	ShellThreadData* data = new ShellThreadData;
	data->result_.cmd_ = cmd;
	data->result_.userId_ = userId;
	data->result_.userData_ = userData;
	data->timeoutms_ = timeoutms;
	data->wxHandler_ = wxHandler;
	data->wxid_ = wxid;

	DWORD tid;
	HANDLE hThread = CreateThread(
		NULL,
		0,
		&shellWinThread2_,
		reinterpret_cast<void*>(data),
		0,
		&tid);
	return true;
}

bool shellWin_(ShellThreadData& data)
{
	data.completed_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (data.completed_ == NULL)
	{
		Logger::systemError("CreateEvent");
		data.result_.success_ = false;
		return false;
	}

	// start worker thread that executes the shell
	DWORD tid;
	HANDLE hThread = CreateThread(
		NULL, 
		0, 
		&shellWinThread1_,
		reinterpret_cast<void *>(&data), 
		0, 
		&tid);

	// wait for completion
	DWORD res = WaitForSingleObject(
		data.completed_, 
		data.timeoutms_ == -1 ? INFINITE : data.timeoutms_);

	if (res == WAIT_OBJECT_0)
	{
		CloseHandle(data.completed_);
	}
	else if (res == WAIT_TIMEOUT)
	{
		Logger::error("WaitForSingleObject timedout");
		data.result_.timedOut_ = true;
		data.result_.success_ = false;
	}
	else
	{
		Logger::systemError("WaitForSingleObject");
		data.result_.success_ = false;
	}

	// notify client
	if (data.handler_ != nullptr)
		data.handler_(data.result_);

	// notify GUI
	if (data.wxHandler_ != nullptr)
	{
		wxShellExecuteEvent evt(data.result_, data.wxid_);
		data.wxHandler_->AddPendingEvent(evt);
	}

	if (!data.result_.success_)
		Logger::error("ShellExecute failed");
	return data.result_.success_;
}

DWORD shellWinThread1_(void* pdata)
{
	ShellThreadData& data = *reinterpret_cast<ShellThreadData*>(pdata);

	HANDLE hStdOutRd = NULL;
	HANDLE hStdOutWr = NULL;
	HANDLE hStdErrRd = NULL;
	HANDLE hStdErrWr = NULL;

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hStdOutRd, &hStdOutWr, &saAttr, 0))
	{
		Logger::systemError("Error CreatePipe");
		data.result_.success_ = false;
		SetEvent(data.completed_);
		return false;
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(hStdOutRd, HANDLE_FLAG_INHERIT, 0))
	{
		Logger::systemError("Error SetHandleInformation");
		data.result_.success_ = false;
		SetEvent(data.completed_);
		return false;
	}

	if (!CreatePipe(&hStdErrRd, &hStdErrWr, &saAttr, 0))
	{
		Logger::systemError("Error CreatePipe");
		data.result_.success_ = false;
		SetEvent(data.completed_);
		return false;
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(hStdErrRd, HANDLE_FLAG_INHERIT, 0))
	{
		Logger::systemError("Error SetHandleInformation");
		data.result_.success_ = false;
		SetEvent(data.completed_);
		return false;
	}

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOA siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdOutput = hStdOutWr;
	siStartInfo.hStdError = hStdErrWr;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	 
	// CreateProcess may modify the command line
	constexpr int cmdMaxLen = 2000;
	char cmd_[cmdMaxLen * sizeof(char)];
	ZeroMemory(cmd_, cmdMaxLen * sizeof(char));
	data.result_.cmd_.copy(cmd_, cmdMaxLen);

	// Create the child process. 
	bSuccess = CreateProcessA(
		NULL,
		cmd_,		   // command line 
		&saAttr,       // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW,     // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	 // If an error occurs, exit the application. 
	if (!bSuccess)
	{
		Logger::systemError("Error CreateProcess");
		CloseHandle(hStdOutRd);
		CloseHandle(hStdOutWr);
		CloseHandle(hStdErrRd);
		CloseHandle(hStdErrWr);
		data.result_.success_ = false;
		SetEvent(data.completed_);
		return false;
	}

	// child process has write handle now, close this handle
	CloseHandle(hStdOutWr);
	CloseHandle(hStdErrWr);

	DWORD dwRead;
	constexpr int BUFSIZE = 4096;
	CHAR chBuf[BUFSIZE];
	bool bStdOut = true;
	bool bStdErr = true;

	while (true)
	{
		if (ReadFile(hStdOutRd, chBuf, BUFSIZE, &dwRead, NULL))
		{
			if (dwRead > 0)
				data.result_.stdout_ += std::string(chBuf, dwRead);
		}
		else
		{
			bStdOut = false;
			if (!bStdErr) break;
		}

		if (ReadFile(hStdErrRd, chBuf, BUFSIZE, &dwRead, NULL))
		{
			if (dwRead > 0)
				data.result_.stderr_ += std::string(chBuf, dwRead);
		}
		else
		{
			bStdErr = false;
			if (!bStdOut) break;
		}
	}

	CloseHandle(hStdOutRd);
	CloseHandle(hStdErrRd);

	// get exit code
	DWORD ec = 0;
	GetExitCodeProcess(piProcInfo.hProcess, &ec);
	data.result_.exitCode_ = ec;
	data.result_.pid_ = piProcInfo.dwProcessId;
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);

	// update result
	data.result_.success_ = ec == 0;
	SetEvent(data.completed_);
	return ec == 0;
}

DWORD WINAPI shellWinThread2_(void* pdata)
{
	ShellThreadData& data = *reinterpret_cast<ShellThreadData*>(pdata);
	bool res = shellWin_(data);

	if (!data.result_.timedOut_)
		delete &data;
	else
		// data cannot be deleted as child process is still running
		Logger::warning("CreateProcess timedout, memory leak {}", data.result_.cmd_);

	return data.result_.success_ ? TRUE : FALSE;
}


ShellThreadData::ShellThreadData() :
	timeoutms_(0),
	wxHandler_(nullptr),
	wxid_     (wxID_ANY),
	handler_  (nullptr),
#ifdef WINDOWS_BUILD
	completed_(NULL)
#elif LINUX_BUILD
	fpStdout_ (0),
	fpStderr_ (0)
#endif
{
}



