/*
 * VideoUpdaterDialog.h
 *
 *  Created on: 10 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_VOLUMESELECTDIALOG_H_
#define COMMON_VOLUMESELECTDIALOG_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "DialogEx.h"
#include "FileSet.h"

class wxShellExecuteEvent;
class wxDirPickerCtrl;
class wxFileDirPickerEvent;

class VolumeSelectDialog : public DialogEx
{
public:

	static int Run(wxWindow *parent);

	VolumeSelectDialog(wxWindow *parent);
	virtual ~VolumeSelectDialog() = default;

private:

	virtual void onOk(wxCommandEvent &event);
	void updateGui();
	void onDirPicker(wxFileDirPickerEvent &event);

	wxTextCtrl* password_;
	wxTextCtrl *filter_;
	wxDirPickerCtrl *dirPicker_;

};

#endif /* COMMON_VOLUMESELECTDIALOG_H_ */
