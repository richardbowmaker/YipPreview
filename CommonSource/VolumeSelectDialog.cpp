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

#include <algorithm>
#include <cwchar>
#include <memory>
#include <sstream>

#include "_Types.h"
#include "Constants.h"
#include "FileSetManager.h"
#include "Logger.h"
#include "Main.h"
#include "Utilities.h"
#include "VolumeManager.h"


std::string VolumeSelectDialog::password_{"dummypassword"};
std::string VolumeSelectDialog::filter_{"*.hc"};
std::string VolumeSelectDialog::dir_;

int VolumeSelectDialog::Run(wxWindow *parent)
{
	if (dir_.size() == 0)
		dir_ = FU::pathToLocal(R"(/YipPreview/Encrypted)");

	VolumeSelectDialog dlg(parent);
	return dlg.ShowModal();
}

VolumeSelectDialog::VolumeSelectDialog(wxWindow *parent) :
	DialogEx(parent, wxID_ANY, wxSize(820, 700), wxOK | wxCANCEL)
{
	setTitle("Volume Select");

	dirPicker_  = new wxDirPickerCtrl(getPanel(), wxID_ANY, "", "Select volumes folder",
					wxPoint(10, 10), wxSize(600, 25), wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL | wxDIRP_SMALL);
	txtFilter_     = new wxTextCtrl(getPanel(), wxID_ANY, wxEmptyString, wxPoint(10, 45), wxSize(100, 25));
	butFind_    = new wxButton(getPanel(), wxID_ANY, "Find", wxPoint(150, 45));
	txtPassword_   = new wxTextCtrl  (getPanel(), wxID_ANY, wxEmptyString, wxPoint(10, 80), wxSize(300, 25));
	volumeList_ = new wxCheckListBox(getPanel(), wxID_ANY, wxPoint(10, 115), wxSize(800, 500), 0, nullptr, wxLB_SINGLE);

	// load last values used
	dirPicker_->SetPath(dir_);
	txtPassword_->SetValue(password_);
	txtFilter_->SetValue(filter_);

	dirPicker_->Bind(wxEVT_DIRPICKER_CHANGED, &VolumeSelectDialog::onDirPicker, this);
	butFind_->Bind(wxEVT_BUTTON, &VolumeSelectDialog::onFind, this);
	volumeList_->Bind(wxEVT_CHECKLISTBOX, &VolumeSelectDialog::onCheck, this);

	dirPicker_->SetFocus();

	populateListBox();
	updateGui();
}

void VolumeSelectDialog::updateGui()
{
}

void VolumeSelectDialog::populateListBox()
{
	volumeList_->Clear();
	VolumeCollT vols = VolumeManager::getVolumes();

	for (auto vol : vols)
	{
		int n = volumeList_->Append(vol->toString().c_str());
		volumeList_->Check(n, vol->getIsSelected());
	}
}

void VolumeSelectDialog::onOk(wxCommandEvent& event)
{
	std::string pwd{txtPassword_->GetValue()};
	VolumeManager::mountVolumes(pwd);
	FileSetManager::setFileSets(VolumeManager::getFileSets());
	Main::get().populateGui();

	// save values used
	dir_ = dirPicker_->GetPath();
	password_ = txtPassword_->GetValue();
	filter_ = txtFilter_->GetValue();

	DialogEx::onOk(event);
}

void VolumeSelectDialog::onFind(wxCommandEvent &event)
{
	StringCollT files;
	std::string path{dirPicker_->GetPath()};
	std::string filter{txtFilter_->GetValue()};
	if (FU::findMatchingFiles(
			path,
			files,
			filter,
			true))
	{
		for (auto f : files) VolumeManager::add(f, true);
		populateListBox();
	}
	else
		Logger::error("VolumeSelectDialog::onFind() error search {} for matching file {}",
				dirPicker_->GetPath(),
				txtFilter_->GetValue());

	event.Skip();
}

void VolumeSelectDialog::onDirPicker(wxFileDirPickerEvent& event)
{
	Logger::info("selected folder %s", dirPicker_->GetPath().c_str());
	txtFilter_->SetFocus();
	event.Skip();
}

void VolumeSelectDialog::onCheck(wxCommandEvent &event)
{
	int n = event.GetInt();
	VolumeT v = VolumeManager::getVolume(n);
	v->setIsSelected(volumeList_->IsChecked(n));
	event.Skip();
}








