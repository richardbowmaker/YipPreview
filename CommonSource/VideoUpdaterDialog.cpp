/*
 * VideoUpdaterDialog.cpp
 *
 *  Created on: 10 Mar 2020
 *      Author: richard
 */

#include "VideoUpdaterDialog.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <cwchar>
#include <sstream>

#include "Constants.h"
#include "Logger.h"
#include "Main.h"
#include "ShellExecute.h"
#include "Utilities.h"

int VideoUpdaterDialog::Run(wxWindow *parent, FileSetT &fileset)
{
	VideoUpdaterDialog dlg(parent, fileset);
	return dlg.ShowModal();
}

VideoUpdaterDialog::VideoUpdaterDialog(wxWindow *parent, FileSetT &fileset) :
	DialogEx(parent, wxID_ANY, wxSize(400, 400), wxOK | wxCANCEL),
	txtDuration_(nullptr),
	chkAdjustVolume_(nullptr),
	chkRemoveAudio_(nullptr),
	chkCompress_(nullptr),
	chkNewImage_(nullptr),
	fileset_(fileset),
	duration_(0)
{
	setTitle("Update Video", fileset_);
	// add controls to panel
	txtDuration_ = new wxStaticText(getPanel(), wxID_ANY, "Duration: ", wxPoint(10, 10), wxDefaultSize, wxALIGN_LEFT);

	// add check boxes
	chkAdjustVolume_ = new wxCheckBox(getPanel(), wxID_ANY, "Adjust volume", wxPoint(10, 35));
	chkRemoveAudio_  = new wxCheckBox(getPanel(), wxID_ANY, "Remove audio",  wxPoint(10, 60));
	chkCompress_     = new wxCheckBox(getPanel(), wxID_ANY, "Compress",      wxPoint(10, 85));
	chkNewImage_     = new wxCheckBox(getPanel(), wxID_ANY, "New image",     wxPoint(10, 110));

	Bind(wxEVT_CHECKBOX, [this](wxCommandEvent &) { updateGui(); });

	// get duration from file set, if it does not have one
	// then calculate it
	duration_ = fileset_->getDurationMs() / 1000;
	if (duration_ > 0)
		txtDuration_->SetLabelText(
				std::string(txtDuration_->GetLabelText().c_str()) +
			fileset_->getDurationStr());	
	else
		setDuration();

	updateGui();
}

void VideoUpdaterDialog::setDuration()
{
	// ffmpeg -i <filename>
	ShellExecuteResult result;
	std::stringstream cmd;

	cmd << Constants::ffmpeg
		<< " -y -ss 0:10 -to 0:11 -i "
		<< fileset_->getVideo()
		<< Constants::ffmpegEnd;

	// set duration
	Bind(wxEVT_SHELL_EXECUTE_RESULT, &VideoUpdaterDialog::onShellExecute, this);
	ShellExecute::shellAsyncGui(cmd.str(), GetEventHandler());
}

void VideoUpdaterDialog::onShellExecute(wxShellExecuteEvent& event)
{
	Unbind(wxEVT_SHELL_EXECUTE_RESULT, &VideoUpdaterDialog::onShellExecute, this);
	ShellExecuteResult result = event.getResult();
	std::string str = result.getStderr();

	size_t p = str.find("Duration");

	if (p != std::string::npos)
	{
		fileset_->setDurationStr(str.substr(p + 10, 12));
		duration_ = fileset_->getDurationMs() / 1000;
		txtDuration_->SetLabelText(
				std::string(txtDuration_->GetLabelText().c_str()) +
				fileset_->getDurationStr());
		Layout();
		updateGui();
		Main::get().refresh(*fileset_);
	}
	event.Skip();
}

void VideoUpdaterDialog::updateGui()
{
	getOk()->Enable(
		chkAdjustVolume_->GetValue() ||
		chkRemoveAudio_ ->GetValue() ||
		chkCompress_    ->GetValue() ||
		chkNewImage_    ->GetValue());

	chkNewImage_->Enable(duration_ > 0);
}

void VideoUpdaterDialog::onOk(wxCommandEvent &event)
{
	if (chkNewImage_->GetValue())
	{
		// ffmpeg -y -ss 0:10 -to 0:11 -i <filename> -frames 1 <filename no ext>-%d.jpg &2>1
		ShellExecuteResult result;

		// random point in video to grab an image
		int r = US::getRand(1, duration_);

		// ffmpeg command
		char buf[500];
		snprintf(buf, sizeof(buf) / sizeof(wchar_t),
			"%s -y -ss %d:%02d -to %d:%02d -i %s -frames 1 %s-%%d.jpg %s",
			Constants::ffmpeg.c_str(),
			r / 60, r % 60,
			(r + 1) / 60, (r + 1) % 60,
			fileset_->getVideo().c_str(),
			fileset_->getId().c_str(),
			Constants::ffmpegEnd.c_str());

		ShellExecute::shellSync(buf, result, 10000);

		if (result.getSuccess())
		{
			std::string imf = fileset_->getId() + std::string(".jpg");
			if (FU::moveFile(
					fileset_->getId() + std::string("-1.jpg"),
					imf))
			{
				fileset_->set(imf);
			}
		}
		else
			Logger::error("Failed to create new image for {}", fileset_->getId());
	}

	DialogEx::onOk(event);
}





