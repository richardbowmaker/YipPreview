/*
 * ImagePanel.h
 *
 *  Created on: 5 Feb 2020
 *      Author: richard
 */

#ifndef COMMON_IMAGEPANEL_H_
#define COMMON_IMAGEPANEL_H_

#include <memory>
#include <string>
#include <wx/wx.h>
#include <wx/sizer.h>

class MediaPreviewPlayer;

class ImagePanelEvents
{
public:
	ImagePanelEvents() = default;
	~ImagePanelEvents() = default;

	virtual void imageSelected(const int eventId) = 0;
	virtual wxMenu *getPopupMenu(const int eventId) = 0;
};

class ImagePanel : public wxPanel
{
public:

	ImagePanel(
		wxWindow* parent,
		ImagePanelEvents *notify = nullptr,
		const int eventId = 0,
		const int border = 0, 
		const bool zoomable = false);

	virtual ~ImagePanel();

	void uninitialise();
	void setBorderColour(const wxColour &colour);
	void setImage(const std::string file, const wxBitmapType format);
	void startPreview(const std::string file);
	void stopPreview();
	bool inPreview() const;

private:

	void bindEvents();
	void unbindEvents();
	void onPaint(wxPaintEvent &evt);
	void render(wxDC &dc);
	void mouseMoved(wxMouseEvent &event);
	void leftClickDown(wxMouseEvent &event);
	void leftClickUp(wxMouseEvent &event);
	void rightClickDown(wxMouseEvent &event);
	void rightClickUp(wxMouseEvent &event);
	void mouseLeftWindow(wxMouseEvent &event);
	void onSize(wxSizeEvent &event);
	void onKeyUp(wxKeyEvent &event);

	void zoomImage(wxPoint pt, float scalex);
	wxPoint screenToImageCoords(const wxPoint &spt);
	wxPoint imageToScreenCoords(const wxPoint &ipt);
	bool imageCoordsValid(const wxPoint &ipt);

	std::shared_ptr<wxBitmap> image_;
	std::shared_ptr<wxMemoryDC> memDc_;

	// image position and zoom factor as used
	// by wxDC StretchBlit() method
	wxPoint off_;
	float scale_;

	// the initial scaling factor set so that the
	// image fills the panel client area but preserving
	// the aspect ratio
	float scalei_;	

	// mouse move, image dragging support
	bool leftDown_;
	bool moved_;
	wxPoint start_;

	// panel within the image panel to give a border
	wxBoxSizer *sizero_; // sizer for ImagePanel
	wxBoxSizer *sizeri_; // sizer for panel_, only used for media preview player
	wxPanel *paneli_;
	int border_;

	// notification
	ImagePanelEvents *notify_;
	int eventId_;

	// preview player
	MediaPreviewPlayer *preview_;

	bool zoomable_;
};

#endif /* COMMON_IMAGEPANEL_H_ */
