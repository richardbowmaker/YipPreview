/*
 * ShellExecute.cpp
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#include "ShellExecute.h"

#ifdef WINDOWS_BUILD
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
#elif LINUX_BUILD
	#include <algorithm>
	#include <cctype>
	#include <cstdio>
	#include <cwchar>
	#include <errno.h>
	#include <fcntl.h>
	#include <memory>
	#include <pthread.h>
	#include <signal.h>
	#include <stdarg.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string>
	#include <sstream>
	#include <sys/wait.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <utility>
	#include <wchar.h>
#endif

#include "Logger.h"
#include "Utilities.h"

ShellExecute::ShellExecute()
{
}

ShellExecute::~ShellExecute()
{
}

#ifdef WINDOWS_BUILD
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

	if (reinterpret_cast<int>(n) > 32)
		return true;
	else
	{
		Logger::systemError("ShellExecute::shell() CreateProcessA() failed, {} {} ", verb, cmd);
		return false;
	}
}
#endif

// shell
bool ShellExecute::shell(const std::string &cmd)
{
	Logger::info("ShellExecute::shell {}", cmd);
#ifdef WINDOWS_BUILD
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
#elif LINUX_BUILD
	ShellThreadData data;
	data.result_.cmd_ = cmd;
	if (shellStart(data))
	{
		fclose(data.fpStdout_);
		fclose(data.fpStderr_);
		return true;
	}
	else
		return false;
#endif
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
#ifdef WINDOWS_BUILD
	return shellWin_(data);
#elif LINUX_BUILD
	if (!shellStart(data)) return false;
	return shellWait(data);
#endif
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
#ifdef WINDOWS_BUILD
	res = shellWin_(data);
#elif LINUX_BUILD
	if (!shellStart(data)) return false;
	res = shellWait(data);
#endif
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

#ifdef WINDOWS_BUILD
	DWORD tid;
	HANDLE hThread = CreateThread(
		NULL,
		0,
		&ShellExecute::shellWinThread2_,
		reinterpret_cast<void*>(data),
		0,
		&tid);
	return true;
#elif LINUX_BUILD	
	if (!shellStart(*data)) return false;
	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &ShellExecute::shellThreadWait, (void*)data) != 0)
	{
		Logger::systemError("Error creating thread");
		delete data;
		return false;
	}

	// allow client to continue
	return true;
#endif
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

#ifdef WINDOWS_BUILD
	DWORD tid;
	HANDLE hThread = CreateThread(
		NULL,
		0,
		&ShellExecute::shellWinThread2_,
		reinterpret_cast<void*>(data),
		0,
		&tid);
	return true;
#elif LINUX_BUILD	
	if (!shellStart(*data)) return false;
	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &ShellExecute::shellThreadWaitGui, (void*)data) != 0)
	{
		Logger::systemError("Error creating thread");
		delete data;
		return false;
	}
	// allow client to continue
	return true;
#endif
}

#ifdef WINDOWS_BUILD
bool ShellExecute::shellWin_(ShellThreadData& data)
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
		&ShellExecute::shellWinThread1_,
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

DWORD WINAPI ShellExecute::shellWinThread1_(void* pdata)
{
	ShellExecute::ShellThreadData& data = *reinterpret_cast<ShellExecute::ShellThreadData*>(pdata);

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

DWORD WINAPI ShellExecute::shellWinThread2_(void* pdata)
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

#elif LINUX_BUILD

//-----------------------------------------------------------

void *ShellExecute::shellThreadWait(void *ptr)
{
	if (ptr == nullptr) return nullptr;

	// wait for shell to complete
	ShellThreadData *data = (ShellThreadData*)ptr;
	shellWait(*data);

	// notify client
	if (data->handler_ != nullptr)
		data->handler_(data->result_);
	delete data;
	return 0;
}

void *ShellExecute::shellThreadWaitGui(void *ptr)
{
	if (ptr == nullptr) return nullptr;

	// wait for shell to complete
	ShellThreadData *data = (ShellThreadData*)ptr;
	shellWait(*data);

	// notify client
	if (data->wxHandler_ != nullptr)
	{
		wxShellExecuteEvent evt(data->result_, data->wxid_);
		data->wxHandler_->AddPendingEvent(evt);
	}
	delete data;
	return 0;
}

bool ShellExecute::shellStart(ShellThreadData &data)
{
	pid_t pid;
	constexpr int kFdRead = 0;
	constexpr int kFdWrite = 1;
	int fdStdout[2] = {0};
	int fdStderr[2] = {0};

	if (pipe2(fdStdout, O_CLOEXEC) == -1)
	{
	    Logger::systemError("Error creating stdout pipe");
	    return false;
	}

	if (pipe2(fdStderr, O_CLOEXEC) == -1)
	{
	    Logger::systemError("Error creating stderr pipe");
	    return false;
	}

	if((pid = fork()) == -1)
	{
	    Logger::systemError("Error forking process");
	    data.result_.success_ = false;
	    return false;
	}

	// child process
	if (pid == 0)
	{
	    close(fdStdout[kFdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fdStdout[kFdWrite], 1); // Redirect stdout to pipe

	    close(fdStderr[kFdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fdStderr[kFdWrite], 2); // Redirect stderr to pipe

	    setpgid(pid, pid); //Needed so negative PIDs can kill children of /bin/sh
	    execl("/bin/sh", "/bin/sh", "-c",
			  data.result_.cmd_.c_str(),
			  NULL);
	    _exit(0);
	}

	close(fdStdout[kFdWrite]); // Close the WRITE end of the pipe since parent's fd is read-only
	close(fdStderr[kFdWrite]);
	data.result_.pid_ = pid;

	data.fpStdout_ = fdopen(fdStdout[kFdRead], "r");
	data.fpStderr_ = fdopen(fdStderr[kFdRead], "r");

	// set read to non-blocking so we can timeout
	int flags = fcntl(fdStdout[kFdRead], F_GETFL);
	fcntl(fdStdout[kFdRead], F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(fdStderr[kFdRead], F_GETFL);
	fcntl(fdStderr[kFdRead], F_SETFL, flags | O_NONBLOCK);

	return true;
}

bool ShellExecute::shellWait(ShellThreadData &data)
{
	char buff[100000];

	int rerr;
	long stime = US::getMsCounter();

	bool bStdout = true;
	bool bStderr = true;

	while (true)
	{
		US::delay(250);

		// std out handling
		rerr = read(fileno(data.fpStdout_), buff, sizeof(buff) - 1);
		if (rerr == 0)
		{
			bStdout = false;
			if (!bStderr) break;
		}
		else if (rerr < 0)
		{
			if (errno != EWOULDBLOCK)
			{
				fclose(data.fpStdout_);
				fclose(data.fpStderr_);
				Logger::systemError("File stdout read error");
				data.result_.success_ = false;
				return false;
			}
		}
		else if (rerr > 0)
		{
			data.result_.stdout_ += std::string(buff, rerr);
		}

		// std err handling
		rerr = read(fileno(data.fpStderr_), buff, sizeof(buff) - 1);
		if (rerr == 0)
		{
			bStderr = false;
			if (!bStdout) break;
		}
		else if (rerr <= 0)
		{
			if (errno != EWOULDBLOCK)
			{
				fclose(data.fpStdout_);
				fclose(data.fpStderr_);
				Logger::systemError("File stderr read error");
				data.result_.success_ = false;
				return false;
			}
		}
		else if (rerr > 0)
		{
			data.result_.stderr_ += std::string(buff, rerr);
		}

		// check for timedout
		if (data.timeoutms_ != -1)
		{
			long timer = US::getMsCounter();
			if (timer - stime > data.timeoutms_)
			{
				fclose(data.fpStdout_);
				fclose(data.fpStderr_);
				Logger::error("Shell execute timed out");
				data.result_.timedOut_ = true;
				data.result_.success_ = false;
				return false;
			}
		}

		// child process terminated, quit reading
		if (kill(data.result_.pid_, 0) != 0) break;
	}

	// close reading end of pipe
	int exit;
	fclose(data.fpStdout_);
	fclose(data.fpStderr_);
	while (waitpid(data.result_.pid_, &exit, 0) == -1)
	{
		if (errno != EINTR)
		{
			Logger::systemError("Error closing pipe");
			data.result_.success_ = false;
			return false;
		}
	}

	// if child process ended, clear pid
	if (kill(data.result_.pid_, 0) != 0)
		data.result_.pid_ = 0;

	// setup return result
	data.result_.exitCode_ 	= exit;
	data.result_.success_ 	= exit == 0;

	if (!data.result_.success_)
		Logger::error("ShellExecute failed");

	return data.result_.success_;
}

#endif

ShellExecute::ShellThreadData::ShellThreadData() :
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

//-----------------------------------------------------------
// ShellExecuteEvent

IMPLEMENT_DYNAMIC_CLASS(wxShellExecuteEvent, wxCommandEvent)

wxDEFINE_EVENT(wxEVT_SHELL_EXECUTE_RESULT, wxShellExecuteEvent);

wxShellExecuteEvent::wxShellExecuteEvent() :
	wxCommandEvent(wxEVT_SHELL_EXECUTE_RESULT, wxID_ANY)
{
}

wxShellExecuteEvent::wxShellExecuteEvent(ShellExecuteResult &result, int wxid) :
	wxCommandEvent(wxEVT_SHELL_EXECUTE_RESULT, wxid),
	result_(result)
{
}

wxShellExecuteEvent::wxShellExecuteEvent(const wxShellExecuteEvent &other) :
	wxCommandEvent(other)
{
	result_ = other.result_;
}

wxEvent *wxShellExecuteEvent::Clone() const
{
	return new wxShellExecuteEvent(*this);
};

ShellExecuteResult wxShellExecuteEvent::getResult() const
{
	return result_;
}

//-----------------------------------------------------------
// ShellExecuteResult

ShellExecuteResult::ShellExecuteResult() :
	pid_	 (0),
	exitCode_(0),
	success_ (false),
	timedOut_(0),
	userId_	 (0),
	userData_(nullptr)
{
}

ShellExecuteResult::~ShellExecuteResult()
{
}

ShellExecuteResult::ShellExecuteResult(const ShellExecuteResult &other)
{
	*this = other;
}

ShellExecuteResult::ShellExecuteResult(ShellExecuteResult &&other) :
	cmd_	 (std::move(other.cmd_)),
	pid_	 (std::exchange(other.pid_, 0)),
	exitCode_(std::exchange(other.exitCode_, 0)),
	success_ (std::exchange(other.success_, false)),
	stderr_	 (std::move(other.stderr_)),
	stdout_	 (std::move(other.stdout_)),
	timedOut_(std::exchange(other.timedOut_, 0)),
	userId_	 (std::exchange(other.userId_, 0)),
	userData_(std::exchange(other.userData_, nullptr))
{
}

void ShellExecuteResult::clear()
{
	cmd_.clear();
	stdout_.clear();
	stderr_.clear();
	pid_ 		= 0;
	exitCode_ 	= 0;
	success_ 	= false;
	timedOut_ 	= false;
	userId_ 	= 0;
	userData_ 	= nullptr;
}

ShellExecuteResult& ShellExecuteResult::operator=(const ShellExecuteResult &other)
{
	if (&other != this)
	{
		cmd_		= other.cmd_;
		pid_ 		= other.pid_;
		exitCode_	= other.exitCode_;
		success_	= other.success_;
		stdout_ 	= other.stdout_;
		stderr_ 	= other.stderr_;
		timedOut_	= other.timedOut_;
		userId_		= other.userId_;
		userData_	= other.userData_;
	}
	return *this;
}

ShellExecuteResult& ShellExecuteResult::operator=(ShellExecuteResult &&other)
{
	if (&other != this)
	{
		cmd_		= other.cmd_;
		pid_ 		= other.pid_;
		exitCode_	= other.exitCode_;
		success_	= other.success_;
		stdout_ 	= other.stdout_;
		stderr_ 	= other.stderr_;
		timedOut_	= other.timedOut_;
		userId_		= other.userId_;
		userData_	= other.userData_;
		other.clear();
	}
	return *this;
}

std::string ShellExecuteResult::getCmd() const
{
	return cmd_;
}

int ShellExecuteResult::getPid() const
{
	return pid_;
}

int ShellExecuteResult::getExitCode() const
{
	return exitCode_;
}

bool ShellExecuteResult::getSuccess() const
{
	return success_;
}

std::string ShellExecuteResult::getStdout() const
{
	return stdout_;
}

std::string ShellExecuteResult::getStderr() const
{
	return stderr_;
}

bool ShellExecuteResult::getTimedOut() const
{
	return timedOut_;
}

int ShellExecuteResult::getUserId() const
{
	return userId_;
}

void *ShellExecuteResult::getUserData() const
{
	return userData_;
}

std::string ShellExecuteResult::toString() const
{
	char buf[4000];
	snprintf(buf, sizeof(buf) / sizeof(char),
			"Shell result:\ncommand: %s\n%s, id %d, pid: %d, exit code: %d, %s\nstdout:\n%s\nstderr:\n%s",
			cmd_.c_str(),
			(success_ ? "Success" : "Fail"),
			userId_,
			pid_,
			exitCode_,
			(timedOut_ ? ", TimedOut" : ""),
			stdout_.c_str(),
			stderr_.c_str());
	return std::string(buf);
}

void ShellExecuteResult::killChildProcess()
{
#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
	// kills the shell and the child process
	if (pid_ == 0) return;

	// get child pid of the shell, from the shell pid
	char buf[200];
	snprintf(buf, sizeof(buf) / sizeof(char), "/bin/ps --ppid %d", pid_);
	ShellExecuteResult result;

	if (ShellExecute::shellSync(buf, result, 1000))
	{
		std::string s = result.stdout_;

		// extract process id, starting first digit
		std::string::iterator n =
				std::find_if(s.begin(), s.end(), [](char c){ return std::isdigit(c); });

		if (n != s.end())
		{
			int pid;
			sscanf(s.c_str() + (n - s.begin()), "%d", &pid);
			kill(pid, SIGTERM);
		}
		kill(pid_, SIGTERM);
	}
#endif
}


