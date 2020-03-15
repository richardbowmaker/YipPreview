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
#include "ShellExecute.h"

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
	setTitle(L"Update Video", fileset_);
	// add controls to panel
	txtDuration_ = new wxStaticText(getPanel(), wxID_ANY, L"Duration: ", wxPoint(10, 10), wxDefaultSize, wxALIGN_LEFT);

	// add check boxes
	chkAdjustVolume_ = new wxCheckBox(getPanel(), 1, L"Adjust volume", wxPoint(10, 35));
	chkRemoveAudio_  = new wxCheckBox(getPanel(), 1, L"Remove audio",  wxPoint(10, 60));
	chkCompress_     = new wxCheckBox(getPanel(), 1, L"Compress",      wxPoint(10, 85));
	chkNewImage_     = new wxCheckBox(getPanel(), 1, L"New image",     wxPoint(10, 110));

	Bind(wxEVT_CHECKBOX, [this](wxCommandEvent &) { updateGui(); });
	updateGui();
	setDuration();
}

void VideoUpdaterDialog::setDuration()
{
	// ffmpeg -i <filename>
	ShellExecuteResult result;
	std::wstringstream cmd;

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
	std::wstring str = result.getStderr();

	size_t p = str.find(L"Duration");

	if (p != std::wstring::npos)
	{
		// display duration on gui
		std::wstring t = std::wstring(txtDuration_->GetLabelText().wc_str()) +
				str.substr(p + 10, 11);
		txtDuration_->SetLabelText(t.c_str());
		Layout();

		// parse and save duration in seconds
		wchar_t* q;
		long h = wcstol((str.substr(p + 10, 2)).c_str(), &q, 10);
		long m = wcstol((str.substr(p + 13, 2)).c_str(), &q, 10);
		long s = wcstol((str.substr(p + 16, 2)).c_str(), &q, 10);
		duration_ = static_cast<int>(h * 60 * 60 + m * 60 + s);
		updateGui();
	}
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
		std::wstringstream cmd;

		// random point in video to grab an image
		int r = Utilities::getRand(1, duration_);

		// ffmpeg command
		wchar_t buf[500];
		swprintf(buf, sizeof(buf) / sizeof(wchar_t),
			L"%ls -y -ss %d:%02d -to %d:%02d -i %ls -frames 1 %ls-%%d.jpg %ls",
			Constants::ffmpeg.c_str(),
			r / 60, r % 60,
			(r + 1) / 60, (r + 1) % 60,
			fileset_->getVideo().c_str(),
			fileset_->getId().c_str(),
			Constants::ffmpegEnd.c_str());

		ShellExecute::shellSync(buf, result, 10000);

		if (result.getSuccess())
		{
			std::wstring imf = fileset_->getId() + std::wstring(L".jpg");
			if (FU::moveFile(
					fileset_->getId() + std::wstring(L"-1.jpg"),
					imf))
			{
				fileset_->set(imf);
			}
		}
		else
			Logger::error(L"Failed to create new image for %ls", fileset_->getId().c_str());
	}

	DialogEx::onOk(event);
}





