/*
 * DialogEx.h
 *
 *  Created on: 10 Mar 2020
 *      Author: richard
 */

#ifndef COMMON_DIALOGEX_H_
#define COMMON_DIALOGEX_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <string>

#include "FileSet.h"
#include "Properties.h"

class DialogEx : public wxDialog
{
public:

	static int Run(wxWindow* parent);

	DialogEx(
			wxWindow *parent,
			wxWindowID id,
			const wxSize &size,
			const int buttons     = wxOK,
			const wxString &title = wxEmptyString,
			const wxPoint &pos    = wxDefaultPosition,
			long style            = wxDEFAULT_DIALOG_STYLE,
			const wxString &name  = wxDialogNameStr);

	virtual ~DialogEx() = default;

protected:

	void setTitle(const std::string title);
	void setTitle(const std::string title, FileSetT fileset);
	wxPanel  *getPanel();
	wxButton *getOk();
	wxButton *getCancel();
	wxButton *getYes();
	wxButton *getNo();
	wxButton *getApply();
	wxButton *getClose();
	wxButton *getHelp();

	// overrideables
	virtual void onOk    (wxCommandEvent &event) { EndModal(wxID_OK);     event.Skip(); };
	virtual void onCancel(wxCommandEvent &event) { EndModal(wxID_CANCEL); event.Skip(); };
	virtual void onYes   (wxCommandEvent &event) { EndModal(wxID_YES);    event.Skip(); };
	virtual void onNo    (wxCommandEvent &event) { EndModal(wxID_NO);     event.Skip(); };
	virtual void onApply (wxCommandEvent &event) { EndModal(wxID_APPLY);  event.Skip(); };
	virtual void onClose (wxCommandEvent &event) { EndModal(wxID_CLOSE);  event.Skip(); };
	virtual void onHelp  (wxCommandEvent &event) { EndModal(wxID_HELP);   event.Skip(); };

private:

	wxPanel  *panel_;
	wxButton *ok_;
	wxButton *cancel_;
	wxButton *yes_;
	wxButton *no_;
	wxButton *apply_;
	wxButton *close_;
	wxButton *help_;
};

#endif /* COMMON_DIALOGEX_H_ */
