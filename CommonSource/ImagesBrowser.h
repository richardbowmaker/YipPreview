
#ifndef COMMON_IMAGESBROWSER_H_
#define COMMON_IMAGESBROWSER_H_

#include <string>
#include <wx/wx.h>

#include "ImagePanel.h"

class ImagesBrowserData
{
public:

	virtual ~ImagesBrowserData() = default;

	virtual int getNoOfRows() = 0;
	virtual int getNoOfCols() = 0;
	virtual int getNoOfImages() = 0;
	virtual int getSelected() = 0;
	virtual void setSelected(const int selected) = 0;
	virtual std::wstring getImage(const int n) = 0;
};

class ImagesBrowser : public wxPanel, ImagePanelEvents
{
public:

	ImagesBrowser(wxWindow *parent, ImagesBrowserData *idata);
	virtual ~ImagesBrowser();

	void initialise();
	void setTop(int top);
	void setSelected(const int selected);
	void pageUp();
	void pageDown();
	void cursorUp();
	void cursorDown();
	void cursorLeft();
	void cursorRight();
	bool hasFocus();	
	void setFocus(const bool focus);

	// ImagePanelEvents
	virtual void selected(const int eventId);
	virtual void contextMenu(const int eventId);

private:

	void displayAt(const int top);
	void cursorMove(const int step);

	int top_;
	ImagesBrowserData *idata_;
	bool focus_;
};

#endif // COMMON_IMAGESBROWSER_H_
