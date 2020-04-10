/*
 * ShellExecute.cpp
 *
 *
 *  Linux version
 *
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
	FILE* fpStdout_;
	FILE* fpStderr_;
}

// helpers
void *shellThreadWait(void *ptr);
void *shellThreadWaitGui(void *ptr);
bool shellStart(ShellThreadData &data);
bool shellWait(ShellThreadData &data);

// shell
bool ShellExecute::shell(const std::string &cmd)
{
	Logger::info("ShellExecute::shell {}", cmd);
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
	if (!shellStart(data)) return false;
	return shellWait(data);
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
	if (!shellStart(data)) return false;
	res = shellWait(data);
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

	if (!shellStart(*data)) return false;
	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &shellThreadWait, (void*)data) != 0)
	{
		Logger::systemError("Error creating thread");
		delete data;
		return false;
	}
	// allow client to continue
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

	if (!shellStart(*data)) return false;
	pthread_t thread_;
	if (pthread_create(&thread_, NULL, &shellThreadWaitGui, (void*)data) != 0)
	{
		Logger::systemError("Error creating thread");
		delete data;
		return false;
	}
	// allow client to continue
	return true;
}

//-----------------------------------------------------------

void *shellThreadWait(void *ptr)
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

void *shellThreadWaitGui(void *ptr)
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

bool shellStart(ShellThreadData &data)
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

bool shellWait(ShellThreadData &data)
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

ShellThreadData::ShellThreadData() :
	timeoutms_(0),
	wxHandler_(nullptr),
	wxid_     (wxID_ANY),
	handler_  (nullptr),
	fpStdout_ (0),
	fpStderr_ (0)
{
}

