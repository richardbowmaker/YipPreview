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

class DialogEx : public wxDialog
{
public:

	DialogEx(
			wxWindow *parent,
			wxWindowID id,
			const wxSize &size,
			const int buttons     = wxOK,
			const wxString &title = wxEmptyString,
			const wxPoint &pos    = wxDefaultPosition,
			long style            = wxDEFAULT_DIALOG_STYLE,
			const wxString &name  = wxDialogNameStr);

protected:

	void setTitle(const std::wstring title);
	void setTitle(const std::wstring title, FileSetT fileset);
	wxPanel  *getPanel();
	wxButton *getOk();
	wxButton *getCancel();
	wxButton *getYes();
	wxButton *getNo();
	wxButton *getApply();
	wxButton *getClose();
	wxButton *getHelp();

	// overrideables
	virtual void onOk    (wxCommandEvent &event) { EndModal(wxID_OK);     };
	virtual void onCancel(wxCommandEvent &event) { EndModal(wxID_CANCEL); };
	virtual void onYes   (wxCommandEvent &event) { EndModal(wxID_YES);    };
	virtual void onNo    (wxCommandEvent &event) { EndModal(wxID_NO);     };
	virtual void onApply (wxCommandEvent &event) { EndModal(wxID_APPLY);  };
	virtual void onClose (wxCommandEvent &event) { EndModal(wxID_CLOSE);  };
	virtual void onHelp  (wxCommandEvent &event) { EndModal(wxID_HELP);   };

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
