/*
 * TestDialog.h
 *
 *  Created on: 15 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_TESTDIALOG_H_
#define COMMON_TESTDIALOG_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "DialogEx.h"
#include "FileSet.h"


class MediaPreviewPlayer;

class TestDialog : public DialogEx
{
public:

	TestDialog(wxWindow *parent, FileSetT &fileset);
	virtual ~TestDialog() = default;

	static int Run(wxWindow *parent, FileSetT &fileset);


//	virtual ~TestDialog();

private:

	virtual void onOk(wxCommandEvent &event);
	virtual void onCancel(wxCommandEvent &event);

	virtual void onButton (wxCommandEvent &event);


	FileSetT fileset_;
	MediaPreviewPlayer *player_;
	wxPanel *panel_;
	wxBoxSizer *sizer_;
	wxButton *button_;

};

#endif /* COMMON_TESTDIALOG_H_ */
