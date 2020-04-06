/*
 * DialogEx.cpp
 *
 *  Created on: 10 Mar 2020
 *      Author: richard
 */

#include "DialogEx.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <assert.h>
#include <string>

#include "Constants.h"
#include "Utilities.h"


int DialogEx::Run(wxWindow* parent)
{
	DialogEx dlg(parent, wxID_ANY, wxSize(200, 200));
	return dlg.ShowModal();
}

DialogEx::DialogEx(
		wxWindow *parent,
		wxWindowID id,
		const wxSize &size,
		const int buttons 	  /*= wxID_OK*/,
		const wxString &title /*= wxEmptyString */,
		const wxPoint &pos 	  /*= wxDefaultPosition*/,
		long style 			  /*= wxDEFAULT_DIALOG_STYLE*/,
		const wxString &name  /*= wxDialogNameStr*/) :
	wxDialog(parent, id, title, pos, size, style, name),
	panel_	{nullptr},
	ok_		{nullptr},
	cancel_ {nullptr},
	yes_	{nullptr},
	no_		{nullptr},
	apply_	{nullptr},
	close_	{nullptr},
	help_	{nullptr}
{
	SetTitle(title);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// add standard buttons
	wxSizer *buttonSizer = CreateButtonSizer(buttons);

	if (buttons & wxOK)
	{
		ok_ = dynamic_cast<wxButton *>(FindWindowById(wxID_OK, this));
		getOk()->Bind(wxEVT_BUTTON, &DialogEx::onOk, this);
	}
	if (buttons & wxCANCEL)
	{
		cancel_ = dynamic_cast<wxButton *>(FindWindowById(wxID_CANCEL, this));
		getCancel()->Bind(wxEVT_BUTTON, &DialogEx::onCancel, this);
	}
	if (buttons & wxYES)
	{
		yes_ = dynamic_cast<wxButton *>(FindWindowById(wxID_YES, this));
		getCancel()->Bind(wxEVT_BUTTON, &DialogEx::onYes, this);
	}
	if (buttons & wxNO)
	{
		no_ = dynamic_cast<wxButton *>(FindWindowById(wxID_NO, this));
		getCancel()->Bind(wxEVT_BUTTON, &DialogEx::onNo, this);
	}
	if (buttons & wxAPPLY)
	{
		apply_ = dynamic_cast<wxButton *>(FindWindowById(wxID_APPLY, this));
		getCancel()->Bind(wxEVT_BUTTON, &DialogEx::onApply, this);
	}
	if (buttons & wxCLOSE)
	{
		close_ = dynamic_cast<wxButton *>(FindWindowById(wxID_CLOSE, this));
		getCancel()->Bind(wxEVT_BUTTON, &DialogEx::onClose, this);
	}
	if (buttons & wxHELP)
	{
		help_ = dynamic_cast<wxButton *>(FindWindowById(wxID_HELP, this));
		getCancel()->Bind(wxEVT_BUTTON, &DialogEx::onHelp, this);
	}

	// set sizer proportions so that dialog buttons
	// have just enough height	
	int h = ok_->GetSize().GetHeight();
	panel_ = new wxPanel(this);
    sizer->Add(panel_, size.GetHeight() - h, wxEXPAND);
	sizer->Add(buttonSizer, h, wxTOP | wxBOTTOM, 5);
}

void DialogEx::setTitle(const std::string title)
{
	SetLabel(Constants::title + wxString(" - ") + title);
}

void DialogEx::setTitle(const std::string title, FileSetT fileset)
{
	SetLabel(Constants::title +
			 wxString(" - ") +
			 title +
			 wxString(" - ") +
			 fileset->getId());
}

wxPanel *DialogEx::getPanel()
{
	return panel_;
}

wxButton *DialogEx::getOk()
{
	assert(ok_);
	return ok_;
}

wxButton *DialogEx::getCancel()
{
	assert(cancel_);
	return cancel_;
}

wxButton *DialogEx::getYes()
{
	assert(yes_);
	return yes_;
}

wxButton *DialogEx::getNo()
{
	assert(no_);
	return no_;
}

wxButton *DialogEx::getApply()
{
	assert(apply_);
	return apply_;
}

wxButton *DialogEx::getClose()
{
	assert(close_);
	return close_;
}

wxButton *DialogEx::getHelp()
{
	assert(help_);
	return help_;
}
