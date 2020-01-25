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
	ShellExecuteResult result;	// throw away
	result.cmd_ = cmd;

	FILE* fp1;
	FILE* fp2;
	if (!shellStart(result, fp1, fp2)) return false;
	return shellWait(result, fp1, fp2, timeoutms);
}

// Synchronous shell execute that returns the result data
bool ShellExecute::shellSync(
		const std::wstring &cmd,
		ShellExecuteResult &result,
		const int timeoutms /*= -1 */)
{
	result.clear();
	result.cmd_ = cmd;

	FILE* fp1;
	FILE* fp2;
	if (!shellStart(result, fp1, fp2)) return false;
	return shellWait(result, fp1, fp2, timeoutms);
}

// data passed to pthread function via pointer
struct ShellThreadData
{
public:

	FILE* fp1_;
	FILE* fp2_;
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
	FILE* fp1;
	FILE* fp2;
	if (!shellStart(result, fp1, fp2)) return false;

	ShellThreadData* ptr = new ShellThreadData;
	ptr->fp1_ 	    = fp1;
	ptr->fp2_ 	    = fp2;
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
	FILE* fp1;
	FILE* fp2;
	if (!shellStart(result, fp1, fp2)) return false;

	ShellThreadData* ptr = new ShellThreadData;
	ptr->fp1_ 	    = fp1;
	ptr->fp2_ 	    = fp2;
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
	shellWait(p->result_, p->fp1_, p->fp2_, p->timeoutms_);

	// notify client
	if (p->handler_ != nullptr) p->handler_(p->result_);
	delete p;
}

void *ShellExecute::shellThreadWaitGui(void *ptr)
{
	if (ptr == nullptr) return nullptr;

	// wait for shell to complete
	ShellThreadData *p = (ShellThreadData*)ptr;
	shellWait(p->result_, p->fp1_, p->fp2_, p->timeoutms_);

	// notify client
	wxShellExecuteResult evt(p->result_, p->wxid_);

	if (p->guiHandler_ != nullptr) p->guiHandler_->AddPendingEvent(evt);
	delete p;
}

bool ShellExecute::shellStart(ShellExecuteResult &result, FILE *&fp1, FILE *&fp2)
{
	pid_t child_pid;
	constexpr int kFdRead = 0;
	constexpr int kFdWrite = 1;
	int fd1[2] = {0};
	int fd2[2] = {0};

	if (pipe(fd1) == -1)
	{
	    Logger::systemError(errno, L"Error creating pipe");
	    result.error_ = errno;
	    return false;
	}

	if (pipe(fd2) == -1)
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
	    close(fd1[kFdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fd1[kFdWrite], 1); // Redirect stdout to pipe

	    close(fd2[kFdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fd2[kFdWrite], 2); // Redirect stderr to pipe

	    setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
	    execl(
	    		"/bin/sh", "/bin/sh", "-c",
				SU::wStrToStr(result.cmd_).c_str(),
				NULL);
	    _exit(0);
	}

	close(fd1[kFdWrite]); // Close the WRITE end of the pipe since parent's fd is read-only
	close(fd2[kFdWrite]);
	result.pid_ = child_pid;

	fp1 = fdopen(fd1[kFdRead], "r");
	fp2 = fdopen(fd2[kFdRead], "r");

	// set read to non-blocking so we can timeout
	int flags = fcntl(fd1[kFdRead], F_GETFL);
	fcntl(fd1[kFdRead], F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(fd2[kFdRead], F_GETFL);
	fcntl(fd2[kFdRead], F_SETFL, flags | O_NONBLOCK);

	return true;
}

bool ShellExecute::shellWait(ShellExecuteResult &result, FILE *&fp1, FILE *&fp2, const int timeoutms)
{
	char buff[100000] = {0};
	std::stringstream stdout;
	std::stringstream stderr;

	int rerr;
	long stime = Utilities::getMsCounter();

	bool bout = true;
	bool berr = true;

	while (true)
	{
		// std out handling
		buff[0] = 0;
		rerr = read(fileno(fp1), buff, sizeof(buff) - 1);
		if (rerr == 0)
		{
			bout = false;
			if (!berr) break;
		}
		else if (rerr < 0)
		{
			if (errno != EWOULDBLOCK)
			{
				fclose(fp1);
				fclose(fp2);
				Logger::systemError(errno, L"File stdout read error");
				result.error_ = errno;
				result.success_ = false;
				return false;
			}
		}
		else if (rerr > 0)
		{
			stdout << std::string(buff);
		}

		// std err handling
		buff[0] = 0;
		rerr = read(fileno(fp2), buff, sizeof(buff) - 1);
		if (rerr == 0)
		{
			berr = false;
			if (!bout) break;
		}
		else if (rerr <= 0)
		{
			if (errno != EWOULDBLOCK)
			{
				fclose(fp1);
				fclose(fp2);
				Logger::systemError(errno, L"File stderr read error");
				result.error_ = errno;
				result.success_ = false;
				return false;
			}
		}
		else if (rerr > 0)
		{
			std::string s = std::string(buff);
			std::cout << "-----------------------------------" << std::endl;
			std::cout << s << std::endl;
			stderr << std::string(buff);
		}

		// check for timedout
		if (timeoutms != -1)
		{
			long timer = Utilities::getMsCounter();
			if (timer - stime > timeoutms)
			{
				fclose(fp1);
				fclose(fp2);
				Logger::error(L"Shell execute timed out");
				result.timedOut_ = true;
				result.success_ = false;
				return false;
			}
		}
	}

	// save std output
	result.stdout_ = SU::strToWStr(stdout.str());
	result.stderr_ = SU::strToWStr(stderr.str());

	// close read end of pipe
	int exit;
	fclose(fp1);
	fclose(fp2);
	while (waitpid(result.pid_, &exit, 0) == -1)
	{
		if (errno != EINTR)
		{
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

	std::cout << "-----------------------------------" << std::endl;
	std::cout << "-----------------------------------" << std::endl;
	std::cout << stderr.str() << std::endl;

	std::cout << "-----------------------------------" << std::endl;
	std::cout << "-----------------------------------" << std::endl;

	std::wstring str(SU::strToWStr(stderr.str())), temp;
	std::wstringstream wss(str);
	while(std::getline(wss, temp, L'\n'))
	{
		std::cout << temp << std::endl;
	}




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


