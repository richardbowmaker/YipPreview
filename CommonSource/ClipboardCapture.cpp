/*
 * ClipboardCapture.cpp
 *
 *  Created on: 7 Apr 2020
 *      Author: richard
 */

#include "ClipboardCapture.h"

#include <wx/clipbrd.h>
#include <wx/wx.h>

#include "Constants.h"
#include "FileSetManager.h"
#include "Logger.h"
#include "ImagePanel.h"
#include "Main.h"
#include "VolumeManager.h"
#include "Utilities.h"

ClipboardCapture::ClipboardCapture() :
	owner_	{nullptr},
	timer_	{nullptr}
{
}

ClipboardCapture::~ClipboardCapture()
{
}

bool ClipboardCapture::initialise(wxWindow *owner)
{
	if (owner == nullptr)
	{
		Logger::error("ClipboardCapture::initialise() no owner");
		return false;
	}
	owner_ = owner;

	if (timer_ != nullptr)
		Logger::warning("ClipboardCapture::initialise() already initialised");

	timer_ = new wxTimer(owner_);
	owner_->Bind(wxEVT_TIMER, &ClipboardCapture::onTimer, this, wxID_ANY);
	timer_->Start(1000);
}

void ClipboardCapture::uninitialise()
{
	if (timer_ != nullptr)
	{
		timer_->Stop();
		owner_->Unbind(wxEVT_TIMER, &ClipboardCapture::onTimer, this, wxID_ANY);
		timer_ = nullptr;
		bitmap_.reset();
	}
}

void ClipboardCapture::onTimer(wxTimerEvent &event)
{
	if (wxTheClipboard->Open())
	{
        if (wxTheClipboard->IsSupported(wxDF_TEXT))
	    {
            wxTextDataObject data;
	        wxTheClipboard->GetData(data);
		    std::string text = std::string(data.GetText());
		    if (text.size() > 0 && text_.compare(text) != 0)
		    {
		    	text_ = text;
		    	// clear the clipboard
	        	wxTheClipboard->SetData( new wxTextDataObject("") );
	        	Logger::info("Clipboard text = {}", text_);
		    }
	    }
	    if (wxTheClipboard->IsSupported(wxDF_BITMAP))
	    {
	    	if (bitmap_ == nullptr)
	    	{
				wxBitmapDataObject data;
				wxTheClipboard->GetData(data);
				bitmap_ = std::make_shared<wxBitmap>(data.GetBitmap());
				wxTheClipboard->SetData(new wxTextDataObject("_"));
				Logger::info("Clipboard bitmap, size = {}", data.GetDataSize());

				// now we have the link and the image, clear the clipboard
				wxTheClipboard->SetData(new wxTextDataObject(""));

				ClipboardCaptureDialog::Run(owner_, text_, bitmap_);

				bitmap_.reset();
	    	}
	    }
	    wxTheClipboard->Close();
	}
	event.Skip();
}

//---------------------------------------------------------------------------
// clipboard popup dialog
//---------------------------------------------------------------------------

int ClipboardCaptureDialog::Run(wxWindow *parent, const std::string text, std::shared_ptr<wxBitmap> bitmap)
{
	ClipboardCaptureDialog dlg(parent, text, bitmap);
	return dlg.ShowModal();
}

ClipboardCaptureDialog::ClipboardCaptureDialog(wxWindow *parent, const std::string text, std::shared_ptr<wxBitmap> bitmap) :
	DialogEx(parent, wxID_ANY, wxSize(1000, 1000), wxOK | wxCANCEL),
	text_	{text},
	bitmap_ {bitmap}

{
	setTitle("Test dialog");

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	getPanel()->SetSizer(sizer);

	ImagePanel *image = new ImagePanel(getPanel());
    sizer->Add(image, 1, wxEXPAND);
    image->setImage(bitmap);
}

void ClipboardCaptureDialog::onOk(wxCommandEvent &event)
{
	// 10k for an image approx
	VolumeT vol = VolumeManager::findVolumeWithFreeSpace(10 * 1024);

	if (vol.get() != nullptr)
	{
		// save bit map to file
		std::string id = FileSetManager::getNextId();
		std::string image = vol->getFilesDirectory() + Constants::pathSeparator + id + std::string(".jpg");
		if (bitmap_->SaveFile(image, wxBITMAP_TYPE_JPEG))
		{
			// create new file set and save link
			FileSetT fs = std::make_shared<FileSet>(vol.get(), image);
			fs->setLinkText(text_);

			// add to data and refresh gui
			vol->addFileSet(fs);
			FileSetManager::addFileSet(fs);
			Main::get().addFileSet(fs);
		}
		else
			Logger::error("ClipboardCaptureDialog::onOk() save bit map to {} failed", SU::singleQuotes(image));

	}
	else
		Logger::error("Insufficient disk space to store captured image");

	DialogEx::onOk(event);
}

void ClipboardCaptureDialog::onCancel(wxCommandEvent &event)
{

	DialogEx::onCancel(event);
}








