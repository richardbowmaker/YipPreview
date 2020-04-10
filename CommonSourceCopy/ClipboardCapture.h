/*
 * ClipboardCapture.h
 *
 *  Created on: 7 Apr 2020
 *      Author: richard
 */

#ifndef COMMON_CLIPBOARDCAPTURE_H_
#define COMMON_CLIPBOARDCAPTURE_H_

#include <memory>
#include <string>

#include "DialogEx.h"

class wxFrame;
class wxTimer;
class wxTimerEvent;
class wxBitmap;

class ClipboardCapture
{
public:
	ClipboardCapture();
	virtual ~ClipboardCapture();

	bool initialise(wxWindow *owner);
	void uninitialise();

private:

	void onTimer(wxTimerEvent &event);

	wxWindow *owner_;
	wxTimer *timer_;
	std::string text_;
	std::shared_ptr<wxBitmap> bitmap_;
};

// popup dialog, asks user if they want to save data
class ClipboardCaptureDialog : public DialogEx
{
public:

	ClipboardCaptureDialog(wxWindow *parent, const std::string text, std::shared_ptr<wxBitmap> bitmap);
	virtual ~ClipboardCaptureDialog() = default;

	static int Run(wxWindow *parent, const std::string text, std::shared_ptr<wxBitmap> bitmap);

private:

	virtual void onOk(wxCommandEvent &event);
	virtual void onCancel(wxCommandEvent &event);

	std::string text_;
	std::shared_ptr<wxBitmap> bitmap_;
};


#endif /* COMMON_CLIPBOARDCAPTURE_H_ */
