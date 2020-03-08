
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
		ImagePanel* pnlImg = new ImagePanel(this, this, i, 5);
		sizer->Add(pnlImg, 1, wxEXPAND, 0);
	}

	top_ = -1;
	displayAt(0);
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

	// cannot scroll beyond end of data
	if (top + n > idata_->getNoOfImages())
		top = idata_->getNoOfImages() - n - 1;

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
				if (focus_)
					imgPnl->setBorderColour(Constants::blue);
				else
					imgPnl->setBorderColour(Constants::grey);
			else
				imgPnl->setBorderColour(Constants::white);
		}
	}
	top_ = top;
}

void ImagesBrowser::setSelected(const int selected)
{
	if (selected == -1) return;

	int n = idata_->getNoOfRows() * idata_->getNoOfCols();

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
				if (focus_)
					imgPnl->setBorderColour(Constants::blue);
				else
					imgPnl->setBorderColour(Constants::grey);
			else
				imgPnl->setBorderColour(Constants::white);
		}
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

void ImagesBrowser::cursorUp()
{

}

void ImagesBrowser::cursorDown()
{
	int n = idata_->getNoOfCols() * idata_->getNoOfRows();
	int s = idata_->getSelected();

	// new selected item is one row down
	int t = s + idata_->getNoOfCols();
	idata_->setSelected(t);

	if (t >= idata_->getNoOfImages())
	{
		setTop(0);
	}
	else if (t >= top_ + n)
	{
		// new selected item is off screen then scroll one row
		setTop(top_ + idata_->getNoOfRows());
	}
	else
	{
		setSelected(t);
	}
}

//-------------------------------------------------------
// ImagesBrowser
//-------------------------------------------------------

void ImagesBrowser::selected(const int eventId)
{
	idata_->setSelected(top_ + eventId);
	setSelected(top_ + eventId);
	setFocus(true);
}

void ImagesBrowser::contextMenu(const int eventId)
{
	setFocus(true);
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
