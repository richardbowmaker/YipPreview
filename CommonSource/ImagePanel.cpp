/*
 * ImagePanel.cpp
 *
 *  Created on: 5 Feb 2020
 *      Author: richard
 */

#include "ImagePanel.h"

#include <algorithm>

#include "Constants.h"
#include "Logger.h"

ImagePanel::ImagePanel(
	wxWindow* parent,
	ImagePanelEvents *notify /*= nullptr*/,
	const int eventId /*= 0*/,
	const int border /*= 0*/,
	const bool zoomable /*= false*/) : 
	wxPanel		(parent),
	off_		(wxPoint(0, 0)),
	scale_		(0.0f),
	scalei_		(0.0f),
	leftDown_	(false),
	moved_		(false),
	start_		(wxPoint(0, 0)),
	border_		(border),
	notify_		(notify),
	eventId_	(eventId),
	zoomable_	(zoomable)
{
	// create panel within a panel for a border round the image
	panel_ = new wxPanel(this);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(panel_, 1, wxEXPAND | wxALL, border_);
	SetSizer(sizer);

	setBorderColour(Constants::white);
}

ImagePanel::~ImagePanel()
{
}

void ImagePanel::setBorderColour(const wxColour &colour)
{
	SetBackgroundColour(colour);
}

void ImagePanel::setImage(const wxString file, const wxBitmapType format)
{
	panel_->Unbind(wxEVT_RIGHT_UP,		&ImagePanel::rightClickUp,	 this, wxID_ANY);
	panel_->Unbind(wxEVT_LEFT_DOWN,		&ImagePanel::leftClickDown,  this, wxID_ANY);		
	panel_->Unbind(wxEVT_PAINT,			&ImagePanel::onPaint,		 this, wxID_ANY);
	panel_->Unbind(wxEVT_MOTION,		&ImagePanel::mouseMoved,	 this, wxID_ANY);
	panel_->Unbind(wxEVT_LEFT_UP,		&ImagePanel::leftClickUp,	 this, wxID_ANY);
	panel_->Unbind(wxEVT_RIGHT_DOWN,	&ImagePanel::rightClickDown, this, wxID_ANY);
	panel_->Unbind(wxEVT_SIZE,			&ImagePanel::onSize,		 this, wxID_ANY);
	panel_->Unbind(wxEVT_KEY_UP,		&ImagePanel::onKeyUp,		 this, wxID_ANY);

	// clear the previous image 
	memDc_ = nullptr;
	image_ = nullptr;
	wxClientDC dc(panel_);
	dc.Clear();

	// load the new image
	if (file.size() > 0)
	{
		image_ = std::make_shared<wxBitmap>(file, format);
		image_->LoadFile(file, format);

		// render image
		render(dc);

		panel_->Bind(wxEVT_PAINT,     &ImagePanel::onPaint,       this, wxID_ANY);
		panel_->Bind(wxEVT_LEFT_DOWN, &ImagePanel::leftClickDown, this, wxID_ANY);
		panel_->Bind(wxEVT_RIGHT_UP,  &ImagePanel::rightClickUp,  this, wxID_ANY);

		if (zoomable_)
		{
			panel_->Bind(wxEVT_MOTION,		&ImagePanel::mouseMoved,	 this, wxID_ANY);
			panel_->Bind(wxEVT_LEFT_UP,		&ImagePanel::leftClickUp,	 this, wxID_ANY);
			panel_->Bind(wxEVT_RIGHT_DOWN,	&ImagePanel::rightClickDown, this, wxID_ANY);
			panel_->Bind(wxEVT_SIZE,		&ImagePanel::onSize,		 this, wxID_ANY);
			panel_->Bind(wxEVT_KEY_UP,		&ImagePanel::onKeyUp,		 this, wxID_ANY);
		}
	}
}

void ImagePanel::mouseMoved(wxMouseEvent &event) 
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
			wxClientDC dc(panel_);
			render(dc);
			start_ = pt;
		}
	}
}

// void ImagePanel::mouseWheelMoved(wxMouseEvent& event) {}

void ImagePanel::leftClickDown(wxMouseEvent &event)
{
	if (zoomable_)
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
	else if (notify_ != nullptr)
		notify_->selected(eventId_);
}

void ImagePanel::leftClickUp(wxMouseEvent &event) 
{
	if (!moved_) zoomImage(event.GetPosition(), 1.2f);
	leftDown_ = false;
	moved_ = false;
}

void ImagePanel::rightClickDown(wxMouseEvent &event)
{
}

void ImagePanel::rightClickUp(wxMouseEvent &event)
{
	if (zoomable_)
		zoomImage(event.GetPosition(), 1.0f / 1.2f);
	else if (notify_ != nullptr)
		notify_->contextMenu(eventId_);
}

void ImagePanel::mouseLeftWindow(wxMouseEvent &event) 
{
	leftDown_ = false;
	moved_ = true;
}

void ImagePanel::onPaint(wxPaintEvent &evt)
{
	wxPaintDC dc(panel_);
    render(dc);
}

void ImagePanel::onKeyUp(wxKeyEvent& event)
{
	//int n = 0;
}

void ImagePanel::render(wxDC &dc)
{
	// the first calls to render occur before the client area
	// size is set, so ignore these
	wxSize size = dc.GetSize();
	if (size.GetWidth() == 0) return;

	// quit if no image set, nothing to render
	if (image_.get() == nullptr) return;

	// on first render, scale image to fill panel
	if (memDc_.get() == nullptr)
	{
		memDc_ = std::make_shared<wxMemoryDC>(*image_.get());

		float scalex = (float)(dc.GetSize().GetWidth())  / (float)(image_->GetWidth());
		float scaley = (float)(dc.GetSize().GetHeight()) / (float)(image_->GetHeight());

		if (scalex <= scaley)
		{
			scalei_ = scalex;
			off_.y = (dc.GetSize().GetHeight() - (image_->GetHeight() * scalex)) / 2;
		}
		else
		{
			scalei_ = scaley;
			off_.x = (dc.GetSize().GetWidth() - (image_->GetWidth() * scaley)) / 2;
		}
		scale_ = scalei_;
	}

	dc.Clear();
	dc.StretchBlit(
		off_.x,
		off_.y,
		(int)((float) image_->GetWidth() * scale_),
		(int)((float) image_->GetHeight() * scale_),
		memDc_.get(), 
		0, 
		0, 
		image_->GetWidth(),
		image_->GetHeight());
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

	wxClientDC dc(panel_);
	render(dc);
}

wxPoint ImagePanel::screenToImageCoords(const wxPoint &spt)
{
	// convert screen co-ords to image co-ords
	return wxPoint(
		(int)((float)(spt.x - off_.x) / scale_),
		(int)((float)(spt.y - off_.y) / scale_));
}

wxPoint ImagePanel::imageToScreenCoords(const wxPoint &ipt)
{
	return wxPoint(
		(int)(scale_ * (float)ipt.x) + off_.x,
		(int)(scale_ * (float)ipt.y) + off_.y);
}

bool ImagePanel::imageCoordsValid(const wxPoint &ipt)
{
	return ipt.x >= 0 && ipt.x <= image_->GetWidth() && ipt.y >= 0 && ipt.y <= image_->GetHeight();
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
	wxClientDC dc(panel_);
	render(dc);
}


