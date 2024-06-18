#include <wxCustomButton.h>

BEGIN_EVENT_TABLE(wxCustomButton, wxPanel)
	EVT_MOTION(wxCustomButton::mouseMoved)
	EVT_LEFT_DOWN(wxCustomButton::mouseDown)
	//EVT_LEFT_UP(wxCustomButton::mouseReleased)
	EVT_LEAVE_WINDOW(wxCustomButton::mouseLeftWindow)
	EVT_ENTER_WINDOW(wxCustomButton::mouseEnter)
	EVT_LEAVE_WINDOW(wxCustomButton::mouseLeave)
	EVT_PAINT(wxCustomButton::paintEvent)
END_EVENT_TABLE()

wxCustomButton::wxCustomButton(wxWindow* parent, wxWindowID id, const wxString& text, const wxPoint& pos, const wxSize& size) :
	wxWindow(parent, id, pos, size),
	isHovered(false),
	text(text),
	hoverColor(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUHILIGHT))
{
	SetMinSize(wxSize(buttonWidth, buttonHeight));
	buffer = wxBitmap(buttonWidth, buttonHeight);
}

void wxCustomButton::paintEvent(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	render(dc);
}

void wxCustomButton::render(wxDC& dc)
{
	if (isHovered)
	{
		dc.SetBrush(wxBrush(hoverColor));
		dc.SetPen(wxPen(*wxBLACK, 1));
		dc.SetTextForeground(*wxBLACK);
	}
	else
	{
		dc.SetBrush(wxBrush(GetBackgroundColour()));
		dc.SetPen(wxPen(*wxBLACK, 1));
		dc.SetTextForeground(wxColour(192, 192, 192));
	}

	dc.DrawRectangle(0, 0, buttonWidth, buttonHeight);
	dc.DrawText(text, 10, 5);
}

void wxCustomButton::SetHoverColor(wxColour& colour)
{
	hoverColor = colour;
	Refresh();
}

void wxCustomButton::ResetHoverColor()
{
	hoverColor = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHILIGHT);
	Refresh();
}

void wxCustomButton::mouseMoved(wxMouseEvent& event)
{
	isHovered = true;
	Refresh();
}

void wxCustomButton::mouseDown(wxMouseEvent& event)
{
	//pressedDown = true;
	//Refresh();

	wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	evt.SetEventObject(this);
	HandleWindowEvent(evt);
}

//void wxCustomButton::mouseReleased(wxMouseEvent& event)
//{
//	pressedDown = false;
//	Refresh();
//	wxMessageBox(wxT("You pressed a custom button"));
//}

void wxCustomButton::mouseLeftWindow(wxMouseEvent& event)
{
	isHovered = false;
	Refresh();
}

void wxCustomButton::mouseEnter(wxMouseEvent& event)
{
	isHovered = true;
	Refresh();
}

void wxCustomButton::mouseLeave(wxMouseEvent& event)
{
	isHovered = false;
	Refresh();
}