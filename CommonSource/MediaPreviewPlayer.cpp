/*
 * MediaPreviewPlayer.cpp
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#include "MediaPreviewPlayer.h"


#ifdef WINDOWS_BUILD
	#include <stdlib.h>
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

MediaPreviewPlayer::MediaPreviewPlayer(
		wxWindow *parent,
		wxWindowID id        /*= wxID_ANY*/,
		const wxPoint &pos 	 /*= wxDefaultPosition*/,
		const wxSize &size   /*= wxDefaultSize*/,
		long  style 		 /*= wxTAB_TRAVERSAL*/,
		const wxString &name /*= wxPanelNameStr*/) :
	wxPanel(parent, id, pos, size, style, name),
	duration_(0),
	player_(nullptr),
	timer_(nullptr),
	clipIx_(0)
{
	// add media control to this panel
	player_ = new wxMediaCtrl();

#ifdef WINDOWS_BUILD
	// force windows media player
	player_->Create(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxMEDIABACKEND_WMP10);
#elif LINUX_BUILD
	player_->Create(this, wxID_ANY);
#endif
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
	file_ = file;
	return *this;
}

void MediaPreviewPlayer::onMediaPlay(wxMediaEvent& event)
{
	player_->Play();
	player_->SetVolume(0);

	// set the duration
	int d = duration();
	if (d != -1)
		duration_ = d;

	calculateClips();
	doTimer();
	timer_->Start(kTimer);
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
	player_->Play();
	clipIx_ = (clipIx_ + 1) % kNoOfClips;

	// on recycle calculate a new set of random clips
	if (clipIx_ == 0) calculateClips();
}

// calculate a random set of preview start times in seconds
void MediaPreviewPlayer::calculateClips()
{
	clips_.clear();
	clipIx_ = 0;
	srand (time(NULL));
	for (int i = 0; i < kNoOfClips; ++i)
		clips_.emplace_back(rand() % duration_ + 1);
	std::sort(clips_.begin(), clips_.end());
}

// for the linux version we have to use ffmpeg to get the duration
// as the media control does not provide it
int MediaPreviewPlayer::duration()
{
#ifdef WINDOWS_BUILD
	return static_cast<int>(player_->Length() / 1000);
#elif LINUX_BUILD
	// get duration
	ShellExecuteResult res;
	std::wstring cmd = Constants::ffmpeg + std::wstring(L" -i ") + file_ + Constants::ffmpegEnd;
	if (!ShellExecute::shellSync(cmd, res, 10000)) return -1;

	// get duration from ffmpeg output
	std::wstring serr = res.getStderr();
	size_t pos = serr.find(L"Duration:");
	int d = 0;

	if (pos != std::wstring::npos)
	{
		// Duration: 00:02:46.49
		int h = wcstol(serr.substr(pos + 10, 2).c_str(), nullptr, 10);
		int m = wcstol(serr.substr(pos + 13, 2).c_str(), nullptr, 10);
		int s = wcstol(serr.substr(pos + 16, 2).c_str(), nullptr, 10);
		d = 3600 * h + 60 * m + s;
	}
	return d;
#endif
}







