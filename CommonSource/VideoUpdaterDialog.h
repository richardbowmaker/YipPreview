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

class VideoUpdaterDialog: public DialogEx
{
public:

	VideoUpdaterDialog(wxWindow *parent, FileSetT &fileset);

	static int Run(wxWindow *parent, FileSetT &fileset);

private:

	virtual void onOk(wxCommandEvent &event);

	wxCheckBox *chkAdjustVolume_;
	wxCheckBox *chkRemoveAudio_;
	wxCheckBox *chkCompress_;
	wxCheckBox *chkNewImage_;

	FileSetT fileset_;

};

#endif /* COMMON_VIDEOUPDATERDIALOG_H_ */
