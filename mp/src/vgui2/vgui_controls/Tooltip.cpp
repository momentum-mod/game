//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
// This class is a message box that has two buttons, ok and cancel instead of
// just the ok button of a message box. We use a message box class for the ok button
// and implement another button here.
//=============================================================================//

#include <math.h>
#define PROTECTED_THINGS_DISABLE

#include <vgui/IInput.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/IPanel.h>

#include <vgui_controls/Tooltip.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;


//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
static vgui::DHANDLE< TextEntry > s_TooltipWindow;
static int s_iTooltipWindowCount = 0;


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
BaseTooltip::BaseTooltip(Panel *parent, const char *text) 
{
	SetText(text);

	_displayOnOneLine = false;
	_makeVisible = false;
    _visible = false;
	_isDirty = false;
	_enabled = true;

	_tooltipDelay = 0; // default delay for opening tooltips
	_delay = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Reset the tooltip delay timer
//-----------------------------------------------------------------------------
void BaseTooltip::ResetDelay()
{
	_isDirty = true;
	_delay = system()->GetTimeMillis() + _tooltipDelay;
}

//-----------------------------------------------------------------------------
// Purpose: Set the tooltip delay before a tooltip comes up.
//-----------------------------------------------------------------------------
void BaseTooltip::SetTooltipDelay( int tooltipDelay )
{
	_tooltipDelay = tooltipDelay;
}

//-----------------------------------------------------------------------------
// Purpose: Get the tooltip delay before a tooltip comes up.
//-----------------------------------------------------------------------------
int BaseTooltip::GetTooltipDelay()
{
	return _tooltipDelay;
}

//-----------------------------------------------------------------------------
// Purpose: Set the tool tip to display on one line only
//			Default is multiple lines.
//-----------------------------------------------------------------------------
void BaseTooltip::SetTooltipFormatToSingleLine()
{
	_displayOnOneLine = true;
	_isDirty = true;
}

//-----------------------------------------------------------------------------
// Purpose: Set the tool tip to display on multiple lines.
//-----------------------------------------------------------------------------
void BaseTooltip::SetTooltipFormatToMultiLine()
{
	_displayOnOneLine = false;
	_isDirty = true;
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void BaseTooltip::ShowTooltip(Panel *currentPanel)
{
	_makeVisible = true;

	PerformLayout();
}

void BaseTooltip::SetEnabled( bool bState )
{
	_enabled = bState;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool BaseTooltip::ShouldLayout( void )
{
	if (!_makeVisible)
		return false;

	if (_delay > system()->GetTimeMillis())
		return false;	

	// We only need to layout when we first become visible
	if ( !_isDirty )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void BaseTooltip::HideTooltip()
{
	_makeVisible = false;
	_isDirty = true;
    _visible = false;
}

//-----------------------------------------------------------------------------
// Purpose: Set the tooltip text
//-----------------------------------------------------------------------------
void BaseTooltip::SetText(const char *text)
{
	_isDirty = true;

	if (!text)
	{
		text = "";
	}

	if (m_Text.Size() > 0)
	{
		m_Text.RemoveAll();
	}
    const size_t textSize = strlen(text);
    for (unsigned int i = 0; i < textSize; i++)
	{
		m_Text.AddToTail(text[i]);
	}
	m_Text.AddToTail('\0');
	
	if (s_TooltipWindow.Get() && m_pParent == s_TooltipWindow.Get()->GetParent())
	{
		s_TooltipWindow->SetText(m_Text.Base());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the tooltip text
//-----------------------------------------------------------------------------
const char *BaseTooltip::GetText()
{
	return m_Text.Base();
}

//-----------------------------------------------------------------------------
// Purpose: Position the tool tip
//-----------------------------------------------------------------------------
void BaseTooltip::PositionWindow( void )
{
    Panel *pTipPanel = s_TooltipWindow.Get();
	int iTipW, iTipH;
	pTipPanel->GetSize( iTipW, iTipH );

	int cursorX, cursorY;
	input()->GetCursorPos(cursorX, cursorY);

	int wide, tall;
	surface()->GetScreenSize(wide, tall);

	if (wide - iTipW > cursorX)
	{
		cursorY += 20;
		// menu hanging right
		if (tall - iTipH > cursorY)
		{
			// menu hanging down
			pTipPanel->SetPos(cursorX, cursorY);
		}
		else
		{
			// menu hanging up
			pTipPanel->SetPos(cursorX, cursorY - iTipH - 20);
		}
	}
	else
	{
		// menu hanging left
		if (tall - iTipH > cursorY)
		{
			// menu hanging down
			pTipPanel->SetPos(cursorX - iTipW, cursorY);
		}
		else
		{
			// menu hanging up
			pTipPanel->SetPos(cursorX - iTipW, cursorY - iTipH - 20 );
		}
	}	
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
TextTooltip::TextTooltip(Panel *parent, const char *text) : BaseTooltip( parent, text )
{
	if (!s_TooltipWindow.Get())
	{
		s_TooltipWindow = new TextEntry(nullptr, "tooltip");
        s_TooltipWindow->SetProportional(parent->IsProportional());

 		s_TooltipWindow->InvalidateLayout(false, true);

		// this bit of hackery is necessary because tooltips don't get ApplySchemeSettings called from their parents
        ApplySchemeSettings(parent->GetScheme());
	}
	s_iTooltipWindowCount++;

	// this line prevents the main window from losing focus
	// when a tooltip pops up
	s_TooltipWindow->MakePopup(false, true);
	s_TooltipWindow->SetKeyBoardInputEnabled( false );
	s_TooltipWindow->SetMouseInputEnabled( false );
	
	SetText(text);
	s_TooltipWindow->SetText(m_Text.Base());
	s_TooltipWindow->SetEditable(false);
	s_TooltipWindow->SetMultiline(true);
	s_TooltipWindow->SetVisible(false);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
TextTooltip::~TextTooltip()
{
	if (--s_iTooltipWindowCount < 1)
	{
		if ( s_TooltipWindow.Get() )
		{
			s_TooltipWindow->MarkForDeletion();
		}
		s_TooltipWindow = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the tooltip text
//-----------------------------------------------------------------------------
void TextTooltip::SetText(const char *text)
{
	BaseTooltip::SetText( text );
	
	if (s_TooltipWindow.Get())
	{
		s_TooltipWindow->SetText(m_Text.Base());
        SizeTextWindow();
	}
}

//-----------------------------------------------------------------------------
// Purpose: gets the font from the scheme
//-----------------------------------------------------------------------------
void TextTooltip::ApplySchemeSettings(HScheme hScheme)
{
    IScheme *pScheme = scheme()->GetIScheme(hScheme);
	if ( s_TooltipWindow && pScheme )
	{
        s_TooltipWindow->SetBgColor(s_TooltipWindow->GetSchemeColor("Tooltip.BgColor", s_TooltipWindow->GetBgColor(), pScheme));
        s_TooltipWindow->SetFgColor(s_TooltipWindow->GetSchemeColor("Tooltip.TextColor", s_TooltipWindow->GetFgColor(), pScheme));
        s_TooltipWindow->SetBorder(pScheme->GetBorder("ToolTipBorder"));

        const char *pFontName = pScheme->GetResourceString("Tooltip.TextFont");
        HFont font = INVALID_FONT;
	    if (pFontName && pFontName[0] != '\0') 
            font = pScheme->GetFont(pFontName, s_TooltipWindow->IsProportional());
        if (font == INVALID_FONT)
	        font = pScheme->GetFont("DefaultSmall", s_TooltipWindow->IsProportional());
        s_TooltipWindow->SetFont(font);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void TextTooltip::ShowTooltip(Panel *currentPanel)
{
	if ( s_TooltipWindow.Get() )
	{
        if (s_TooltipWindow->GetTextLength() <= 0)
		{
			// Empty tool tip, no need to show it
			_makeVisible = false;
			return;
		}

		Panel *pCurrentParent = s_TooltipWindow->GetParent();

		_isDirty = _isDirty || pCurrentParent != currentPanel;
		s_TooltipWindow->SetParent(currentPanel);
        ApplySchemeSettings(currentPanel->GetScheme());
		s_TooltipWindow->SetText( m_Text.Base() );
	}
	BaseTooltip::ShowTooltip( currentPanel );
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void TextTooltip::PerformLayout()
{
	if ( !ShouldLayout() )
		return;
	// we're ready, just make us visible
	if ( !s_TooltipWindow.Get() )
		return;

	_isDirty = false;
    _visible = true;

	s_TooltipWindow->SetVisible(true);
	surface()->MovePopupToFront(s_TooltipWindow->GetVPanel());
	s_TooltipWindow->SetKeyBoardInputEnabled( false );
	s_TooltipWindow->SetMouseInputEnabled( false );

    SizeTextWindow();
    PositionWindow();
}

//-----------------------------------------------------------------------------
// Purpose: Size the text window so all the text fits inside it.
//-----------------------------------------------------------------------------
void TextTooltip::SizeTextWindow()
{
	if ( !s_TooltipWindow.Get() )
		return;

	if (_displayOnOneLine)
	{
		// We want the tool tip to be one line
		s_TooltipWindow->SetMultiline(false);
		s_TooltipWindow->SetToFullWidth();
	}
	else
	{
		// We want the tool tip to be one line
		s_TooltipWindow->SetMultiline(false);
		s_TooltipWindow->SetToFullWidth();
		int wide, tall;
		s_TooltipWindow->GetSize( wide, tall );
		double newWide = sqrt( (2.0/1) * wide * tall );
		int newTall = newWide / 2;
		s_TooltipWindow->SetMultiline(true);
		s_TooltipWindow->SetSize(int(newWide), newTall );
		s_TooltipWindow->SetToFullHeight();
		
		s_TooltipWindow->GetSize( wide, tall );
		
		if (( wide < 100 ) && ( s_TooltipWindow->GetNumLines() == 2) ) 
		{
			s_TooltipWindow->SetMultiline(false);
			s_TooltipWindow->SetToFullWidth();	
		}
		else
		{
			
			while ( (float)wide/(float)tall < 2 )
			{
				s_TooltipWindow->SetSize( wide + 1, tall );
				s_TooltipWindow->SetToFullHeight();
				s_TooltipWindow->GetSize( wide, tall );
			}
		}
		//s_TooltipWindow->GetSize( wide, tall );
	//	ivgui()->DPrintf("End Ratio: %f\n", (float)wide/(float)tall);		
	}
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void TextTooltip::HideTooltip()
{
	if ( s_TooltipWindow.Get() )
	{
		s_TooltipWindow->SetVisible(false);
	}

	BaseTooltip::HideTooltip();
}

