
#include "ImagesBrowser.h"

#include <cmath>
#include <wx/wx.h>

#include "Constants.h"
#include "ImagePanel.h"
#include "Logger.h"

ImagesBrowser::ImagesBrowser(wxWindow *parent, ImagesBrowserData *idata) :
	wxPanel(parent),
	top_(0),
	idata_(idata),
	focus_(false)
{
}

ImagesBrowser::~ImagesBrowser()
{
}

void ImagesBrowser::initialise()
{
	int rows = idata_->getNoOfRows();
	int cols = idata_->getNoOfCols();

	wxGridSizer* sizer = new wxGridSizer(rows, cols, 0, 0);
	SetSizer(sizer);

	for (int i = 0; i < rows * cols; ++i)
	{
		ImagePanel* pnlImg = new ImagePanel(this, this, i, 5, true);
		sizer->Add(pnlImg, 1, wxEXPAND, 0);
	}

	top_ = -1;
	displayAt(0);
}

void ImagesBrowser::uninitialise()
{
	stopPreview();
	int n = idata_->getNoOfRows() * idata_->getNoOfCols();

	wxWindowList panels = GetChildren();
	for (int i = 0; i < n; ++i)
	{
		ImagePanel *imgPnl = reinterpret_cast<ImagePanel *>(panels[i]);
		imgPnl->uninitilaise();
	}
}

void ImagesBrowser::setTop(const int top)
{
	displayAt(top);
}

void ImagesBrowser::displayAt(int top)
{
	if (top == top_) return;

	// total images
	int n = idata_->getNoOfRows() * idata_->getNoOfCols();
	stopPreview();

	// cannot scroll beyond end of data
	if (top + n > idata_->getNoOfImages())
		top = idata_->getNoOfImages() - n;

	if (top < 0) top = 0;

	// set the images in each image panel
	wxSizer* sizer = GetSizer();
	sizer->Layout();
	int si = idata_->getSelected();
	wxWindowList panels = GetChildren();

	for (int i = 0; i < n; ++i)
	{
		ImagePanel *imgPnl = reinterpret_cast<ImagePanel *>(panels[i]);
		imgPnl->setImage(idata_->getImage(top + i), wxBITMAP_TYPE_JPEG);

		if (si != -1)
		{
			if (top + i == idata_->getSelected())
			{
				if (focus_)
					imgPnl->setBorderColour(Constants::blue);
				else
					imgPnl->setBorderColour(Constants::grey);

				// start preview
				imgPnl->startPreview(idata_->getVideo(top + i));
			}
			else
				imgPnl->setBorderColour(Constants::white);
		}
	}
	top_ = top;
	Refresh();
}

void ImagesBrowser::setSelected(const int selected)
{
	if (selected == -1) return;

	int n = idata_->getNoOfRows() * idata_->getNoOfCols();
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

				imgPnl->startPreview(idata_->getVideo(top_ + i));
			}
			else
				imgPnl->setBorderColour(Constants::white);
		}
	}
}

void ImagesBrowser::stopPreview()
{
	wxWindowList panels = GetChildren();
	int n = idata_->getNoOfRows() * idata_->getNoOfCols();
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
		Utilities::pageUp(
			idata_->getNoOfImages(),
			top_,
			idata_->getNoOfRows() * idata_->getNoOfCols())
	);
}

void ImagesBrowser::pageDown()
{
	displayAt(
		Utilities::pageDown(
			idata_->getNoOfImages(),
			top_,
			idata_->getNoOfRows() * idata_->getNoOfCols())
	);
}

void ImagesBrowser::cursorMove(const int step)
{
	int n = idata_->getNoOfImages();
	// visible items
	int v = idata_->getNoOfCols() * idata_->getNoOfRows();
	int s = idata_->getSelected();

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
	idata_->setSelected(sn);
}

void ImagesBrowser::cursorUp()
{
	cursorMove(-idata_->getNoOfCols());
}

void ImagesBrowser::cursorDown()
{
	cursorMove(idata_->getNoOfCols());
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
	idata_->setSelected(top_ + eventId);
}

wxMenu *ImagesBrowser::getPopupMenu(const int eventId)
{
	return idata_->getPopupMenu(eventId + top_);
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
		setSelected(idata_->getSelected());
	}
	else
		focus_ = focus;
}
