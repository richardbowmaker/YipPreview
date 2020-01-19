/*
 * Event.h
 *
 *  Created on: 19 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_EVENTS_H_
#define COMMON_EVENTS_H_

#include <wx/wx.h>


class wxLoggerEvent : public wxCommandEvent
{
public:
	wxLoggerEvent();
	wxLoggerEvent(const wxLoggerEvent &other);

	virtual wxEvent *Clone() const;

	int getLevel() const;
	void setLevel(int level);

	DECLARE_DYNAMIC_CLASS(wxLoggerEvent)

private:

	int level_;
};


typedef void (wxEvtHandler::*wxLoggerEventFunction)(wxLoggerEvent&);
#define wxLoggerEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxLoggerEventFunction, func)


wxDECLARE_EVENT(wxEVT_LOGGER_EVENT, wxLoggerEvent);

// for use in message maps
#define EVT_LOGGER_EVENT_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    	wxEVT_LOGGER_EVENT, id, wxID_ANY, \
		wxLoggerEventHandler(fn), \
        (wxObject *) NULL \
    ),

class Events
{
public:

	virtual ~Events() = default;

private:

	Events() = default;
	Events(const Events&) = default;
	Events(const Events&&);



};

#endif /* COMMON_EVENTS_H_ */
