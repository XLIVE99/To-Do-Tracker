#include <MultiLineCustomRenderer.h>

MultiLineCustomRenderer::MultiLineCustomRenderer(wxDataViewCellMode mode, int align)
	: wxDataViewCustomRenderer("string", mode, align)
{ }

bool MultiLineCustomRenderer::Render(wxRect rect, wxDC* dc, int state)
{
	int columnWidth = GetOwner()->GetWidth() - 20;
	wxDataViewCtrl* parentCtrl = GetOwner()->GetOwner();
	wxString modified = WrapText(parentCtrl, m_value, columnWidth);

	int flags = 0;
	if (state & wxDATAVIEW_CELL_SELECTED)
		flags |= wxCONTROL_SELECTED;
	if (!parentCtrl->IsEnabled() && GetEnabled())
		flags |= wxCONTROL_DISABLED;

	wxDataViewItemAttr attr = GetAttr();

	// First render the box
	//dc->SetBrush(attr.GetBackgroundColour());
	//dc->DrawRectangle(rect);

	// Then render the text
	wxRendererNative::Get().DrawItemText(
		parentCtrl,
		*dc,
		modified,
		rect,
		GetEffectiveAlignment(),
		flags,
		wxELLIPSIZE_NONE
	);

	return true;
}

wxSize MultiLineCustomRenderer::GetSize() const
{
	int columnWidth = GetOwner()->GetWidth() - 20;
	wxSize txtSize = GetTextExtent(m_value);

	wxString wrapped = WrapText(GetOwner()->GetOwner(), m_value, columnWidth);

	int lines = wrapped.Freq('\n') + 1;
	//txtSize.SetHeight(txtSize.GetHeight() * lines);

	//std::cout << "text size (" << txtSize.x << ", " << txtSize.y << ")" << std::endl;
	//std::cout << "column width: " << columnWidth << std::endl;

	//int heightMultiplier = (txtSize.x / columnWidth) + 1;

	//std::cout << "height multiplier: " << heightMultiplier << std::endl;
	//std::cout << "result size: (" << columnWidth << ", " << (txtSize.y * heightMultiplier) << ")" << std::endl;

	return wxSize(columnWidth, txtSize.y * lines);
}

bool MultiLineCustomRenderer::SetValue(const wxVariant& value)
{
	m_value = value.GetString();

	wxString test = WrapText(GetOwner()->GetOwner(), value.GetString(), GetOwner()->GetWidth());
	//std::cout << "new lines " << test.Freq('\n') << std::endl;

	return true;
}

bool MultiLineCustomRenderer::GetValue(wxVariant& WXUNUSED(value)) const
{
	return true;
}

bool MultiLineCustomRenderer::HasEditorCtrl() const
{
	return true;
}

wxWindow* MultiLineCustomRenderer::CreateEditorCtrl(wxWindow* parent, wxRect labelRect, const wxVariant& value)
{
	wxSize rectSize = labelRect.GetSize();
	wxTextCtrl* textCtrl = new wxTextCtrl(parent, wxID_ANY, value, labelRect.GetPosition(), wxSize(rectSize.x, rectSize.y + 20), wxTE_MULTILINE);

	textCtrl->SetInsertionPointEnd();

	return textCtrl;
}

bool MultiLineCustomRenderer::GetValueFromEditorCtrl(wxWindow* ctrl, wxVariant& value)
{
	wxTextCtrl* textCtrl = wxDynamicCast(ctrl, wxTextCtrl);
	if (!textCtrl)
		return false;

	if (textCtrl->GetValue().IsEmpty())
	{
		wxLogError("Task can't be empty!");

		return false;
	}

	value = textCtrl->GetValue();

	return true;
}

wxString WrapText(wxWindow* win, const wxString& text, int widthMax)
{
	class HardBreakWrapper : public wxTextWrapper
	{
	public:
		HardBreakWrapper(wxWindow* win, const wxString& text, int widthMax)
		{
			Wrap(win, text, widthMax);
		}

		wxString const& GetWrapped() const { return m_wrapped; }

	protected:
		virtual void OnOutputLine(const wxString& line)
		{
			m_wrapped += line;
		}

		virtual void OnNewLine()
		{
			m_wrapped += '\n';
		}
	private:
		wxString m_wrapped;
	};

	HardBreakWrapper wrapper(win, text, widthMax);
	return wrapper.GetWrapped();
}