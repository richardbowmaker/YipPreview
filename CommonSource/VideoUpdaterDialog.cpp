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
	chkAdjustVolume_(nullptr),
	chkRemoveAudio_(nullptr),
	chkCompress_(nullptr),
	chkNewImage_(nullptr),
	fileset_(fileset)
{
	setTitle(L"Update Video", fileset_);
	// add controls to panel
	//new wxStaticText(panel, wxID_ANY, L"Volumes dB", wxPoint(10, 10));

	// add check boxes
	chkAdjustVolume_ = new wxCheckBox(getPanel(), wxID_ANY, L"Adjust volume", wxPoint(10, 10));
	chkRemoveAudio_  = new wxCheckBox(getPanel(), wxID_ANY, L"Remove audio",  wxPoint(10, 35));
	chkCompress_     = new wxCheckBox(getPanel(), wxID_ANY, L"Compress",      wxPoint(10, 60));
	chkNewImage_     = new wxCheckBox(getPanel(), wxID_ANY, L"New image",     wxPoint(10, 85));

	Bind(wxEVT_CHECKBOX, [this](wxCommandEvent &)
							{
								getOk()->Enable(
									chkAdjustVolume_->GetValue() ||
									chkRemoveAudio_ ->GetValue() ||
									chkCompress_    ->GetValue() ||
									chkNewImage_    ->GetValue());
							});

	getOk()->Enable(false);

}

void VideoUpdaterDialog::onOk(wxCommandEvent &event)
{
	if (chkNewImage_->GetValue())
	{
		// ffmpeg -y -ss 0:10 -to 0:11 -i <filename> -frames 1 <filename no ext>-%d.jpg
		ShellExecuteResult result;
		std::wstringstream cmd;
#ifdef WINDOWS_BUILD
//		cmd << L"ffmpeg -y -ss 0:10 -to 0:11 -i "
#elif LINUX_BUILD
		cmd << L"/bin/ffmpeg -y -ss 0:10 -to 0:11 -i "
#endif
			<< fileset_->getVideo()
			<< L" -frames 1 "
			<< fileset_->getId() << L"-%d.jpg"
			<< L" &2>1";

		ShellExecute::shellSync(cmd.str(), result, 10000);

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





