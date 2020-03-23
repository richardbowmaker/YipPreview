/*
 * TestDialog.cpp
 *
 *  Created on: 15 Mar 2020
 *      Author: richard
 */

#include "TestDialog.h"

#include "Constants.h"
#include "Logger.h"
#include "MediaPreviewPlayer.h"


int TestDialog::Run(wxWindow *parent, FileSetT &fileset)
{
	TestDialog dlg(parent, fileset);
	return dlg.ShowModal();
}

TestDialog::TestDialog(wxWindow *parent, FileSetT &fileset) :
	DialogEx(parent, wxID_ANY, wxSize(400, 400), wxOK | wxCANCEL),
	fileset_(fileset),
	player_(nullptr),
	panel_(nullptr),
	sizer_(nullptr),
	button_(nullptr)
{
	setTitle(L"Test dialog", fileset_);

	sizer_ = new wxBoxSizer(wxVERTICAL);
	panel_ = new wxPanel(getPanel());
	panel_->SetBackgroundColour(Constants::lightBlue);
	sizer_->Add(panel_, 10, wxEXPAND);

	button_ = new wxButton(getPanel(), wxID_ANY, L"Toggle", wxPoint(0, 0), wxSize(30, 30));
	button_->Bind(wxEVT_BUTTON, &TestDialog::onButton, this);
	sizer_->Add(button_, 1, wxEXPAND);

	getPanel()->SetSizer(sizer_);

	// add controls to panel
	player_ = new MediaPreviewPlayer(getPanel()); //, wxID_ANY, wxPoint(0, 0), wxSize(100, 100));

	wxBoxSizer *sizer_ = new wxBoxSizer(wxVERTICAL);
	sizer_->Add(player_, 1, wxSHAPED | wxALIGN_CENTER);
	getPanel()->SetSizer(sizer_);

	// start the preview
	player_->setFile(fileset_->getVideo());
	player_->startPreview();
}

void TestDialog::onOk(wxCommandEvent &event)
{
	if (player_ != nullptr) player_->stopPreview();
	DialogEx::onOk(event);
}

void TestDialog::onCancel(wxCommandEvent &event)
{
	if (player_ != nullptr) player_->stopPreview();
	DialogEx::onCancel(event);
}

void TestDialog::onButton(wxCommandEvent &event)
{


	button_->Unbind(wxEVT_BUTTON, &TestDialog::onButton, this);

	if (panel_ == nullptr)
	{
		player_->stopPreview();

		sizer_->Clear();
		getPanel()->DestroyChildren();
		player_ = nullptr;

		panel_ = new wxPanel(getPanel());
		panel_->SetBackgroundColour(Constants::lightBlue);
		sizer_->Add(panel_, 10, wxEXPAND);

		button_ = new wxButton(getPanel(), wxID_ANY, L"Toggle", wxPoint(0, 0), wxSize(30, 30));
		button_->Bind(wxEVT_BUTTON, &TestDialog::onButton, this);
		sizer_->Add(button_, 1, wxEXPAND);
	}
	else if (player_ == nullptr)
	{
		sizer_->Clear();
		getPanel()->DestroyChildren();
		panel_ = nullptr;

		player_ = new MediaPreviewPlayer(getPanel(), wxID_ANY, wxPoint(0, 0), wxSize(100, 100));
		sizer_->Add(player_, 10, wxSHAPED | wxALIGN_CENTER);

		button_ = new wxButton(getPanel(), wxID_ANY, L"Toggle", wxPoint(0, 0), wxSize(30, 30));
		button_->Bind(wxEVT_BUTTON, &TestDialog::onButton, this);
		sizer_->Add(button_, 1, wxEXPAND);

		// start the preview
		player_->setFile(fileset_->getVideo());
		player_->startPreview();
	}

	getPanel()->Layout();

}




