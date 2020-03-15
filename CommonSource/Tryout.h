#ifndef TRYOUT_H
#define TRYOUT_H

#include <wx/wx.h>

wxDECLARE_EVENT(wxEVT_MY_CUSTOM_COMMAND, wxCommandEvent);

class TryOut
{
public:
	TryOut() = default;
	~TryOut() = default;

	static void ThreadEvents(wxEvtHandler* parent);
	static void ExecIt();
	static void WorkerThread();
	static void AsyncShell(wxEvtHandler *handler);
	static void ReadAsync();
	static void ShellAsync();


};


#endif 
