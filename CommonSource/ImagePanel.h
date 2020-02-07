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

	virtual ~ImagePanel();

    ImagePanel(wxWindow *parent, wxString file, wxBitmapType format);

	void paintEvent(wxPaintEvent & evt);
	void paintNow();

	void render(wxDC& dc);

	// some useful events

	void mouseMoved(wxMouseEvent& event);
//	void mouseWheelMoved(wxMouseEvent& event);
	void leftClickDown(wxMouseEvent& event);
	void leftClickUp(wxMouseEvent& event);
	void rightClickDown(wxMouseEvent& event);
	void rightClickUp(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
//	void keyPressed(wxKeyEvent& event);
//	void keyReleased(wxKeyEvent& event);
	void onSize(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()

private:

	void zoomImage(wxPoint pt, float scalex);
	wxPoint screenToImageCoords(const wxPoint& spt);
	wxPoint imageToScreenCoords(const wxPoint &ipt);
	bool imageCoordsValid(const wxPoint& ipt);

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

};

#endif /* COMMON_IMAGEPANEL_H_ */
