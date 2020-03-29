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


int VolumeSelectDialog::Run(wxWindow *parent)
{
	VolumeSelectDialog dlg(parent);
	return dlg.ShowModal();
}

VolumeSelectDialog::VolumeSelectDialog(wxWindow *parent) :
	DialogEx(parent, wxID_ANY, wxSize(820, 700), wxOK | wxCANCEL)
{
	setTitle(L"Volume Select");

	dirPicker_  = new wxDirPickerCtrl(getPanel(), wxID_ANY, L"", L"Select volumes folder",
					wxPoint(10, 10), wxSize(600, 25), wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL | wxDIRP_SMALL);
	filter_     = new wxTextCtrl(getPanel(), wxID_ANY, wxEmptyString, wxPoint(10, 45), wxSize(100, 25));
	butFind_    = new wxButton(getPanel(), wxID_ANY, L"Find", wxPoint(150, 45));
	password_   = new wxTextCtrl  (getPanel(), wxID_ANY, wxEmptyString, wxPoint(10, 80), wxSize(300, 25));
	volumeList_ = new wxCheckListBox(getPanel(), wxID_ANY, wxPoint(10, 115), wxSize(800, 500), 0, nullptr, wxLB_SINGLE);

	dirPicker_->Bind(wxEVT_DIRPICKER_CHANGED, &VolumeSelectDialog::onDirPicker, this);
	butFind_->Bind(wxEVT_BUTTON, &VolumeSelectDialog::onFind, this);
	volumeList_->Bind(wxEVT_CHECKLISTBOX, &VolumeSelectDialog::onCheck, this);

	password_->SetValue(L"dummypassword");
	filter_->SetValue(L"*.hc");
	dirPicker_->SetPath(FU::pathToLocal(LR"(/YipPreview/Encrypted)"));
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
	VolumeManager::mountVolumes(password_->GetValue().wc_str());
	VolumeCollT vols = VolumeManager::getVolumes();
	std::for_each(vols.begin(), vols.end(), FileSetManager::addFiles);
	Main::get().populateGui();
	DialogEx::onOk(event);
}

void VolumeSelectDialog::onFind(wxCommandEvent &event)
{
	StringCollT files;
	if (FU::findMatchingFiles(
			dirPicker_->GetPath().wc_str(),
			files,
			filter_->GetValue().wc_str(),
			true))
	{
		for (auto f : files) VolumeManager::add(f, true);
		populateListBox();
	}
	else
		Logger::error(L"VolumeSelectDialog::onFind() error search %ls for matching file %ls",
				dirPicker_->GetPath().wc_str(),
				filter_->GetValue().wc_str());
}

void VolumeSelectDialog::onDirPicker(wxFileDirPickerEvent& event)
{
	Logger::info(L"selected folder %ls", dirPicker_->GetPath().wc_str());
	filter_->SetFocus();
}

void VolumeSelectDialog::onCheck(wxCommandEvent &event)
{
	int n = event.GetInt();
	VolumeT v = VolumeManager::getVolume(n);
	v->setIsSelected(volumeList_->IsChecked(n));
}








