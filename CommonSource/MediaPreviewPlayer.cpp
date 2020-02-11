/*
 * MediaPreviewPlayer.cpp
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#include "MediaPreviewPlayer.h"


#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
	#include <algorithm>
	#include <cstdlib>
	#include <stdlib.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

#include "Constants.h"
#include "Logger.h"
#include "ShellExecute.h"
#include "Utilities.h"

constexpr int kNoOfClips = 10;
constexpr int kTimer = 2000; // ms, time each clip is played for

MediaPreviewPlayer::MediaPreviewPlayer(wxWindow *parent, wxWindowID id /*= wxID_ANY*/) : wxPanel(parent, id)
{
	// add media control to this panel
	player_ = new wxMediaCtrl();
	player_->Create(this, wxID_ANY);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(player_, 1, wxEXPAND, 0);
	SetSizer(sizer);
	Bind(wxEVT_MEDIA_LOADED, &MediaPreviewPlayer::onMediaPlay, this, wxID_ANY);

	// timer for forwarding to next clip
	timer_ = new wxTimer(GetEventHandler());
	Bind(wxEVT_TIMER, &MediaPreviewPlayer::onTimer, this, wxID_ANY);
}

MediaPreviewPlayer::~MediaPreviewPlayer()
{
	player_->Stop();
}

MediaPreviewPlayer &MediaPreviewPlayer::setFile(const std::wstring &file)
{
	// get duration
	ShellExecuteResult res;
	std::wstring cmd = Constants::ffmpeg + std::wstring(L" -i ") + file + std::wstring(L" &2>1");
	if (!ShellExecute::shellSync(cmd, res, 2000)) return *this;

	// get duration from ffmpeg output
	std::wstring stderr = res.getStderr();
	size_t pos = stderr.find(L"Duration:");

	if (pos != std::wstring::npos)
	{
		// Duration: 00:02:46.49
		int h = wcstol(stderr.substr(pos + 10, 2).c_str(), nullptr, 10);
		int m = wcstol(stderr.substr(pos + 13, 2).c_str(), nullptr, 10);
		int s = wcstol(stderr.substr(pos + 16, 2).c_str(), nullptr, 10);
		duration_ = 3600 * h + 60 * m + s;
		file_ = file;
	}
	return *this;
}

void MediaPreviewPlayer::onMediaPlay(wxMediaEvent& event)
{
	player_->Play();
	player_->SetVolume(0);
	calculateClips();
	doTimer();
	timer_->Start(kTimer);
}

void MediaPreviewPlayer::play()
{
	if (file_.length() > 0)
		player_->Load(file_.c_str());
	else
		Logger::error(L"MediaPreviewPlayer play file before file has been set");
}

void MediaPreviewPlayer::startPreview()
{
	if (file_.length() > 0)
	{
		player_->Load(file_.c_str());
	}
	else
		Logger::error(L"MediaPreviewPlayer play file before file has been set");
}

void MediaPreviewPlayer::stopPreview()
{
	timer_->Stop();
	player_->Stop();
}

void MediaPreviewPlayer::onTimer(wxTimerEvent &event)
{
	doTimer();
}

void MediaPreviewPlayer::doTimer()
{
	player_->Seek(clips_[clipIx_] * 1000);
	clipIx_ = ++clipIx_ % kNoOfClips;

	// on recycle calculate a new set of random clips
	if (clipIx_ == 0) calculateClips();
}

// calculate a random set of preview start times in seconds
void MediaPreviewPlayer::calculateClips()
{
	clips_.clear();
	clipIx_ = 0;
#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
	srand (time(NULL));
	for (int i = 0; i < kNoOfClips; ++i)
		clips_.emplace_back(rand() % duration_ + 1);
#endif
	std::sort(clips_.begin(), clips_.end());
}






