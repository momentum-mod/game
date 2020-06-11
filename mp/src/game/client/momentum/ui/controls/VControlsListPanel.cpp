#include "cbase.h"

#include "VControlsListPanel.h"

#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/Cursor.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: panel used for inline editing of key bindings
//-----------------------------------------------------------------------------
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

	void OnMousePressed(vgui::MouseCode code) override
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

//-----------------------------------------------------------------------------
// Purpose: Construction
//-----------------------------------------------------------------------------
VControlsListPanel::VControlsListPanel( vgui::Panel *parent, const char *listName )	: vgui::SectionedListPanel( parent, listName )
{
	m_bCaptureMode	= false;
	m_nClickRow		= 0;
	m_pInlineEditPanel = new CInlineEditPanel();
	m_iMouseX = 0;
	m_iMouseY = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
VControlsListPanel::~VControlsListPanel()
{
	m_pInlineEditPanel->MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VControlsListPanel::ApplySchemeSettings(IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: Start capture prompt display
//-----------------------------------------------------------------------------
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
		vgui::input()->GetCursorPos( m_iMouseX, m_iMouseY );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finish capture prompt display
//-----------------------------------------------------------------------------
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
			vgui::input()->SetCursorPos ( m_iMouseX, m_iMouseY );	
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set active row column
//-----------------------------------------------------------------------------
void VControlsListPanel::SetItemOfInterest(int itemID)
{
	m_nClickRow	= itemID;
}

//-----------------------------------------------------------------------------
// Purpose: Retrieve row, column of interest
//-----------------------------------------------------------------------------
int VControlsListPanel::GetItemOfInterest()
{
	return m_nClickRow;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if we're currently waiting to capture a key
//-----------------------------------------------------------------------------
bool VControlsListPanel::IsCapturing( void )
{
	return m_bCaptureMode;
}

//-----------------------------------------------------------------------------
// Purpose: Forwards mouse pressed message up to keyboard page when in capture
//-----------------------------------------------------------------------------
void VControlsListPanel::OnMousePressed(vgui::MouseCode code)
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


//-----------------------------------------------------------------------------
// Purpose: input handler
//-----------------------------------------------------------------------------
void VControlsListPanel::OnMouseDoublePressed( vgui::MouseCode code )
{
	if (IsItemIDValid(GetSelectedItem()))
	{
		// enter capture mode
		OnKeyCodePressed(KEY_ENTER);
	}
	else
	{
		BaseClass::OnMouseDoublePressed(code);
	}
}
