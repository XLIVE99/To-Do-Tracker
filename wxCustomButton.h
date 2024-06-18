#include <wx/wx.h>
#include <wx/sizer.h>
#pragma once

class wxCustomButton : public wxWindow
{
	bool isHovered;
	wxString text;

	static const int buttonWidth = 200;
	static const int buttonHeight = 50;

	wxBitmap buffer; // Double-Buffer
	wxColour hoverColor;

public:
	wxCustomButton(wxWindow* parent, wxWindowID id, const wxString& text, const wxPoint& pos, const wxSize& size);

	void paintEvent(wxPaintEvent& evt);
	void render(wxDC& dc);

	void SetHoverColor(wxColour& colour);
	void ResetHoverColor();

	// Mouse events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	//void mouseReleased(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void mouseEnter(wxMouseEvent& event);
	void mouseLeave(wxMouseEvent& event);

	DECLARE_EVENT_TABLE()
};