//#include <wx/wxprec.h>
#include <wx/log.h>
#include <wx/dataview.h>
#include <wx/renderer.h>
#include <wx/textwrapper.h>
#pragma once

// A simple renderer that wraps each word on a new line
class MultiLineCustomRenderer : public wxDataViewCustomRenderer
{
public:
	MultiLineCustomRenderer(wxDataViewCellMode mode = wxDATAVIEW_CELL_EDITABLE, int align = -1);

	virtual bool Render(wxRect rect, wxDC* dc, int state) override;

	virtual wxSize GetSize() const override;

	virtual bool SetValue(const wxVariant& value) override;

	virtual bool GetValue(wxVariant& WXUNUSED(value)) const override;

	virtual bool HasEditorCtrl() const override;

	virtual wxWindow* CreateEditorCtrl(wxWindow* parent, wxRect labelRect, const wxVariant& value) override;

	virtual bool GetValueFromEditorCtrl(wxWindow* ctrl, wxVariant& value) override;

private:
	wxString m_value;
};

wxString WrapText(wxWindow* win, const wxString& text, int widthMax);