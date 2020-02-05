/*
 * ImagePanel.h
 *
 *  Created on: 5 Feb 2020
 *      Author: richard
 */

#ifndef COMMON_IMAGEPANEL_H_
#define COMMON_IMAGEPANEL_H_

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

//	void mouseMoved(wxMouseEvent& event);
//	void mouseDown(wxMouseEvent& event);
//	void mouseWheelMoved(wxMouseEvent& event);
//	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
//	void mouseLeftWindow(wxMouseEvent& event);
//	void keyPressed(wxKeyEvent& event);
//	void keyReleased(wxKeyEvent& event);
	void onSize(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()

private:

	 wxBitmap image;

};

#endif /* COMMON_IMAGEPANEL_H_ */
