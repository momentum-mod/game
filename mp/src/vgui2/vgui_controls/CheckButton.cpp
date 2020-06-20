//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IBorder.h>
#include <KeyValues.h>

#include <vgui_controls/Image.h>
#include <vgui_controls/CheckButton.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

#define DEFAULT_CHECK_INSET 3

CheckImage::CheckImage(CheckButton *pCheckButton) : TextImage("g"), _border(nullptr), _CheckButton(pCheckButton)
{
    SetSize(20, 13);
}

void CheckImage::Paint()
{
	// draw background
	if (_CheckButton->IsEnabled() && _CheckButton->IsCheckButtonCheckable() )
	{
		DrawSetColor(_bgColor);
	}
	else
	{
		DrawSetColor(_CheckButton->GetDisabledBgColor());
	}
	int x, y, wide, tall;
	GetPos(x, y);
	GetSize(wide, tall);
	surface()->DrawFilledRect(x, y, x + wide, y + tall);

	// draw border box
	if (_border)
	{
		_border->Paint(x, y, x + wide, y + tall);
	}

	// draw selected check
	if (_CheckButton->IsSelected())
	{
		DrawSetTextFont(GetFont());

		if ( !_CheckButton->IsEnabled() )
		{
			DrawSetTextColor( _CheckButton->GetDisabledFgColor() );
		}
		else
		{
			DrawSetTextColor(_checkColor);
		}

		int charWide = surface()->GetCharacterWidth(GetFont(), 'b');
		int charTall = surface()->GetFontTall(GetFont());

		DrawPrintChar((wide / 2) - (charWide / 2), (tall / 2) - (charTall / 2), 'b');
	}
}

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CheckButton, CheckButton );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CheckButton::CheckButton(Panel *parent, const char *panelName, const char *text) : ToggleButton(parent, panelName, text)
{
	m_bCheckButtonCheckable = true;

	_checkBoxImage = new CheckImage(this);
	m_iCheckInset = GetScaledVal(DEFAULT_CHECK_INSET);

	SetTextImageIndex(1);
	SetImageAtIndex(0, _checkBoxImage, m_iCheckInset);

	_selectedFgColor = Color( 196, 181, 80, 255 );
	_disabledFgColor = Color(130, 130, 130, 255);
	_disabledBgColor = Color(62, 70, 55, 255);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CheckButton::~CheckButton()
{
	delete _checkBoxImage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CheckButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetDefaultColor( GetSchemeColor("CheckButton.TextColor", pScheme), GetBgColor() );
	_checkBoxImage->SetBkColor(GetSchemeColor("CheckButton.BgColor", Color(62, 70, 55, 255), pScheme));
	const auto pCheckBtnBorder = pScheme->GetBorder("CheckButtonBorder");
	_checkBoxImage->SetBorder(pCheckBtnBorder ? pCheckBtnBorder : pScheme->GetBorder("ButtonDepressedBorder"));
	_checkBoxImage->SetCheckColor(GetSchemeColor("CheckButton.Check", Color(20, 20, 20, 255), pScheme));
	_selectedFgColor = GetSchemeColor("CheckButton.SelectedTextColor", GetSchemeColor("ControlText", pScheme), pScheme);
	_disabledFgColor = GetSchemeColor("CheckButton.DisabledFgColor", Color(130, 130, 130, 255), pScheme);
	_disabledBgColor = GetSchemeColor("CheckButton.DisabledBgColor", Color(62, 70, 55, 255), pScheme);

	Color bgArmedColor = GetSchemeColor( "CheckButton.ArmedBgColor", Color(62, 70, 55, 255), pScheme); 
	SetArmedColor( GetFgColor(), bgArmedColor );

	Color bgDepressedColor = GetSchemeColor( "CheckButton.DepressedBgColor", Color(62, 70, 55, 255), pScheme); 
	SetDepressedColor( GetFgColor(), bgDepressedColor );

	_highlightFgColor = GetSchemeColor( "CheckButton.HighlightFgColor", Color(62, 70, 55, 255), pScheme); 

	_checkBoxImage->SetFont(GetSchemeFont(pScheme, nullptr, "CheckButton.CheckFont", "Marlett"));
	_checkBoxImage->ResizeImageToContent();

	const auto pInsetString = pScheme->GetResourceString("CheckButton.CheckInset");
	if (pInsetString && pInsetString[0])
	{
		m_iCheckInset = GetScaledVal(Q_atoi(pInsetString));
	}
	else
	{
	    m_iCheckInset = GetScaledVal(DEFAULT_CHECK_INSET);
	}

	SetImageAtIndex(0, _checkBoxImage, m_iCheckInset);

	const auto pTextInsetString = pScheme->GetResourceString("CheckButton.TextInset");
	if (pTextInsetString && pTextInsetString[0])
	{
		SetImageAtIndex(1, GetTextImage(), GetScaledVal(Q_atoi(pTextInsetString)));
	}

	// don't draw a background
	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IBorder *CheckButton::GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Checks the button
//-----------------------------------------------------------------------------
void CheckButton::SetSelected(bool state)
{
    SetSelected(state, true);
}

//-----------------------------------------------------------------------------
// Purpose: Checks the button without posting an action signal
//-----------------------------------------------------------------------------
void CheckButton::SilentSetSelected(bool state)
{
    SetSelected(state, false);
}

//-----------------------------------------------------------------------------
// Purpose: Checks the button & fires an action signal if bPostActionSignal is true
//-----------------------------------------------------------------------------
void CheckButton::SetSelected(bool state, bool bPostActionSignal)
{
	if (m_bCheckButtonCheckable)
	{
        if (bPostActionSignal)
        {
            // send a message saying we've been checked
            KeyValues *msg = new KeyValues("CheckButtonChecked", "state", (int)state);
            PostActionSignal(msg);
        }
		
		BaseClass::SetSelected(state);
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets whether or not the state of the check can be changed
//-----------------------------------------------------------------------------
void CheckButton::SetCheckButtonCheckable(bool state)
{
	m_bCheckButtonCheckable = state;
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Gets a different foreground text color if we are selected
//-----------------------------------------------------------------------------
Color CheckButton::GetButtonFgColor()
{
	if ( IsArmed() )
	{
		return _highlightFgColor;
	}

	if (IsSelected())
	{
		return _selectedFgColor;
	}

	return BaseClass::GetButtonFgColor();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CheckButton::OnCheckButtonChecked(Panel *panel)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CheckButton::SetHighlightColor(Color fgColor)
{
	if ( _highlightFgColor != fgColor )
	{
		_highlightFgColor = fgColor;

		InvalidateLayout(false);
	}
}

