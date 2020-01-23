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

class ShellExecute
{
public:
	ShellExecute();
	virtual ~ShellExecute();

	using ShellExecuteEventHandlerPtr = void (*)(ShellExecuteResult &result);

	static bool shellSync(
			const std::wstring &cmd,
			const int timeoutms = -1);

	static bool shellSync(
			const std::wstring &cmd,
			ShellExecuteResult &result,
			const int timeoutms = -1);

	static bool shellAsync(
			const std::wstring &cmd,
			const int timeoutms,
			const int id,
			void* data,
			ShellExecuteEventHandlerPtr handler);

	static bool shellAsyncGui(
			const std::wstring &cmd,
			const int timeoutms,
			const int id,
			void *data,
			wxEvtHandler *wxHandler);

private:

	static void* shellAsyncWait(void *ptr);
	static void* shellAsyncWaitGui(void *ptr);
	static bool startShell(ShellExecuteResult &result, FILE *&fp);
	static bool shellWait(ShellExecuteResult &result_, FILE *fp_, const int timeoutms);
};

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
	int  getId() const;
	void* getUserData() const;
	std::wstring toString() const;

	void killChildProcess();
	void clear();

	friend class ShellExecute;

private:

	std::wstring 	cmd_;		// command executed
	int 			pid_;		// process id of child shell, 0 if destroyed
	int 			exitCode_;	// exit code from command
	bool 			success_;	// executed successfully
	int 			error_;		// system error code
	std::wstring 	stdout_;	// stdout capture
	bool 			timedOut_;	// true if command timedout
	int				id_;		// id set by user, allows the same handler to handle more than one event
	void* 			userData_;	// user data, asynch shell calls only only
};

//----------------------------------------------------------------------------
// shell execute  event, used to notify GUI of completion of shell execute
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
//		}
//


class wxShellExecuteResult : public wxCommandEvent
{
public:
	wxShellExecuteResult();
	wxShellExecuteResult(ShellExecuteResult &result);
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
