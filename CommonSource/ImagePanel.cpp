/*
 * ImagePanel.cpp
 *
 *  Created on: 5 Feb 2020
 *      Author: richard
 */

#include "ImagePanel.h"

#include <algorithm>

#include "Logger.h"

ImagePanel::ImagePanel(wxWindow* parent, wxString file, wxBitmapType format) :
	wxPanel(parent),
	off_(wxPoint(0,0)),
	scale_(0.0f),
	scalei_(0.0f),
	leftDown_(false),
	moved_(false),
	start_(wxPoint(0,0))
{
	// load the file... ideally add a check to see if loading was successful
	image_.LoadFile(file, format);
}

ImagePanel::~ImagePanel()
{
	// TODO Auto-generated destructor stub
}

BEGIN_EVENT_TABLE(ImagePanel, wxPanel)
// some useful events

    EVT_MOTION(ImagePanel::mouseMoved)
	EVT_LEFT_DOWN(ImagePanel::leftClickDown)
	EVT_LEFT_UP(ImagePanel::leftClickUp)
	EVT_RIGHT_DOWN(ImagePanel::rightClickDown)
	EVT_RIGHT_UP(ImagePanel::rightClickUp)
	EVT_LEAVE_WINDOW(ImagePanel::mouseLeftWindow)
// EVT_KEY_DOWN(ImagePanel::keyPressed)
// EVT_KEY_UP(ImagePanel::keyReleased)
// EVT_MOUSEWHEEL(ImagePanel::mouseWheelMoved)
   EVT_SIZE(ImagePanel::onSize)


// catch paint events
EVT_PAINT(ImagePanel::paintEvent)

END_EVENT_TABLE()


// some useful events

void ImagePanel::mouseMoved(wxMouseEvent& event) 
{
	if (leftDown_)
	{
		wxPoint pt = event.GetPosition();
		int xd = pt.x - start_.x;
		int yd = pt.y - start_.y;

		// sometimes left click involves a small amount
		// of movement, in which case a zoom not a move
		// should be performed
		if (std::abs(xd) + std::abs(yd) > 4)
		{
			moved_ = true;
			off_.x += xd;
			off_.y += yd;
			wxClientDC dc(this);
			render(dc);
			start_ = pt;
		}
	}
}

// void ImagePanel::mouseWheelMoved(wxMouseEvent& event) {}

void ImagePanel::leftClickDown(wxMouseEvent& event)
{
	// user can drag image, starting point mouse down
	// must be wthin the image

	wxPoint spt = event.GetPosition();
	wxPoint ipt = screenToImageCoords(spt);

	if (imageCoordsValid(ipt))
	{
		leftDown_ = true;
		moved_ = false;
		start_ = spt;
	}
}

void ImagePanel::leftClickUp(wxMouseEvent& event) 
{
	if (!moved_) zoomImage(event.GetPosition(), 1.2f);
	leftDown_ = false;
	moved_ = false;
}

void ImagePanel::rightClickDown(wxMouseEvent& event)
{
}

void ImagePanel::rightClickUp(wxMouseEvent& event)
{
	zoomImage(event.GetPosition(), 1.0f / 1.2f);
}

void ImagePanel::mouseLeftWindow(wxMouseEvent& event) 
{
	leftDown_ = false;
	moved_ = true;
}

// void ImagePanel::keyPressed(wxKeyEvent& event) {}
// void ImagePanel::keyReleased(wxKeyEvent& event) {}


/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

void ImagePanel::paintEvent(wxPaintEvent & evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void ImagePanel::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ImagePanel::render(wxDC&  dc)
{
	// the first calls to render occur before the client area
	// size is set, so ignore these
	wxSize size = dc.GetSize();
	if (size.GetWidth() == 0) return;

	// on first render, scale image to fill panel
	if (memDc_.get() == nullptr)
	{
		memDc_ = std::make_unique<wxMemoryDC>(image_);

		scalei_ = std::min(
			(float)dc.GetSize().GetWidth()  / image_.GetWidth(),
			(float)dc.GetSize().GetHeight() / image_.GetHeight());

		scale_ = scalei_;
	}

	dc.Clear();
	dc.StretchBlit(
		off_.x,
		off_.y,
		(int)((float) image_.GetWidth() * scale_),
		(int)((float) image_.GetHeight() * scale_),
		memDc_.get(), 
		0, 
		0, 
		image_.GetWidth(),
		image_.GetHeight());
}

void ImagePanel::onSize(wxSizeEvent &event)
{
//	// don't handle size until image has been displayed
//	if (size_.GetWidth() == 0 || size_.GetHeight() == 0) return;
//
//	wxSize size = event.GetSize();
//
//	// get x and y scaling
//	float xs = (float)size.GetWidth() / (float)size_.GetWidth();
//	float ys = (float)size.GetHeight() / (float)size_.GetHeight();
//
//	int xoff = (int)((float)xoff_ * xs);
//	int yoff = (int)((float)yoff_ * ys);
//
//	wxPoint spt = imageToScreenCoords(wxPoint(imgW_, imgH_));
//	int wsy = size_.GetHeight() - spt.y;
//	if (wsy < 0) wsy = 0;
//
//
//	float sx = (float)(size.GetWidth() - xoff) / (float)(size_.GetWidth() - xoff_);
//	float sy = (float)(size.GetHeight() - yoff) / (float)(size_.GetHeight() - yoff_);
//	float s = 1.0f;
//
//	if (sx <= 1.0f && sy <= 1.0f) s = std::min(sx, sy);
//	else if (sx >= 1.0f && sy >= 1.0f) s = std::max(sx, sy);
//	else if (sx <= 1.0f) s = sx;
//	else s = sy;
//
//
//
//
//	xoff_ = xoff;
//	yoff_ = yoff;
//	scale_ *= s;
//	scalei_*= s;
//	size_ = size;
//
//	//Logger::info(L"old size (%d, %d), new size (%d, %d), xs %d, ys %d",
//	//	size_.GetWidth(), size_.GetHeight(),
//	//	size.GetWidth(), size.GetHeight(),
//	//	xs, ys
//	//	);

	wxClientDC dc(this);
	render(dc);
}

wxPoint ImagePanel::screenToImageCoords(const wxPoint& spt)
{
	// convert screen co-ords to image co-ords
	return wxPoint(
		(int)((float)(spt.x - off_.x) / scale_),
		(int)((float)(spt.y - off_.y) / scale_));
}

wxPoint ImagePanel::imageToScreenCoords(const wxPoint& ipt)
{
	return wxPoint(
		(int)(scale_ * (float)ipt.x) + off_.x,
		(int)(scale_ * (float)ipt.y) + off_.y);
}

bool ImagePanel::imageCoordsValid(const wxPoint& ipt)
{
	return ipt.x >= 0 && ipt.x <= image_.GetWidth() && ipt.y >= 0 && ipt.y <= image_.GetHeight();
}

void ImagePanel::zoomImage(wxPoint pt, float scalex)
{
	// when user left clicks on the image the image
	// is zoomed in by 20% and the point clicked remains
	// at the same screen position
	// right clicking zooms out by 20%

	// get image co-ords, quit if outside image area
	wxPoint ipt = screenToImageCoords(pt);
	if (!imageCoordsValid(ipt)) return;

	// zoom in (scalex > 1), or out (scalex < 1)
	float scale = scale_ * scalex;

	// limit scaling to range scalei .. scalei * 10
	if (scale - scalei_ < -scalei_ * 0.05) return;
	if (scale > 10 * scalei_) return;

	// calculate new offsets
	off_.x += (int)((float)ipt.x * (scale_ - scale));
	off_.y += (int)((float)ipt.y * (scale_ - scale));

	scale_ = scale;
	wxClientDC dc(this);
	render(dc);
}


