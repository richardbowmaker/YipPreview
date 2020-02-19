
#include "ImagesBrowser.h"

#include <cmath>
#include <wx\wx.h>

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
		ImagePanel* pnlImg = new ImagePanel(this, 5);
		sizer->Add(pnlImg, 1, wxEXPAND, 0);
		//		pnlImg->Bind(wxEVT_SET_FOCUS, &ImagesBrowser::onImageFocus, this, wxID_ANY, wxID_ANY, (wxObject*)pnlImg);
	}

	top_ = -1;
	displayTop(0);
}

void ImagesBrowser::setTop(const int top)
{
	displayTop(top);
}

void ImagesBrowser::displayTop(const int top)
{
	if (top == top_) return;

	// total controls
	int n = idata_->getNoOfRows() * idata_->getNoOfCols();

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

void ImagesBrowser::onImageFocus(wxFocusEvent& event)
{
	ImagePanel *pnlImg = (ImagePanel *)event.GetEventUserData();
	pnlImg->setBorderColour(Constants::blue);
}
