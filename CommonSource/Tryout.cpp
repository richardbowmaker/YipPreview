


#include <wx/wx.h>
#include <wx/thread.h>

#include "Logger.h"
#include "Tryout.h"
#include "Events.h"



wxDEFINE_EVENT(wxEVT_MY_CUSTOM_COMMAND, wxCommandEvent);

// a thread class that will periodically send events to the GUI thread
class MyThread : public wxThread
{
protected:
	wxEvtHandler* m_parent;

public:
	explicit MyThread(wxEvtHandler* parent) : wxThread(), m_parent(parent) {}

	ExitCode Entry() override
	{
		for (unsigned n = 0; n < 1; n++)
		{
			// notify the main thread
//			wxCommandEvent evt(wxEVT_MY_CUSTOM_COMMAND, wxID_ANY);
			wxLoggerEvent evt;
			evt.SetInt(n);
			evt.SetString(wxString(L"From thread"));
			evt.setLevel(22);
			m_parent->AddPendingEvent(evt);

			this->Sleep(500);
		}

		return ExitCode(nullptr);
	}

};

MyThread* thread;

void TryOut::ThreadEvents(wxEvtHandler* parent)
{
	thread = new MyThread(parent);

	wxThreadError err = thread->Create();

	if (err != wxTHREAD_NO_ERROR)
	{
		Logger::error(L"Couldn't create thread!");
		return;
	}

	err = thread->Run();

	if (err != wxTHREAD_NO_ERROR)
	{
		Logger::error(L"Couldn't run thread!");
		return;
	}



}

///////////////////////////////
/*
class MyFrame : public wxFrame
{
private:
	DECLARE_EVENT_TABLE()
	MyThread* m_pThread;
	wxStaticText* m_static_text;

public:
	MyFrame() : wxFrame(nullptr, wxID_ANY, wxT("Hello wxWidgets"), wxPoint(50, 50), wxSize(800, 600))
	{
		// create the thread
		m_pThread = new MyThread(this);
		wxThreadError err = m_pThread->Create();

		if (err != wxTHREAD_NO_ERROR)
		{
			wxMessageBox(_("Couldn't create thread!"));
			return;
		}

		err = m_pThread->Run();

		if (err != wxTHREAD_NO_ERROR)
		{
			wxMessageBox(_("Couldn't run thread!"));
			return;
		}

		const auto panel = new wxPanel(this);
		const auto sizer = new wxBoxSizer(wxHORIZONTAL);
		m_static_text = new wxStaticText(panel, wxID_ANY, "Counter: 0", wxPoint(50, 50), wxSize(200, 200));
		m_static_text->Show(true);

		sizer->Add(m_static_text);
		panel->SetSizer(sizer);
	}

	~MyFrame()
	{
		m_pThread = nullptr;
	}

	void OnThread(wxCommandEvent& evt)
	{
		m_static_text->SetLabelText(wxString::Format("Counter: %i", evt.GetInt()));
	}
};

// catch the event from the thread
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_COMMAND(wxID_ANY, wxEVT_MY_EVENT, MyFrame::OnThread)
END_EVENT_TABLE()

class MyApp : public wxApp
{
private:
	wxFrame* m_frame;

public:
	bool OnInit() override
	{
		m_frame = new MyFrame();
		m_frame->Show();

		return true;
	}
};

IMPLEMENT_APP(MyApp)

*/
