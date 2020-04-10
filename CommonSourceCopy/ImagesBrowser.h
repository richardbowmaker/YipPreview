
#ifndef COMMON_IMAGESBROWSER_H_
#define COMMON_IMAGESBROWSER_H_

#include <string>
#include <wx/wx.h>

#include "ImagePanel.h"

class ImagesBrowserServer
{
public:

	virtual ~ImagesBrowserServer() = default;

	virtual int browserGetNoOfRows() = 0;
	virtual int browserGetNoOfCols() = 0;
	virtual int browserGetNoOfImages() = 0;
	virtual int browserGetSelected() = 0;
	virtual void browserSetSelected(const int selected) = 0;
	virtual std::string browserGetImage(const int n) = 0;
	virtual std::string browserGetVideo(const int n) = 0;
	virtual wxMenu *browserGetPopupMenu(const int item) = 0;
};

class ImagesBrowser : public wxPanel, ImagePanelEvents
{
public:

	ImagesBrowser(wxWindow *parent, ImagesBrowserServer *iServer);
	virtual ~ImagesBrowser();

	void initialise();
	void uninitialise();
	void setTop(const int top);
	int getTop();
	void displayAt(const int top);
	void setSelected(const int selected);
	void pageUp();
	void pageDown();
	void cursorUp();
	void cursorDown();
	void cursorLeft();
	void cursorRight();
	bool hasFocus();	
	void setFocus(const bool focus);
	void stopPreview();

	// ImagePanelEvents
	virtual void imageSelected(const int eventId);
	virtual wxMenu *getPopupMenu(const int eventId);

private:

	void cursorMove(const int step);

	int top_;
	ImagesBrowserServer *iServer_;
	bool focus_;
};

#endif // COMMON_IMAGESBROWSER_H_
