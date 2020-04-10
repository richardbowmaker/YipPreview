/*
 * TestDialog.cpp
 *
 *  Created on: 15 Mar 2020
 *      Author: richard
 */

#include "TestDialog.h"

#include <wx/wx.h>
#include <wx/mediactrl.h>

#include "Constants.h"
#include "Logger.h"
#include "MediaPreviewPlayer.h"


int TestDialog::Run(wxWindow *parent, FileSetT &fileset)
{
	TestDialog dlg(parent, fileset);
	return dlg.ShowModal();
}

TestDialog::TestDialog(wxWindow *parent, FileSetT &fileset) :
	DialogEx(parent, wxID_ANY, wxSize(1000, 1000), wxOK | wxCANCEL),
	fileset_{fileset},
	player_{nullptr}
{
	setTitle("Test dialog");

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	getPanel()->SetSizer(sizer);

	player_ = new wxMediaCtrl();

#ifdef WINDOWS_BUILD
	// force windows media player
	player_->Create(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxMEDIABACKEND_WMP10);
#elif LINUX_BUILD
	player_->Create(this, wxID_ANY);
#endif

    sizer->Add(player_, 1, wxSHAPED | wxCENTRE);
	player_->Bind(wxEVT_MEDIA_LOADED, &TestDialog::onMediaPlay, this, wxID_ANY);

	if (fileset_->hasVideo())
		player_->Load(fileset_->getVideo().c_str());

	player_->ShowPlayerControls();
}

void TestDialog::onOk(wxCommandEvent &event)
{
	//player_

//	DialogEx::onOk(event);
}

void TestDialog::onCancel(wxCommandEvent &event)
{
	player_->Stop();
	DialogEx::onCancel(event);
}

void TestDialog::onMediaPlay(wxMediaEvent &event)
{
	player_->Play();
	event.Skip();
}





