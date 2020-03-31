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
class wxCheckListBox;

class VolumeSelectDialog : public DialogEx
{
public:

	static int Run(wxWindow *parent);

	VolumeSelectDialog(wxWindow *parent);
	virtual ~VolumeSelectDialog() = default;

private:

	virtual void onOk(wxCommandEvent &event);
	virtual void onFind(wxCommandEvent &event);
	virtual void onCheck(wxCommandEvent &event);
	void updateGui();
	void onDirPicker(wxFileDirPickerEvent &event);
	void populateListBox();

	wxTextCtrl* txtPassword_;
	wxTextCtrl *txtFilter_;
	wxDirPickerCtrl *dirPicker_;
	wxCheckListBox *volumeList_;
	wxButton *butFind_;

	static std::wstring password_;
	static std::wstring filter_;
	static std::wstring dir_;

};

#endif /* COMMON_VOLUMESELECTDIALOG_H_ */
