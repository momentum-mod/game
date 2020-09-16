#include "cbase.h"

#include "VControlsListPanel.h"

#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/Cursor.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

class CInlineEditPanel : public Panel
{
public:
	CInlineEditPanel() : Panel(nullptr, "InlineEditPanel")
	{
	}

    void Paint() override
    {
		int x = 0, y = 0, wide, tall;
		GetSize(wide, tall);

		// Draw a white rectangle around that cell
		surface()->DrawSetColor( m_cInlineRectColor );
		surface()->DrawFilledRect( x, y, x + wide, y + tall );
	}

    void OnKeyCodeTyped(KeyCode code) override
    {
		// forward up
		if (GetParent())
		{
			GetParent()->OnKeyCodeTyped(code);
		}
	}

    void ApplySchemeSettings(IScheme *pScheme) override
    {
		Panel::ApplySchemeSettings(pScheme);
		SetBorder(pScheme->GetBorder("DepressedButtonBorder"));

		m_cInlineRectColor = GetSchemeColor("InlineEditPanel.Color", Color(0, 165, 255, 255), pScheme);
	}

	void OnMousePressed(MouseCode code) override
    {
		// forward up mouse pressed messages to be handled by the key options
		if (GetParent())
		{
			GetParent()->OnMousePressed(code);
		}
	}

private:
	Color m_cInlineRectColor;
};

VControlsListPanel::VControlsListPanel(Panel *parent, const char *listName )	: SectionedListPanel( parent, listName )
{
	m_bCaptureMode	= false;
	m_nClickRow		= 0;
	m_pInlineEditPanel = new CInlineEditPanel();
	m_iMouseX = 0;
	m_iMouseY = 0;
}

VControlsListPanel::~VControlsListPanel()
{
	m_pInlineEditPanel->MarkForDeletion();
}

void VControlsListPanel::StartCaptureMode( HCursor hCursor )
{
	m_bCaptureMode = true;
	EnterEditMode(m_nClickRow, 1, m_pInlineEditPanel);
	input()->SetMouseFocus(m_pInlineEditPanel->GetVPanel());
	input()->SetMouseCapture(m_pInlineEditPanel->GetVPanel());

	engine->StartKeyTrapMode();

	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);

		// save off the cursor position so we can restore it
        input()->GetCursorPos( m_iMouseX, m_iMouseY );
	}
}

void VControlsListPanel::EndCaptureMode( HCursor hCursor )
{
	m_bCaptureMode = false;
	input()->SetMouseCapture(NULL);
	LeaveEditMode();
	RequestFocus();
	input()->SetMouseFocus(GetVPanel());
	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);
		surface()->SetCursor(hCursor);	
		if ( hCursor != dc_none )
		{
            input()->SetCursorPos ( m_iMouseX, m_iMouseY );	
		}
	}
}

void VControlsListPanel::SetItemOfInterest(int itemID)
{
	m_nClickRow	= itemID;
}

int VControlsListPanel::GetItemOfInterest()
{
	return m_nClickRow;
}

bool VControlsListPanel::IsCapturing( void )
{
	return m_bCaptureMode;
}

void VControlsListPanel::OnMousePressed(MouseCode code)
{
	if (IsCapturing())
	{
		// forward up mouse pressed messages to be handled by the key options
		if (GetParent())
		{
			GetParent()->OnMousePressed(code);
		}
	}
	else
	{
		BaseClass::OnMousePressed(code);
	}
}

void VControlsListPanel::OnMouseDoublePressed(MouseCode code )
{
	if (IsItemIDValid(GetSelectedItem()))
	{
		OnKeyCodePressed(KEY_ENTER);
	}
	else
	{
		BaseClass::OnMouseDoublePressed(code);
	}
}

void VControlsListPanel::OnKeyCodeTyped(KeyCode code)
{
    if (code == KEY_ENTER)
    {
		StartCaptureMode(dc_blank);
    }
	else
	{
	    BaseClass::OnKeyCodeTyped(code);
	}
}