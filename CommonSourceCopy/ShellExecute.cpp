/*
 * ShellExecute.cpp
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#include "ShellExecute.h"

#include "Logger.h"
#include "Utilities.h"

ShellExecute::ShellExecute()
{
}

ShellExecute::~ShellExecute()
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


