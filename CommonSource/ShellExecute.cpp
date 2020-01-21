/*
 * ShellExecute.cpp
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#include "ShellExecute.h"

#include <errno.h>
#include <fcntl.h>
#include <cstdio>
#include <string>
#include <sstream>
//#include <string.h>
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

bool ShellExecute::shell(const std::string &cmd, int &exitCode, std::string &stdout, const int timeoutms)
{
	pid_t child_pid;
	constexpr int fdRead = 0;
	constexpr int fdWrite = 1;
	int fd[2] = {0};

	if (pipe(fd) == -1)
	{
	    Logger::systemError(errno, L"Error creating pipe");
	    return false;
	}

	if((child_pid = fork()) == -1)
	{
	    Logger::systemError(errno, L"Error forking process");
	    return false;
	}

	// child process
	if (child_pid == 0)
	{
	    close(fd[fdRead]);    // Close the READ end of the pipe since the child's fd is write-only
	    dup2(fd[fdWrite], 1); // Redirect stdout to pipe

	    setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
	    execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL);
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
		rerr = read(fileno(fp), buff, sizeof(buff)-1);
		if (rerr == 0)
		{
			break;
		}
		else if (rerr <= 0)
		{
			if (errno != EWOULDBLOCK)
			{
				Logger::systemError(errno, L"File read error");
				fclose(fp);
				return false;
			}
			else
			{
				if (timeoutms != -1)
				{
					long timer = Utilities::getMsCounter();
					if (timer - stime > timeoutms)
					{
						Logger::systemError(errno, L"Shell execute timed out");
						fclose(fp);
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
	int stat;
	int ec = fclose(fp);
	while (waitpid(child_pid, &stat, 0) == -1)
	{
	    if (errno != EINTR)
	    {
			Logger::systemError(errno, L"Error closing pipe");
			fclose(fp);
			return false;
	    }
	}

	exitCode = ec;
	stdout = output.str();
	return ec == 0;
}


//-----------------------------------------------------------
// ShellExecuteEvent

ShellExecuteEvent::ShellExecuteEvent()
{
}

ShellExecuteEvent::~ShellExecuteEvent()
{
}

ShellExecuteEvent::ShellExecuteEvent(const ShellExecuteEvent &other)
{
	*this = other;
}

ShellExecuteEvent::ShellExecuteEvent(ShellExecuteEvent &&other)
{
	*this = other;
	other.stdout_.clear();
}

ShellExecuteEvent& ShellExecuteEvent::operator=(const ShellExecuteEvent &other)
{
	if (&other != this)
	{
		pid_ 	= other.pid_;
		error_ 	= other.error_;
		stdout_ = other.stdout_;
	}
	return *this;
}

int ShellExecuteEvent::getError() const
{
	return error_;
}

int ShellExecuteEvent::getPid() const
{
	return pid_;
}

std::string ShellExecuteEvent::getStdout() const
{
	return stdout_;
}
