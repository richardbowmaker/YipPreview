/*
 * ImagePanel.h
 *
 *  Created on: 5 Feb 2020
 *      Author: richard
 */

#ifndef COMMON_IMAGEPANEL_H_
#define COMMON_IMAGEPANEL_H_

#include <memory>
#include <wx/wx.h>
#include <wx/sizer.h>

class ImagePanel : public wxPanel
{
public:

    ImagePanel(wxWindow *parent, const int border = 0);
	virtual ~ImagePanel();

	void setBorderColour(const wxColour &colour);
	void setImage(const wxString file, const wxBitmapType format);

private:

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

	 wxBitmap image_;
	 std::unique_ptr<wxMemoryDC> memDc_;

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
	 wxPanel* panel_;
	 int border_;

};

#endif /* COMMON_IMAGEPANEL_H_ */
