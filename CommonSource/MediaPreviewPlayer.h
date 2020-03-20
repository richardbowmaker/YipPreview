/*
 * MediaPreviewPlayer.h
 *
 *  Created on: 11 Feb 2020
 *      Author: richard
 */

#ifndef MEDIAPREVIEWPLAYER_H_
#define MEDIAPREVIEWPLAYER_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <string>
#include <vector>
#include <wx/mediactrl.h>

class MediaPreviewPlayer : public wxPanel
{
public:

	MediaPreviewPlayer(
			wxWindow *parent,
			wxWindowID id        = wxID_ANY,
			const wxPoint &pos 	 = wxDefaultPosition,
			const wxSize &size   = wxDefaultSize,
			long  style 		 = wxTAB_TRAVERSAL,
			const wxString &name = wxPanelNameStr);
	virtual ~MediaPreviewPlayer();

	MediaPreviewPlayer &setFile(const std::wstring &file);
	void startPreview();
	void stopPreview();

private:

	void onMediaPlay(wxMediaEvent &event);
	void onTimer(wxTimerEvent &event);
	void doTimer();
	void calculateClips();
	long duration();

	// the video file
	std::wstring file_;
	int duration_;	// seconds

	// controls
	wxMediaCtrl *player_;
	wxTimer *timer_;

	// clips, list of preview starting points in seconds
	std::vector<int> clips_;
	// next clip to be played
	int clipIx_;

};

#endif /* MEDIAPREVIEWPLAYER_H_ */
