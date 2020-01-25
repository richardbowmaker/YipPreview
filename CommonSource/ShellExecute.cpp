/*
 * ShellExecute.cpp
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#include "ShellExecute.h"

#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <memory>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "Logger.h"
#include "Utilities.h"

ShellExecute::ShellExecute()
{
}

ShellExecute::~ShellExecute()
{
}

// Synchronous shell execute
bool ShellExecute::shellSync(
		const std::wstring &cmd,
		const int timeoutms /*= -1*/)
{
	ShellThreadData data;
	data.result_.cmd_ = cmd;
	data.timeoutms_   = timeoutms;

	if (!shellStart(data)) return false;
	return shellWait(data);
}

// Synchronous shell execute that returns the result data
bool ShellExecute::shellSync(
		const std::wstring &cmd,
		ShellExecuteResult &result,
		const int timeoutms /*= -1 */)
{
	ShellThreadData data;
	data.result_.cmd_ = cmd;
	data.timeoutms_   = timeoutms;

	if (!shellStart(data)) return false;
	bool res = shellWait(data);
	result = data.result_;
	return res;
}

// Asynch shell execute, notification is via user supplied handler on worker thread
bool ShellExecute::shellAsync(
		const std::wstring &cmd,
		ShellExecuteEventHandlerPtr handler,
		const int userId,
		void* userData,
		const int timeoutms)
{
	ShellThreadData* data = new ShellThreadData;
	data->result_.cmd_ 	    = cmd;
	data->result_.userId_   = userId;
	data->result_.userData_ = userData;
	data->timeoutms_        = timeoutms;
	data->handler_ 			= handler;

	if (!shellStart(*data)) return false;

	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &ShellExecute::shellThreadWait, (void*)data) != 0)
	{
		Logger::systemError(errno, L"Error creating thread");
		delete data;
		return false;
	}

	// allow client to continue
	return true;
}

// Asynch shell execute, notification is via wxWidgets event handler on GUI thread
bool ShellExecute::shellAsyncGui(
		const std::wstring &cmd,
		wxEvtHandler *wxHandler,
		const int wxid,
		const int userId,
		void *userData,
		const int timeoutms)
{
	ShellThreadData* data = new ShellThreadData;
	data->result_.cmd_ 	    = cmd;
	data->result_.userId_   = userId;
	data->result_.userData_ = userData;
	data->timeoutms_        = timeoutms;
	data->wxHandler_ 		= wxHandler;
	data->wxid_ 		    = wxid;

	if (!shellStart(*data)) return false;

	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &ShellExecute::shellThreadWaitGui, (void*)data) != 0)
	{
		Logger::systemError(errno, L"Error creating thread");
		delete data;
		return false;
	}

	// allow client to continue
	return true;
}

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
		wxShellExecuteResult evt(data->result_, data->wxid_);
		data->wxHandler_->AddPendingEvent(evt);
	}
	delete data;
}

bool ShellExecute::shellStart(ShellThreadData &data)
{
	pid_t child_pid;
	constexpr int kFdRead = 0;
	constexpr int kFdWrite = 1;
	int fdStdout[2] = {0};
	int fdStderr[2] = {0};

	if (pipe(fdStdout) == -1)
	{
	    Logger::systemError(errno, L"Error creating stdout pipe");
	    data.result_.error_ = errno;
	    return false;
	}

	if (pipe(fdStderr) == -1)
	{
	    Logger::systemError(errno, L"Error creating stderr pipe");
	    data.result_.error_ = errno;
	    return false;
	}

	if((child_pid = fork()) == -1)
	{
	    Logger::systemError(errno, L"Error forking process");
	    data.result_.error_ = errno;
	    data.result_.success_ = false;
	    return false;
	}

	// child process
	if (child_pid == 0)
	{
	    close(fdStdout[kFdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fdStdout[kFdWrite], 1); // Redirect stdout to pipe

	    close(fdStderr[kFdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fdStderr[kFdWrite], 2); // Redirect stderr to pipe

	    setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
	    execl(
	    		"/bin/sh", "/bin/sh", "-c",
				SU::wStrToStr(data.result_.cmd_).c_str(),
				NULL);
	    _exit(0);
	}

	close(fdStdout[kFdWrite]); // Close the WRITE end of the pipe since parent's fd is read-only
	close(fdStderr[kFdWrite]);
	data.result_.pid_ = child_pid;

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
	long stime = Utilities::getMsCounter();

	bool bStdout = true;
	bool bStderr = true;

	while (true)
	{
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
				Logger::systemError(errno, L"File stdout read error");
				data.result_.error_   = errno;
				data.result_.success_ = false;
				return false;
			}
		}
		else if (rerr > 0)
		{
			data.result_.stdout_ += SU::strToWStr(std::string(buff, rerr));
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
				Logger::systemError(errno, L"File stderr read error");
				data.result_.error_   = errno;
				data.result_.success_ = false;
				return false;
			}
		}
		else if (rerr > 0)
		{
			data.result_.stderr_ += SU::strToWStr(std::string(buff, rerr));
		}

		// check for timedout
		if (data.timeoutms_ != -1)
		{
			long timer = Utilities::getMsCounter();
			if (timer - stime > data.timeoutms_)
			{
				fclose(data.fpStdout_);
				fclose(data.fpStderr_);
				Logger::error(L"Shell execute timed out");
				data.result_.timedOut_ = true;
				data.result_.success_ = false;
				return false;
			}
		}
	}

	// close read end of pipe
	int exit;
	fclose(data.fpStdout_);
	fclose(data.fpStderr_);
	while (waitpid(data.result_.pid_, &exit, 0) == -1)
	{
		if (errno != EINTR)
		{
			Logger::systemError(errno, L"Error closing pipe");
			data.result_.error_ = errno;
			data.result_.success_ = false;
			return false;
		}
	}

	// setup return result
	data.result_.exitCode_ 	= exit;
	data.result_.pid_ 		= 0;
	data.result_.success_ 	= exit == 0;
	return data.result_.success_;
}

ShellExecute::ShellThreadData::ShellThreadData() :
	fpStdout_ (0),
	fpStderr_ (0),
	timeoutms_(0),
	wxHandler_(nullptr),
	wxid_     (wxID_ANY),
	handler_  (nullptr)
{
}

//-----------------------------------------------------------
// ShellExecuteEvent

IMPLEMENT_DYNAMIC_CLASS(wxShellExecuteResult, wxCommandEvent)

wxDEFINE_EVENT(wxEVT_SHELL_EXECUTE_RESULT, wxShellExecuteResult);

wxShellExecuteResult::wxShellExecuteResult() :
	wxCommandEvent(wxEVT_SHELL_EXECUTE_RESULT, wxID_ANY)
{
}

wxShellExecuteResult::wxShellExecuteResult(ShellExecuteResult &result, int wxid) :
	wxCommandEvent(wxEVT_SHELL_EXECUTE_RESULT, wxid),
	result_(result)
{
}

wxShellExecuteResult::wxShellExecuteResult(const wxShellExecuteResult &other) :
	wxCommandEvent(other)
{
	result_ = other.result_;
}

wxEvent *wxShellExecuteResult::Clone() const
{
	return new wxShellExecuteResult(*this);
};

ShellExecuteResult wxShellExecuteResult::getResult() const
{
	return result_;
}

//-----------------------------------------------------------
// ShellExecuteResult

ShellExecuteResult::ShellExecuteResult() :
	pid_	 (0),
	exitCode_(0),
	success_ (false),
	error_	 (0),
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
	stderr_	 (std::move(other.stderr_)),
	stdout_	 (std::move(other.stdout_)),
	pid_	 (std::exchange(other.pid_, 0)),
	exitCode_(std::exchange(other.exitCode_, 0)),
	success_ (std::exchange(other.success_, false)),
	error_	 (std::exchange(other.error_, 0)),
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
	error_ 		= 0;
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
		error_ 		= other.error_;
		stdout_ 	= other.stdout_;
		stderr_ 	= other.stderr_;
		timedOut_	= other.timedOut_;
		userId_		= other.userId_;
		userData_	= other.userData_;
	}
	return *this;
}

std::wstring ShellExecuteResult::getCmd() const
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

int ShellExecuteResult::getError() const
{
	return error_;
}

std::wstring ShellExecuteResult::getStdout() const
{
	return stdout_;
}

std::wstring ShellExecuteResult::getStderr() const
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

std::wstring ShellExecuteResult::toString() const
{
	wchar_t buf[4000];
	swprintf(buf, sizeof(buf) / sizeof(wchar_t),
			L"Shell result:\ncommand: %ls\n%ls, id %d, pid: %d, exit code: %d, error: %d%ls\nstdout:\n%ls\nstderr:\n%ls",
			cmd_.c_str(),
			(success_ ? L"Success" : L"Fail"),
			userId_,
			pid_,
			exitCode_,
			error_,
			(timedOut_ ? L", TimedOut" : L""),
			stdout_.c_str(),
			stderr_.c_str());
	return std::wstring(buf);
}

void ShellExecuteResult::killChildProcess()
{
	if (pid_ == 0) return;

	// to do
}


