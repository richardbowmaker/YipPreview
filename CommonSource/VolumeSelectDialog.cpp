/*
 * VideoUpdaterDialog.cpp
 *
 *  Created on: 10 Mar 2020
 *      Author: richard
 */

#include "VolumeSelectDialog.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/filepicker.h>

#include <cwchar>
#include <sstream>

#include "Constants.h"
#include "Logger.h"
#include "Main.h"


int VolumeSelectDialog::Run(wxWindow *parent)
{
	VolumeSelectDialog dlg(parent);
	return dlg.ShowModal();
}

VolumeSelectDialog::VolumeSelectDialog(wxWindow *parent) :
	DialogEx(parent, wxID_ANY, wxSize(400, 400), wxOK | wxCANCEL)
{
	setTitle(L"Volume Select");

	dirPicker_ = new wxDirPickerCtrl(getPanel(), wxID_ANY, L"", L"Select volumes folder", 
					wxPoint(10, 10), wxSize(200, 25), wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL | wxDIRP_SMALL);
	filter_    = new wxTextCtrl(getPanel(), wxID_ANY, wxEmptyString, wxPoint(10, 40));
	password_  = new wxTextCtrl  (getPanel(), wxID_ANY, wxEmptyString, wxPoint(10, 70));

	dirPicker_->Bind(wxEVT_DIRPICKER_CHANGED, &VolumeSelectDialog::onDirPicker, this);

	dirPicker_->SetFocus();
}

void VolumeSelectDialog::updateGui()
{
}

void VolumeSelectDialog::onOk(wxCommandEvent& event)
{
	DialogEx::onOk(event);
}

void VolumeSelectDialog::onDirPicker(wxFileDirPickerEvent& event)
{
	Logger::info(L"selected folder %ls", dirPicker_->GetPath().wc_str());
	filter_->SetFocus();
}







