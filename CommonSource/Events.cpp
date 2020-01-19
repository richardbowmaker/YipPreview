/*
 * Event.cpp
 *
 *  Created on: 19 Jan 2020
 *      Author: richard
 */

#include "Events.h"




DEFINE_EVENT_TYPE(wxLOGGER_EVENT);
IMPLEMENT_DYNAMIC_CLASS(wxLoggerEvent, wxCommandEvent )


wxDEFINE_EVENT(wxEVT_LOGGER_EVENT, wxLoggerEvent);


wxLoggerEvent::wxLoggerEvent() :
	wxCommandEvent(wxEVT_LOGGER_EVENT, wxID_ANY),
	level_(0)
{
}

wxLoggerEvent::wxLoggerEvent(const wxLoggerEvent &other) :
	wxCommandEvent(other)
{
	level_ = other.level_;
}

wxEvent *wxLoggerEvent::Clone() const
{
	return new wxLoggerEvent(*this);
};

int wxLoggerEvent::getLevel() const
{
	return level_;
}

void wxLoggerEvent::setLevel(int level)
{
	level_ = level;
}


