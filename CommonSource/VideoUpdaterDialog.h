/*
 * VideoUpdaterDialog.h
 *
 *  Created on: 10 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_VIDEOUPDATERDIALOG_H_
#define COMMON_VIDEOUPDATERDIALOG_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "DialogEx.h"
#include "FileSet.h"

class wxShellExecuteEvent;

class VideoUpdaterDialog : public DialogEx
{
public:

	VideoUpdaterDialog(wxWindow *parent, FileSetT &fileset);

	static int Run(wxWindow *parent, FileSetT &fileset);

private:

	virtual void onOk(wxCommandEvent &event);
	void updateGui();
	void setDuration();
	void onShellExecute(wxShellExecuteEvent& event);

	wxStaticText *txtDuration_;
	wxCheckBox *chkAdjustVolume_;
	wxCheckBox *chkRemoveAudio_;
	wxCheckBox *chkCompress_;
	wxCheckBox *chkNewImage_;

	FileSetT fileset_;
	int duration_;

};

#endif /* COMMON_VIDEOUPDATERDIALOG_H_ */
