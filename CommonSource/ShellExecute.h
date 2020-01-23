/*
 * ShellExecute.h
 *
 *  Created on: 20 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_SHELLEXECUTE_H_
#define COMMON_SHELLEXECUTE_H_

#include <string>
#include <wx/wx.h>


class ShellExecuteResult;

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

	using ShellExecuteEventHandlerPtr = void (*)(ShellExecuteResult &result);

	// simple synchronous, no result object returned
	// returns true if command executed ok
	static bool shellSync(
			const std::wstring &cmd,
			const int timeoutms = -1);

	// synchronous with user supplied result object filled out with the
	// result of the command
	static bool shellSync(
			const std::wstring &cmd,
			ShellExecuteResult &result,
			const int timeoutms = -1);

	// asynchronous, event handler function optional
	static bool shellAsync(
			const std::wstring &cmd,
			ShellExecuteEventHandlerPtr handler = nullptr,
			const int userId = 0,
			void* data = nullptr,
			const int timeoutms = -1);

	// asynchronous, wxEventHandler optional
	static bool shellAsyncGui(
			const std::wstring &cmd,
			wxEvtHandler *wxHandler = nullptr,
			const int wxid = wxID_ANY,
			const int userId = 0,
			void *data = nullptr,
			const int timeoutms = -1);

private:

	static void *shellThreadWait(void *ptr);
	static void *shellThreadWaitGui(void *ptr);
	static bool shellStart(ShellExecuteResult &result, FILE *&fp);
	static bool shellWait(ShellExecuteResult &result_, FILE *fp_, const int timeoutms);
};

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

	std::wstring getCmd() const;
	int  getPid() const;
	int  getExitCode() const;
	bool getSuccess() const;
	int  getError() const;
	std::wstring getStdout() const;
	bool getTimedOut() const;
	int  getUserId() const;
	void *getUserData() const;
	std::wstring toString() const;

	void killChildProcess();	// after a timeout this can be used to kill
								// the hanging process
	void clear();

	friend class ShellExecute;

private:

	std::wstring 	cmd_;		// command executed
	int 			pid_;		// process id of child shell, 0 if already destroyed
	int 			exitCode_;	// exit code from command
	bool 			success_;	// executed successfully
	int 			error_;		// system error code
	std::wstring 	stdout_;	// stdout capture
	bool 			timedOut_;	// true if command timedout

	// async calls only
	int				userId_;	// id set by user, allows the same handler to handle more than one event
	void* 			userData_;	// user data
};

//----------------------------------------------------------------------------
// Shell execute event, used to notify wxWidgets GUI of completion of shell execute
//
// add the following to the message map for the wxEventHandler (e.g. wxFrame)
//
// 	wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
//		...
//		EVT_SHELL_EXECUTE_RESULT_COMMAND(wxID_ANY, MyFrame::OnShellExecute)
//		...
//	wxEND_EVENT_TABLE()
//
// where the handler method OnShellExecute() is;
//
//		void MyFrame::OnShellExecute(wxShellExecuteResult& event)
//		{
//			ShellExecuteResult result = event.getResult();
//
//			// get user data as set in call to shellAsyncGui( ..., ..., data, ...)
//			MyData* data = (MyData *)result.getUserData();
//			...
//			event.Skip();
//		}
//
// To have more than one handler in the wxWidgets map, the wxid must be set to something other than wxID_ANY
// in the event table map and must be the the same as that set in the call to shellAsyncGui().
//----------------------------------------------------------------------------

class wxShellExecuteResult : public wxCommandEvent
{
public:
	wxShellExecuteResult();
	wxShellExecuteResult(ShellExecuteResult &result, int wxid = wxID_ANY);
	wxShellExecuteResult(const wxShellExecuteResult &other);

	virtual wxEvent *Clone() const;

	ShellExecuteResult getResult() const;

	DECLARE_DYNAMIC_CLASS(wxShellExecuteResult)

private:

	ShellExecuteResult result_;
};

typedef void (wxEvtHandler::*wxShellExecuteEventFunction)(wxShellExecuteResult&);
#define wxShellExecuteResultHandler(func) \
    wxEVENT_HANDLER_CAST(wxShellExecuteEventFunction, func)

wxDECLARE_EVENT(wxEVT_SHELL_EXECUTE_RESULT, wxShellExecuteResult);

// for use in message maps
#define EVT_SHELL_EXECUTE_RESULT_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    		wxEVT_SHELL_EXECUTE_RESULT, id, wxID_ANY, \
		wxShellExecuteResultHandler(fn), \
        (wxObject *) NULL \
    ),


#endif /* COMMON_SHELLEXECUTE_H_ */
