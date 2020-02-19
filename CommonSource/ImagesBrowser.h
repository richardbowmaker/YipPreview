
#ifndef COMMON_IMAGESBROWSER_H_
#define COMMON_IMAGESBROWSER_H_

#include <string>
#include <wx\wx.h>

class ImagesBrowserData
{
public:

	virtual int getNoOfRows() = 0;
	virtual int getNoOfCols() = 0;
	virtual int getNoOfImages() = 0;
	virtual int getSelected() = 0;
	virtual std::wstring getImage(const int n) = 0;
};

class ImagesBrowser : public wxPanel
{
public:

	ImagesBrowser(wxWindow *parent, ImagesBrowserData *idata);
	virtual ~ImagesBrowser();

	void initialise();
	void setTop(const int top);

private:

	void displayTop(const int top);
	void onImageFocus(wxFocusEvent& event);

	int top_;
	ImagesBrowserData *idata_;
};

#endif COMMON_IMAGESBROWSER_H_