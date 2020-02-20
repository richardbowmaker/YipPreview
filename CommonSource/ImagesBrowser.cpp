
#include "ImagesBrowser.h"

#include <cmath>
#include <wx/wx.h>

#include "Constants.h"
#include "ImagePanel.h"
#include "Logger.h"

ImagesBrowser::ImagesBrowser(wxWindow *parent, ImagesBrowserData *idata) :
	wxPanel(parent),
	top_(0),
	idata_(idata)
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
		//		pnlImg->Bind(wxEVT_SET_FOCUS, &ImagesBrowser::onImageFocus, this, wxID_ANY, wxID_ANY, (wxObject*)pnlImg);
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
				imgPnl->setBorderColour(Constants::blue);
			else
				imgPnl->setBorderColour(Constants::white);
		}
	}
	top_ = top;
}

void ImagesBrowser::setSelected(const int selected)
{
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
				imgPnl->setBorderColour(Constants::blue);
			else
				imgPnl->setBorderColour(Constants::white);
		}
	}
}

void ImagesBrowser::pageUp()
{
	int n = idata_->getNoOfRows() * idata_->getNoOfCols();
	int t;

	if (top_ == 0)
		// if at top wrap to last page
		t = idata_->getNoOfImages() - n;
	else
		// up a page or to top item if less than a page
		t = top_ >= n ? top_ - n : 0;

	displayAt(t);
}

void ImagesBrowser::pageDown()
{
	int n = idata_->getNoOfRows() * idata_->getNoOfCols();
	int t = top_ + n;

	// if displaying last page, then wrap to first page
	if (t == idata_->getNoOfImages())
		t = 0;

	// if paged to middle of final page then display whole final page
	if (t + n > idata_->getNoOfImages())
		t = idata_->getNoOfImages() - n;

	displayAt(t);
}

void ImagesBrowser::onImageFocus(wxFocusEvent& event)
{
	ImagePanel *pnlImg = (ImagePanel *)event.GetEventUserData();
	pnlImg->setBorderColour(Constants::blue);
}

void ImagesBrowser::selected(const int eventId)
{
	idata_->setSelected(top_ + eventId);
}

void ImagesBrowser::contextMenu(const int eventId)
{

}

