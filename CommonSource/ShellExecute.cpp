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

bool ShellExecute::shellSync(const std::wstring &cmd, const int timeoutms /*= -1*/)
{
	ShellExecuteResult result;	// throw away
	return shell_(cmd, timeoutms, nullptr, result);
}

bool ShellExecute::shellSync(const std::wstring &cmd, ShellExecuteResult &result, const int timeoutms /*= -1 */)
{
	return shell_(cmd, timeoutms, nullptr, result);
}

// core shell execute function
bool ShellExecute::shell_(
	const std::wstring &cmd,
	const int timeoutms,
	void* data,
	ShellExecuteResult &result)
{
	result.clear();
	result.cmd_ = cmd;

	pid_t child_pid;
	constexpr int fdRead = 0;
	constexpr int fdWrite = 1;
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
	    close(fd[fdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fd[fdWrite], 1); // Redirect stdout to pipe

	    setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
	    execl("/bin/sh", "/bin/sh", "-c", SU::wStrToStr(cmd).c_str(), NULL);
	    _exit(0);
	}

	close(fd[fdWrite]); // Close the WRITE end of the pipe since parent's fd is read-only
	FILE* fp = fdopen(fd[fdRead], "r");

	char buff[1000] = {0};
	std::stringstream output;

	// set read to non-blocking so we can timeout
	int flags = fcntl(fd[fdRead], F_GETFL);
	fcntl(fd[fdRead], F_SETFL, flags | O_NONBLOCK);

	int rerr;
	long stime = Utilities::getMsCounter();

	while (true)
	{
		rerr = read(fileno(fp), buff, sizeof(buff) - 1);
		if (rerr == 0)
		{
			break;
		}
		else if (rerr <= 0)
		{
			if (errno != EWOULDBLOCK)
			{
				fclose(fp);
				Logger::systemError(errno, L"File read error");
				result.pid_ = child_pid;
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
						Logger::systemError(errno, L"Shell execute timed out");
						result.pid_ = child_pid;
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

	// close read end of pipe
	int exit;
	fclose(fp);
	while (waitpid(child_pid, &exit, 0) == -1)
	{
	    if (errno != EINTR)
	    {
			fclose(fp);
			Logger::systemError(errno, L"Error closing pipe");
			result.pid_ = child_pid;
		    result.error_ = errno;
			return false;
	    }
	}

	// setup return result
	result.exitCode_ 	= exit;
	result.stdout_ 		= SU::strToWStr(output.str());
	result.pid_ 		= 0;
	result.success_ 	= exit == 0;
	return result.success_;
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
	pid_ = 0;
	exitCode_ = 0;
	success_ = false;
	error_ = 0;
	stdout_.clear();
	timedOut_ = false;
	userData_ = nullptr;
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

void* ShellExecuteResult::getUserData() const
{
	return userData_;
}

std::wstring ShellExecuteResult::toString() const
{
	wchar_t buf[4000];
	swprintf(buf, sizeof(buf) / sizeof(wchar_t),
			L"Shell result:\ncommand: %ls\n%ls, pid: %d, exit code: %d, error: %d%ls\nstdout:\n%ls",
			cmd_.c_str(),
			(success_ ? L"Success" : L"Fail"),
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


