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
	ShellExecuteResult result;	// throw away
	result.cmd_ = cmd;

	FILE* fp;
	if (!shellStart(result, fp)) return false;
	return shellWait(result, fp, timeoutms);
}

// Synchronous shell execute that returns the result data
bool ShellExecute::shellSync(
		const std::wstring &cmd,
		ShellExecuteResult &result,
		const int timeoutms /*= -1 */)
{
	result.clear();
	result.cmd_ = cmd;

	FILE* fp;
	if (!shellStart(result, fp)) return false;
	return shellWait(result, fp, timeoutms);
}

// data passed to pthread function via pointer
struct ShellThreadData
{
public:

	FILE* fp_;
	ShellExecuteResult result_;
	int timeoutms_;
	ShellExecute::ShellExecuteEventHandlerPtr handler_;
	wxEvtHandler* guiHandler_;
	int	wxid_;
};

// Asynch shell execute, notification is via user supplied handler on worker thread
bool ShellExecute::shellAsync(
		const std::wstring &cmd,
		ShellExecuteEventHandlerPtr handler,
		const int userId,
		void* data,
		const int timeoutms)
{
	ShellExecuteResult result;
	result.cmd_ 	 = cmd;
	result.userId_ 	 = userId;
	result.userData_ = data;
	FILE* fp;
	if (!shellStart(result, fp)) return false;

	ShellThreadData* ptr = new ShellThreadData;
	ptr->fp_ 	    = fp;
	ptr->result_ 	= result;
	ptr->timeoutms_ = timeoutms;
	ptr->handler_ 	= handler;

	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &ShellExecute::shellThreadWait, (void*)ptr) != 0)
	{
		Logger::systemError(errno, L"Error creating thread");
		delete ptr;
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
		void *data,
		const int timeoutms)
{
	ShellExecuteResult result;
	result.cmd_ 	 = cmd;
	result.userId_ 	 = userId;
	result.userData_ = data;
	FILE* fp;
	if (!shellStart(result, fp)) return false;

	ShellThreadData* ptr = new ShellThreadData;
	ptr->fp_ 	     = fp;
	ptr->result_ 	 = result;
	ptr->timeoutms_  = timeoutms;
	ptr->guiHandler_ = wxHandler;
	ptr->wxid_ = wxid;

	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &ShellExecute::shellThreadWaitGui, (void*)ptr) != 0)
	{
		Logger::systemError(errno, L"Error creating thread");
		delete ptr;
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
	ShellThreadData *p = (ShellThreadData*)ptr;
	shellWait(p->result_, p->fp_, p->timeoutms_);

	// notify client
	if (p->handler_ != nullptr) p->handler_(p->result_);
	delete p;
}

void *ShellExecute::shellThreadWaitGui(void *ptr)
{
	if (ptr == nullptr) return nullptr;

	// wait for shell to complete
	ShellThreadData *p = (ShellThreadData*)ptr;
	shellWait(p->result_, p->fp_, p->timeoutms_);

	// notify client
	wxShellExecuteResult evt(p->result_, p->wxid_);

	if (p->guiHandler_ != nullptr) p->guiHandler_->AddPendingEvent(evt);
	delete p;
}

bool ShellExecute::shellStart(ShellExecuteResult &result, FILE *&fp)
{
	pid_t child_pid;
	constexpr int kFdRead = 0;
	constexpr int kFdWrite = 1;
	int fd[2] = {0};

	if (pipe(fd) == -1)
	{
	    Logger::systemError(errno, L"Error creating pipe");
	    result.error_ = errno;
	    return false;
	}

	if((child_pid = fork()) == -1)
	{
	    Logger::systemError(errno, L"Error forking process");
	    result.error_ = errno;
		result.success_ = false;
	    return false;
	}

	// child process
	if (child_pid == 0)
	{
	    close(fd[kFdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fd[kFdWrite], 1); // Redirect stdout to pipe

	    setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
	    execl(
	    		"/bin/sh", "/bin/sh", "-c",
				SU::wStrToStr(result.cmd_).c_str(),
				NULL);
	    _exit(0);
	}

	close(fd[kFdWrite]); // Close the WRITE end of the pipe since parent's fd is read-only
	result.pid_ = child_pid;

	fp = fdopen(fd[kFdRead], "r");

	// set read to non-blocking so we can timeout
	int flags = fcntl(fd[kFdRead], F_GETFL);
	fcntl(fd[kFdRead], F_SETFL, flags | O_NONBLOCK);

	return true;
}

bool ShellExecute::shellWait(ShellExecuteResult &result, FILE *fp, const int timeoutms)
{
	char buff[1000] = {0};
	std::stringstream output;

	int rerr;
	long stime = Utilities::getMsCounter();

	while (true)
	{
		rerr = read(fileno(fp), buff, sizeof(buff) - 1);
		if (rerr == 0)
		{
			break;
		}
		else if (rerr < 0)
		{
			if (errno != EWOULDBLOCK)
			{
				fclose(fp);
				Logger::systemError(errno, L"File read error");
				result.error_ = errno;
				result.success_ = false;
				return false;
			}
			else
			{
				if (timeoutms != -1)
				{
					long timer = Utilities::getMsCounter();
					if (timer - stime > timeoutms)
					{
						fclose(fp);
						Logger::error(L"Shell execute timed out");
						result.timedOut_ = true;
						result.success_ = false;
						return false;
					}
				}
			}
		}
		else if (rerr > 0)
		{
			output << std::string(buff);
		}
	}

	// save std output
	result.stdout_ = SU::strToWStr(output.str());

	// close read end of pipe
	int exit;
	fclose(fp);
	while (waitpid(result.pid_, &exit, 0) == -1)
	{
		if (errno != EINTR)
		{
			fclose(fp);
			Logger::systemError(errno, L"Error closing pipe");
			result.error_ = errno;
			result.success_ = false;
			return false;
		}
	}

	// setup return result
	result.exitCode_ 	= exit;
	result.pid_ 		= 0;
	result.success_ 	= exit == 0;

	//notify
	return result.success_;
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

ShellExecuteResult::ShellExecuteResult()
{
	clear();
}

ShellExecuteResult::~ShellExecuteResult()
{
}

ShellExecuteResult::ShellExecuteResult(const ShellExecuteResult &other)
{
	*this = other;
}

ShellExecuteResult::ShellExecuteResult(ShellExecuteResult &&other)
{
	*this = other;
	other.clear();
}

void ShellExecuteResult::clear()
{
	cmd_.clear();
	stdout_.clear();
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
			L"Shell result:\ncommand: %ls\n%ls, id %d, pid: %d, exit code: %d, error: %d%ls\nstdout:\n%ls",
			cmd_.c_str(),
			(success_ ? L"Success" : L"Fail"),
			userId_,
			pid_,
			exitCode_,
			error_,
			(timedOut_ ? L", TimedOut" : L""),
			stdout_.c_str());
	return std::wstring(buf);
}

void ShellExecuteResult::killChildProcess()
{
	if (pid_ == 0) return;

	// to do
}


