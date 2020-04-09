
#include "ImagesBrowser.h"

#include <cmath>
#include <wx/wx.h>

#include "Constants.h"
#include "ImagePanel.h"
#include "Logger.h"
#include "Utilities.h"

ImagesBrowser::ImagesBrowser(wxWindow *parent, ImagesBrowserServer *iServer) :
	wxPanel(parent),
	top_(0),
	iServer_(iServer),
	focus_(false)
{
}

ImagesBrowser::~ImagesBrowser()
{
}

void ImagesBrowser::initialise()
{
	int rows = iServer_->browserGetNoOfRows();
	int cols = iServer_->browserGetNoOfCols();

	wxGridSizer* sizer = new wxGridSizer(rows, cols, 0, 0);
	SetSizer(sizer);

	for (int i = 0; i < rows * cols; ++i)
	{
		ImagePanel* pnlImg = new ImagePanel(this, this, i, 5, true);
		sizer->Add(pnlImg, 1, wxEXPAND, 0);
	}
	top_ = -1;
	Layout();
}

void ImagesBrowser::uninitialise()
{
	stopPreview();

	int n = iServer_->browserGetNoOfRows() * iServer_->browserGetNoOfCols();

	wxWindowList panels = GetChildren();
	for (int i = 0; i < n; ++i)
	{
		ImagePanel *imgPnl = reinterpret_cast<ImagePanel *>(panels[i]);
		imgPnl->uninitialise();
	}
	GetSizer()->Clear();
	SetSizer(nullptr);
	DestroyChildren();
	top_ = -1;
}

void ImagesBrowser::setTop(const int top)
{
	displayAt(top);
}

int ImagesBrowser::getTop()
{
	return top_;
}

void ImagesBrowser::displayAt(int top)
{
	if (top == top_) return;

	// total images
	int n = iServer_->browserGetNoOfRows() * iServer_->browserGetNoOfCols();
	stopPreview();

	// cannot scroll beyond end of data
	if (top + n > iServer_->browserGetNoOfImages())
		top = iServer_->browserGetNoOfImages() - n;

	if (top < 0) top = 0;

	// set the images in each image panel
	wxSizer* sizer = GetSizer();
	sizer->Layout();
	int si = iServer_->browserGetSelected();
	wxWindowList panels = GetChildren();

	for (int i = 0; i < n; ++i)
	{
		ImagePanel *imgPnl = reinterpret_cast<ImagePanel *>(panels[i]);
		if (top + i < iServer_->browserGetNoOfImages())
		{
			imgPnl->setImage(iServer_->browserGetImage(top + i), wxBITMAP_TYPE_JPEG);

			if (si != -1)
			{
				if (top + i == iServer_->browserGetSelected())
				{
					if (focus_)
						imgPnl->setBorderColour(Constants::blue);
					else
						imgPnl->setBorderColour(Constants::grey);

					// start preview
					if (Constants::previewMode)
						imgPnl->startPreview(iServer_->browserGetVideo(top + i));
				}
				else
					imgPnl->setBorderColour(Constants::white);
			}
		}
		else
			imgPnl->setImage("", wxBITMAP_TYPE_JPEG);
	}
	top_ = top;
}

void ImagesBrowser::setSelected(const int selected)
{
	if (selected == -1) return;

	int n = iServer_->browserGetNoOfRows() * iServer_->browserGetNoOfCols();

	stopPreview();

	// if the newly selected item is not currently displayed
	// then display it
	if (selected < top_ || selected >= top_ + n)
	{
		displayAt(selected);
	}
	else
	{
		// change the selection
		wxWindowList panels = GetChildren();

		for (int i = 0; i < n; ++i)
		{
			ImagePanel *imgPnl = reinterpret_cast<ImagePanel *>(panels[i]);

			if (top_ + i == selected)
			{
				if (focus_)
					imgPnl->setBorderColour(Constants::blue);
				else
					imgPnl->setBorderColour(Constants::grey);

				if (Constants::previewMode)
					imgPnl->startPreview(iServer_->browserGetVideo(top_ + i));
			}
			else
				imgPnl->setBorderColour(Constants::white);
		}
	}
}

void ImagesBrowser::stopPreview()
{
	wxWindowList panels = GetChildren();
	int n = iServer_->browserGetNoOfRows() * iServer_->browserGetNoOfCols();
	for (int i = 0; i < n; ++i)
	{
		ImagePanel *imgPnl = reinterpret_cast<ImagePanel *>(panels[i]);

		if (imgPnl->inPreview())
			imgPnl->stopPreview();
	}
}

void ImagesBrowser::pageUp()
{
	displayAt(
		US::pageUp(
			iServer_->browserGetNoOfImages(),
			top_,
			iServer_->browserGetNoOfRows() * iServer_->browserGetNoOfCols())
	);
}

void ImagesBrowser::pageDown()
{
	displayAt(
		US::pageDown(
			iServer_->browserGetNoOfImages(),
			top_,
			iServer_->browserGetNoOfRows() * iServer_->browserGetNoOfCols())
	);
}

void ImagesBrowser::cursorMove(const int step)
{
	int n = iServer_->browserGetNoOfImages();
	// visible items
	int v = iServer_->browserGetNoOfCols() * iServer_->browserGetNoOfRows();
	int s = iServer_->browserGetSelected();

	// sn = new selected item, tn = new top item
	int sn = s + step;
	int tn = top_;

	// if current selected item is not visible then
	// make the new selected the top item
	if (s < top_ || s > top_ + v - 1)
		tn = sn;
	if (sn < 0)
	{
		sn += n;
		tn = n - v;
	}
	else if (sn > n - 1)
	{
		sn -= n;
		tn = 0;
	}
	else if (sn < tn)
		tn = std::max(0, tn - std::abs(step));
	else if (sn > tn + v - 1)
		tn = std::min(n - v, tn + std::abs(step));

	displayAt(tn);
	setSelected(sn);
	iServer_->browserSetSelected(sn);
}

void ImagesBrowser::cursorUp()
{
	cursorMove(-iServer_->browserGetNoOfCols());
}

void ImagesBrowser::cursorDown()
{
	cursorMove(iServer_->browserGetNoOfCols());
}

void ImagesBrowser::cursorLeft()
{
	ImagesBrowser::cursorMove(-1);
}

void ImagesBrowser::cursorRight()
{
	ImagesBrowser::cursorMove(1);
}

//-------------------------------------------------------
// ImagesBrowser
//-------------------------------------------------------

void ImagesBrowser::imageSelected(const int eventId)
{
	setFocus(true);
	setSelected(top_ + eventId);
	iServer_->browserSetSelected(top_ + eventId);
}

wxMenu *ImagesBrowser::getPopupMenu(const int eventId)
{
	return iServer_->browserGetPopupMenu(eventId + top_);
}

bool ImagesBrowser::hasFocus()
{
	return focus_;
}

void ImagesBrowser::setFocus(const bool focus)
{
	// take focus away from grid or logger
	if (focus) SetFocus();

	if (focus != focus_)
	{
		// the selected colour has to change
		focus_ = focus;
		setSelected(iServer_->browserGetSelected());
	}
	else
		focus_ = focus;
}
