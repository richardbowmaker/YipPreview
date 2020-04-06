/*
 * ShellExecute.h
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_SHELLEXECUTE_H_
#define COMMON_SHELLEXECUTE_H_

#ifdef WINDOWS_BUILD
	#include <string>
	#include <Windows.h>
	#include <wx/wx.h>
#elif LINUX_BUILD
	#include <string>
	#include <wx/wx.h>
#endif

//----------------------------------------------------------------------------
// Shell execute result, when a client is notified of shell execute completion
// an instance of this class is provided with the result of the shell.
//----------------------------------------------------------------------------

class ShellExecuteResult
{
public:
	ShellExecuteResult();
	ShellExecuteResult(const ShellExecuteResult &other);
	ShellExecuteResult(ShellExecuteResult &&other);
	virtual ~ShellExecuteResult();
	ShellExecuteResult& operator=(const ShellExecuteResult &other);
	ShellExecuteResult& operator=(ShellExecuteResult &&other);

	std::string getCmd() const;
	int  getPid() const;
	int  getExitCode() const;
	bool getSuccess() const;
	std::string getStdout() const;
	std::string getStderr() const;
	bool getTimedOut() const;
	int  getUserId() const;
	void *getUserData() const;
	std::string toString() const;

	void killChildProcess();	// after a timeout this can be used to kill
								// the hanging process
	void clear();

	friend class ShellExecute;

private:

	std::string 	cmd_;		// command executed
	int 			pid_;		// process id of the shell process, the shell itself has a child process
								// which is the command being run. 0 if exited
	int 			exitCode_;	// exit code from command
	bool 			success_;	// executed successfully
	std::string 	stderr_;
	std::string 	stdout_;	// stdout capture
	bool 			timedOut_;	// true if command timedout

	// async calls only
	int				userId_;	// id set by user, allows the same handler to handle more than one event
	void* 			userData_;	// user data
};


//----------------------------------------------------------------------------
// Shell execute functions, synchronous and asynchronous.
// Asynchronous functions provide a callback on a worker thread
// or a wxWidgets event on the main GUI thread.
//----------------------------------------------------------------------------

class ShellExecute
{
public:
	ShellExecute();
	virtual ~ShellExecute();

	using ShellExecuteEventHandlerT = void (*)(ShellExecuteResult &result);

	// simple synchronous, no result object returned
	// returns true if command executed ok
	static bool shell(const std::string &cmd);

	// simple synchronous, no result object returned
	// returns true if command executed ok
	static bool shellSync(
			const std::string &cmd,
			const int timeoutms = -1);

	// synchronous with user supplied result object filled out with the
	// result of the command
	static bool shellSync(
			const std::string &cmd,
			ShellExecuteResult &result,
			const int timeoutms = -1);

	// asynchronous, event handler function optional
	static bool shellAsync(
			const std::string &cmd,
			ShellExecuteEventHandlerT handler = nullptr,
			const int userId = 0,
			void* userData = nullptr,
			const int timeoutms = -1);

	// asynchronous, wxEventHandler optional
	static bool shellAsyncGui(
			const std::string &cmd,
			wxEvtHandler *wxHandler = nullptr,
			const int wxid = wxID_ANY,
			const int userId = 0,
			void *userData = nullptr,
			const int timeoutms = -1);

	// data passed to pthread function via pointer
	struct ShellThreadData
	{
	public:

		ShellThreadData();

		ShellExecuteResult 	result_;
		int 				timeoutms_;
		wxEvtHandler*		wxHandler_;
		int					wxid_;
		ShellExecuteEventHandlerT handler_;

#ifdef WINDOWS_BUILD
		HANDLE completed_;
#elif LINUX_BUILD
		FILE* fpStdout_;
		FILE* fpStderr_;
#endif

	};

private:

#ifdef WINDOWS_BUILD
	static bool shellWin_(ShellThreadData& data);
	static DWORD WINAPI shellWinThread1_(void* pdata);
	static DWORD WINAPI shellWinThread2_(void* pdata);
#elif LINUX_BUILD
	static void *shellThreadWait(void *ptr);
	static void *shellThreadWaitGui(void *ptr);
	static bool shellStart(ShellThreadData &data);
	static bool shellWait(ShellThreadData &data);
#endif
};

//----------------------------------------------------------------------------
// Shell execute event, used to notify wxWidgets GUI of completion of shell execute
//
// add a handler either method as follows to the wxEvtHandler class, e.g. wxFrame
//
//		void MyFrame::onShellExecute(wxShellExecuteEvent& event)
//		{
//			ShellExecuteResult result = event.getResult();
//
//			// get user data as set in call to shellAsyncGui( ..., ..., data, ...)
//			MyData* data = (MyData *)result.getUserData();
//			...
//			event.Skip();
//		}
//
// map the handler to the event either via a message map, directly, via lambda or functor
//
// message map :-
//
// 	wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
//		...
//		EVT_SHELL_EXECUTE_RESULT_COMMAND(wxID_ANY, MyFrame::onShellExecute)
//		...
//	wxEND_EVENT_TABLE()
//
// bind directly to handler:-
//
// 	Bind(wxEVT_SHELL_EXECUTE_RESULT, &MyFrame::onShellExecute, this);
//
// bind via lambda :-
//
// 	Bind(wxEVT_SHELL_EXECUTE_RESULT, [this](wxShellExecuteEvent& e) -> void { onShellExecute(e); }, wxID_ANY);
//
// bind via functor; (#include <functional>)
//
//	std::function< void(wxShellExecuteEvent&) >
//	shellHandler(std::bind(&MyFrame::onShellExecute, this, std::placeholders::_1));
//	Bind(wxEVT_SHELL_EXECUTE_RESULT, shellHandler, wxID_ANY);
//
// bind to lambda  :-
//
// 	Bind(wxEVT_SHELL_EXECUTE_RESULT, [this](wxShellExecuteEvent& e) -> void {-handling code-;}, wxID_ANY);
//
// To have more than one handler, the wxgrid must be set to something other than wxID_ANY
// in the event table map and must be the the same as that set in the call to shellAsyncGui().
//----------------------------------------------------------------------------

class wxShellExecuteEvent : public wxCommandEvent
{
public:
	wxShellExecuteEvent();
	wxShellExecuteEvent(ShellExecuteResult &result, int wxid = wxID_ANY);
	wxShellExecuteEvent(const wxShellExecuteEvent &other);

	virtual wxEvent *Clone() const;

	ShellExecuteResult getResult() const;

	DECLARE_DYNAMIC_CLASS(wxShellExecuteEvent)

private:

	ShellExecuteResult result_;
};

typedef void (wxEvtHandler::*wxShellExecuteEventFunction)(wxShellExecuteEvent&);
#define wxShellExecuteResultHandler(func) \
    wxEVENT_HANDLER_CAST(wxShellExecuteEventFunction, func)

wxDECLARE_EVENT(wxEVT_SHELL_EXECUTE_RESULT, wxShellExecuteEvent);

// for use in message maps
#define EVT_SHELL_EXECUTE_RESULT_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    		wxEVT_SHELL_EXECUTE_RESULT, id, wxID_ANY, \
		wxShellExecuteResultHandler(fn), \
        (wxObject *) NULL \
    ),


#endif /* COMMON_SHELLEXECUTE_H_ */
