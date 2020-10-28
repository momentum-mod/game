//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/IInput.h>
#include <vgui/IImage.h>
#include <KeyValues.h>

#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/ScrollBarSlider.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

namespace
{

enum
{						 // scroll bar will scroll a little, then continuous scroll like in windows
	SCROLL_BAR_DELAY = 400,	 // default delay for all scroll bars
	SCROLL_BAR_SPEED = 50, // this is how fast the bar scrolls when you hold down the arrow button
	SCROLLBAR_DEFAULT_WIDTH = 17,
};

//-----------------------------------------------------------------------------
// Purpose: Scroll bar button-the arrow button that moves the slider up and down.
//-----------------------------------------------------------------------------
class ScrollBarButton : public Button
{
public:
	ScrollBarButton(Panel *parent, const char *panelName, const char *text) : Button(parent, panelName, text)
	{
		SetButtonActivationType(ACTIVATE_ONPRESSED);

		SetContentAlignment(Label::a_center);
	}

	void OnMouseFocusTicked()
	{
		// pass straight up to parent
		CallParentFunction(new KeyValues("MouseFocusTicked"));
	}
 
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		Button::ApplySchemeSettings(pScheme);

		SetFont(pScheme->GetFont("Marlett", IsProportional() ));
		SetDefaultBorder(pScheme->GetBorder("ScrollBarButtonBorder"));
        SetDepressedBorder(pScheme->GetBorder("ScrollBarButtonDepressedBorder"));
		
		SetDefaultColor(GetSchemeColor("ScrollBarButton.FgColor", pScheme), GetSchemeColor("ScrollBarButton.BgColor", pScheme));
		SetArmedColor(GetSchemeColor("ScrollBarButton.ArmedFgColor", pScheme), GetSchemeColor("ScrollBarButton.ArmedBgColor", pScheme));
		SetDepressedColor(GetSchemeColor("ScrollBarButton.DepressedFgColor", pScheme), GetSchemeColor("ScrollBarButton.DepressedBgColor", pScheme));
	}

	// Don't request focus.
	// This will keep cursor focus in main window in text entry windows.
	virtual void OnMousePressed(MouseCode code)
	{
		if (!IsEnabled())
			return;
		
		if (!IsMouseClickEnabled(code))
			return;
		
		if (IsUseCaptureMouseEnabled())
		{
			{
				SetSelected(true);
				Repaint();
			}
			
			// lock mouse input to going to this button
			input()->SetMouseCapture(GetVPanel());
		}
	}
    virtual void OnMouseReleased(MouseCode code)
    {
		if (!IsEnabled())
			return;
		
		if (!IsMouseClickEnabled(code))
			return;
		
		if (IsUseCaptureMouseEnabled())
		{
			{
				SetSelected(false);
				Repaint();
			}
			
			// lock mouse input to going to this button
			input()->SetMouseCapture(NULL);
		}

		if( input()->GetMouseOver() == GetVPanel() )
		{
			SetArmed( true );
		}
    }

};

}

vgui::Panel *ScrollBar_Vertical_Factory()
{
	return new ScrollBar(nullptr, nullptr, true );
}

vgui::Panel *ScrollBar_Horizontal_Factory()
{
	return new ScrollBar(nullptr, nullptr, false );
}

DECLARE_BUILD_FACTORY_CUSTOM_ALIAS( ScrollBar, ScrollBar_Vertical, ScrollBar_Vertical_Factory );
DECLARE_BUILD_FACTORY_CUSTOM_ALIAS( ScrollBar, ScrollBar_Horizontal, ScrollBar_Horizontal_Factory );
// Default is a horizontal one
DECLARE_BUILD_FACTORY_CUSTOM( ScrollBar, ScrollBar_Horizontal_Factory );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ScrollBar::ScrollBar(Panel *parent, const char *panelName, bool vertical) : Panel(parent, panelName)
{
	_slider = nullptr;
	_button[0] = nullptr;
	_button[1] = nullptr;
	_scrollDelay = SCROLL_BAR_DELAY;
	_respond = true;
	m_pUpArrow = nullptr;
	m_pLine = nullptr;
	m_pDownArrow = nullptr;
	m_pBox = nullptr;
	m_bNoButtons = true;
	m_pOverriddenButtons[0] = nullptr;
	m_pOverriddenButtons[1] = nullptr;
    m_iSchemeScrollWide = 0;

	if (vertical)
	{
		SetSlider(new ScrollBarSlider(nullptr, "Slider", true));
		SetButton(new ScrollBarButton(nullptr, "UpButton", "t"), 0);
		SetButton(new ScrollBarButton(nullptr, "DownButton", "u"), 1);
		_button[0]->SetTextInset(0, 1);
		_button[1]->SetTextInset(0, -1);
	}
	else
	{
		SetSlider(new ScrollBarSlider(nullptr, nullptr, false));
		SetButton(new ScrollBarButton(nullptr, nullptr, "w"), 0);
		SetButton(new ScrollBarButton(nullptr, nullptr, "4"), 1);
		_button[0]->SetTextInset(0, 0);
		_button[1]->SetTextInset(0, 0);
	}

	Panel::SetPaintBorderEnabled(true);
	Panel::SetPaintBackgroundEnabled(false);
	Panel::SetPaintEnabled(true);
	SetButtonPressedScrollValue(20);
	SetBlockDragChaining( true );

	Validate();
}

//-----------------------------------------------------------------------------
// Purpose: sets up the width of the scrollbar according to the scheme
//-----------------------------------------------------------------------------
void ScrollBar::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	const char *resourceString = pScheme->GetResourceString("ScrollBar.Wide");

	if (resourceString)
	{
        m_iSchemeScrollWide = V_atoi(resourceString);
	}

	UpdateButtonsForImages();
}

//-----------------------------------------------------------------------------
// Purpose: Set the slider's Paint border enabled.
//-----------------------------------------------------------------------------
void ScrollBar::SetPaintBorderEnabled(bool state)
{
	if ( _slider )
	{
		_slider->SetPaintBorderEnabled( state );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ScrollBar::SetPaintBackgroundEnabled(bool state)
{
	if ( _slider )
	{
		_slider->SetPaintBackgroundEnabled( state );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ScrollBar::SetPaintEnabled(bool state)
{
	if ( _slider )
	{
		_slider->SetPaintEnabled( state );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Layout the scroll bar and buttons on screen
//-----------------------------------------------------------------------------
void ScrollBar::PerformLayout()
{
    int scrollWide = m_iSchemeScrollWide > 0 ? m_iSchemeScrollWide : SCROLLBAR_DEFAULT_WIDTH;
    scrollWide = GetScaledVal(scrollWide);

    if (_slider && _slider->IsVertical())
    {
        // we're vertical, so reset the width
        SetSize(scrollWide, GetTall());
    }
    else
    {
        // we're horizontal, so the width means the height
        SetSize(GetWide(), scrollWide);
    }

	if (_slider)
	{
		int wide, tall;
		GetPaintSize(wide,tall);
		if (_slider->IsVertical())
		{
			if ( m_bNoButtons )
			{
				_slider->SetBounds(0, 0, wide, tall + 1);
			}
			else
			{
				_slider->SetBounds(0, wide, wide, tall-(wide*2)+1);
				_button[0]->SetBounds(0,0, wide, wide );
				_button[1]->SetBounds(0,tall-wide ,wide, wide );
			}
		}
		else
		{
			if ( m_bNoButtons )
			{
				_slider->SetBounds(tall, 0, wide, tall + 1);
			}
			else
			{
				_slider->SetBounds(tall, -1, wide-(tall*2)+1, tall + 1 );
				_button[0]->SetBounds(0, 0, tall, tall);
				_button[1]->SetBounds(wide-tall, 0, tall, tall);
			}
		}

		// Place the images over the appropriate controls
		int x,y;
		if ( m_pUpArrow )
		{
			_button[0]->GetBounds( x,y,wide,tall );
			m_pUpArrow->SetBounds( x,y,wide,tall );
		}
		if ( m_pDownArrow )
		{
			_button[1]->GetBounds( x,y,wide,tall );
			m_pDownArrow->SetBounds( x,y,wide,tall );
		}
		if ( m_pLine )
		{
			_slider->GetBounds( x,y,wide,tall );
			m_pLine->SetBounds( x,y,wide,tall );
		}
		if ( m_pBox )
		{
			m_pBox->SetBounds( 0, wide, wide, wide );
		}

		_slider->MoveToFront();
		// after resizing our child, we should remind it to perform a layout
		_slider->InvalidateLayout();

		UpdateSliderImages();
	}

	if ( m_bAutoHideButtons )
	{
		SetScrollbarButtonsVisible( _slider->IsSliderVisible() );
	}

	if (m_bNoButtons)
	{
	    SetScrollbarButtonsVisible(false);
	}

	// get tooltips to draw
	Panel::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Set the value of the scroll bar slider.
//-----------------------------------------------------------------------------
void ScrollBar::SetValue(int value)
{
	_slider->SetValue(value);
}

//-----------------------------------------------------------------------------
// Purpose: Get the value of the scroll bar slider.
//-----------------------------------------------------------------------------
int ScrollBar::GetValue()
{
	return _slider->GetValue();
}

//-----------------------------------------------------------------------------
// Purpose: Set the range of the scroll bar slider.
// This the range of numbers the slider can scroll through.
//-----------------------------------------------------------------------------
void ScrollBar::SetRange(int min,int max)
{
	_slider->SetRange(min,max);
}

//-----------------------------------------------------------------------------
// Purpose: Gets the range of the scroll bar slider.
// This the range of numbers the slider can scroll through.
//-----------------------------------------------------------------------------
void ScrollBar::GetRange(int &min, int &max)
{
    _slider->GetRange(min, max);
}

//-----------------------------------------------------------------------------
// Purpose: Send a message when the slider is moved.
// Input  : value - 
//-----------------------------------------------------------------------------
void ScrollBar::SendSliderMoveMessage(int value)
{
	PostActionSignal(new KeyValues("ScrollBarSliderMoved", "position", value));
}

//-----------------------------------------------------------------------------
// Purpose: Called when the Slider is dragged by the user
// Input  : value - 
//-----------------------------------------------------------------------------
void ScrollBar::OnSliderMoved(int value)
{
	SendSliderMoveMessage(value);
	UpdateSliderImages();
}

//-----------------------------------------------------------------------------
// Purpose: Check if the scrollbar is vertical (true) or horizontal (false)
//-----------------------------------------------------------------------------
bool ScrollBar::IsVertical()
{
	return _slider->IsVertical();
}

//-----------------------------------------------------------------------------
// Purpose: Check if the scrollbar slider has full range.
// Normally if you have a scroll bar and the range goes from a to b and
// the slider is sized to c, the range will go from a to b-c.
// This makes it so the slider goes from a to b fully.
//-----------------------------------------------------------------------------
bool ScrollBar::HasFullRange()
{
	return _slider->HasFullRange();
}

//-----------------------------------------------------------------------------
// Purpose: Setup the indexed scroll bar button with the input params.
//-----------------------------------------------------------------------------
//LEAK: new and old slider will leak
void ScrollBar::SetButton(Button *button, int index)
{
	if(_button[index])
	{
		_button[index]->DeletePanel();
	}
	_button[index]=button;
	_button[index]->SetParent(this);
	_button[index]->AddActionSignalTarget(this);
	_button[index]->SetCommand(new KeyValues("ScrollButtonPressed", "index", index));

	Validate();
}


//-----------------------------------------------------------------------------
// Purpose: Return the indexed scroll bar button
//-----------------------------------------------------------------------------
Button* ScrollBar::GetButton(int index)
{
	return _button[index];
}


//-----------------------------------------------------------------------------
// Purpose: Set up the slider.
//-----------------------------------------------------------------------------
//LEAK: new and old slider will leak
void ScrollBar::SetSlider(ScrollBarSlider *slider)
{
	if(_slider)
	{
		_slider->DeletePanel();
	}
	_slider=slider;
	_slider->AddActionSignalTarget(this);
	_slider->SetParent(this);

	Validate();
}

//-----------------------------------------------------------------------------
// Purpose: Return a pointer to the slider.
//-----------------------------------------------------------------------------
ScrollBarSlider *ScrollBar::GetSlider()
{
	return _slider;
}

Button *ScrollBar::GetDepressedButton( int iIndex )
{
    iIndex = clamp<int>(iIndex, 0, 1);
    return m_pOverriddenButtons[iIndex] ? m_pOverriddenButtons[iIndex] : _button[iIndex];
}

//-----------------------------------------------------------------------------
// Purpose: Scrolls in response to clicking and holding on up or down arrow
// The idea is to have the slider move one step then delay a bit and then
// the bar starts moving at normal speed. This gives a stepping feeling
// to just clicking an arrow once.
//-----------------------------------------------------------------------------
void ScrollBar::OnMouseFocusTicked()
{
	int direction = 0;
	
	// top button is down
	if ( GetDepressedButton(0)->IsDepressed() )
	{
		direction = -1;
	}
	// bottom top button is down
	else if (GetDepressedButton(1)->IsDepressed())
	{
		direction = 1;
	}
	else
	{
	    direction = _slider->GetMouseFocusTickDirection();
	}

	// a button is down 
	if ( direction != 0 )  
	{
		RespondToScrollArrow(direction);
		if (_scrollDelay < system()->GetTimeMillis())
		{
			_scrollDelay = system()->GetTimeMillis() + SCROLL_BAR_SPEED;
			_respond = true; 
		}
		else
		{
			_respond = false; 
		}		
	}
	// a button is not down.
	else 
	{
		// if neither button is down keep delay at max
		_scrollDelay = system()->GetTimeMillis() + SCROLL_BAR_DELAY;
		_respond = true; 
	}
}

//-----------------------------------------------------------------------------
// Purpose: move scroll bar in response to the first button
// Input: button and direction to move scroll bar when that button is pressed
//			direction can only by +/- 1 
// Output: whether button is down or not
//-----------------------------------------------------------------------------
void ScrollBar::RespondToScrollArrow(int const direction)
{
	if (_respond)
	{
		int newValue = _slider->GetValue() + (direction * _buttonPressedScrollValue);
		_slider->SetValue(newValue);
		SendSliderMoveMessage(newValue);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Trigger layout changes when the window size is changed.
//-----------------------------------------------------------------------------
void ScrollBar::OnSizeChanged(int wide, int tall)
{
	InvalidateLayout();
	_slider->InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Set how far the scroll bar slider moves when a scroll bar button is
// pressed.
//-----------------------------------------------------------------------------
void ScrollBar::SetButtonPressedScrollValue(int value)
{
	_buttonPressedScrollValue = value;
}

//-----------------------------------------------------------------------------
// Purpose: Set the range of the rangewindow. This is how many 
// lines are displayed at one time 
// in the window the scroll bar is attached to.
// This also controls the size of the slider, its size is proportional
// to the number of lines displayed / total number of lines.
//-----------------------------------------------------------------------------
void ScrollBar::SetRangeWindow(int rangeWindow)
{
	_slider->SetRangeWindow(rangeWindow);
}

//-----------------------------------------------------------------------------
// Purpose: Get the range of the rangewindow. This is how many 
// lines are displayed at one time 
// in the window the scroll bar is attached to.
// This also controls the size of the slider, its size is proportional
// to the number of lines displayed / total number of lines.
//-----------------------------------------------------------------------------
int ScrollBar::GetRangeWindow()
{
	return _slider->GetRangeWindow();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ScrollBar::Validate()
{
	if ( _slider )
	{
		int buttonOffset = 0;
        const auto bVertical = _slider->IsVertical();

        for (const auto pButton : _button)
        {
            if (pButton && pButton->IsVisible())
            {
                buttonOffset += bVertical ? pButton->GetTall() : pButton->GetWide();
            }
        }

		_slider->SetButtonOffset(buttonOffset);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ScrollBar::SetScrollbarButtonsVisible(bool visible)
{
    for (const auto pButton : _button)
    {
        if (pButton)
        {
            pButton->SetVisible(visible);
            pButton->SetEnabled(visible);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ScrollBar::UseImages( const char *pszUpArrow, const char *pszDownArrow, const char *pszLine, const char *pszBox )
{
	if ( pszUpArrow )
	{
		if ( !m_pUpArrow )
		{
			m_pUpArrow = new vgui::ImagePanel( this, "UpArrow" );
			if ( m_pUpArrow )
			{
				m_pUpArrow->SetImage( pszUpArrow );
				m_pUpArrow->SetShouldScaleImage( true );
				m_pUpArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
				m_pUpArrow->SetAlpha( 255 );
				m_pUpArrow->SetZPos( -1 );
			}
		}

		m_pUpArrow->SetImage( pszUpArrow );
		m_pUpArrow->SetRotation( IsVertical() ? ROTATED_UNROTATED : ROTATED_CLOCKWISE_90 );
	}
	else if ( m_pUpArrow )
	{
		m_pUpArrow->MarkForDeletion();
		m_pUpArrow = nullptr;
	}

	if ( pszDownArrow )
	{
		if ( !m_pDownArrow )
		{
			m_pDownArrow = new vgui::ImagePanel( this, "DownArrow" );
			if ( m_pDownArrow )
			{
				m_pDownArrow->SetShouldScaleImage( true );
				m_pDownArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
				m_pDownArrow->SetAlpha( 255 );
				m_pDownArrow->SetZPos( -1 );
			}
		}

		m_pDownArrow->SetImage( pszDownArrow );
		m_pDownArrow->SetRotation( IsVertical() ? ROTATED_UNROTATED : ROTATED_CLOCKWISE_90 );
	}
	else if ( m_pDownArrow )
	{
		m_pDownArrow->MarkForDeletion();
		m_pDownArrow = nullptr;
	}

	if ( pszLine )
	{
		if ( !m_pLine )
		{
			m_pLine = new ImagePanel( this, "Line" );
			if ( m_pLine )
			{
				m_pLine->SetShouldScaleImage( true );
				m_pLine->SetZPos( -1 );
			}
		}

		m_pLine->SetImage( pszLine );
		m_pLine->SetRotation( IsVertical() ? ROTATED_UNROTATED : ROTATED_CLOCKWISE_90 );
	}
	else if ( m_pLine )
	{
		m_pLine->MarkForDeletion();
		m_pLine = nullptr;
	}

	if ( pszBox )
	{
		if ( !m_pBox )
		{
			m_pBox = new ImagePanel( this, "Box" );
			if ( m_pBox )
			{
				m_pBox->SetShouldScaleImage( true );
				m_pBox->SetZPos( -1 );
			}
		}

		m_pBox->SetImage( pszBox );
		m_pBox->SetRotation( IsVertical() ? ROTATED_UNROTATED : ROTATED_CLOCKWISE_90 );
	}
	else if ( m_pBox )
	{
		m_pBox->MarkForDeletion();
		m_pBox = nullptr;
	}

	UpdateButtonsForImages();
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ScrollBar::UpdateButtonsForImages( void )
{
	// Turn off parts of our drawing based on which images we're replacing it with
	if ( m_pUpArrow || m_pDownArrow )
	{
		SetScrollbarButtonsVisible( false );
		_button[0]->SetPaintBorderEnabled( false );
		_button[1]->SetPaintBorderEnabled( false );
		m_bAutoHideButtons = false;
	}
	if ( m_pLine || m_pBox )
	{
		SetPaintBackgroundEnabled( false );
		SetPaintBorderEnabled( false );

		if ( _slider )
		{
			_slider->SetPaintEnabled( false );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ScrollBar::UpdateSliderImages( void )
{
	if ( m_pUpArrow && m_pDownArrow )
	{
		// set the alpha on the up arrow
		int nMin, nMax;
		GetRange( nMin, nMax );
		int nScrollPos = GetValue();
		int nRangeWindow = GetRangeWindow();
		int nBottom = nMax - nRangeWindow;
		if ( nBottom < 0 )
		{
			nBottom = 0;
		}

		// set the alpha on the up arrow
		int nAlpha = ( nScrollPos - nMin <= 0 ) ? 90 : 255;
		m_pUpArrow->SetAlpha( nAlpha );

		// set the alpha on the down arrow
		nAlpha = ( nScrollPos >= nBottom ) ? 90 : 255;
		m_pDownArrow->SetAlpha( nAlpha );
	}

	if ( m_pLine && m_pBox )
	{
		ScrollBarSlider *pSlider = GetSlider();
		if ( pSlider && pSlider->GetRangeWindow() > 0 )
		{
			int x, y, w, t, min, max;
			m_pLine->GetBounds( x, y, w, t );

			// If our slider needs layout, force it to do it now
			if ( pSlider->IsLayoutInvalid() )
			{
				pSlider->InvalidateLayout( true );
			}
			pSlider->GetNobPos( min, max );

			if ( IsVertical() )
			{
				m_pBox->SetBounds( x, y + min, w, ( max - min ) );
			}
			else
			{
				m_pBox->SetBounds( x + min, 0, (max-min), t );
			}
		}
	}
}
void ScrollBar::ApplySettings( KeyValues *pInResourceData )
{
	BaseClass::ApplySettings( pInResourceData );

	m_bNoButtons = pInResourceData->GetBool( "nobuttons", true );

	KeyValues *pSliderKV = pInResourceData->FindKey( "Slider" );
	if ( pSliderKV && _slider )
	{
		_slider->ApplySettings( pSliderKV );
	}

	KeyValues *pDownButtonKV = pInResourceData->FindKey( "DownButton" );
	if ( pDownButtonKV && _button[0] )
	{
		_button[0]->ApplySettings( pDownButtonKV );
	}

	KeyValues *pUpButtonKV = pInResourceData->FindKey( "UpButton" );
	if ( pUpButtonKV && _button[0] )
	{
		_button[1]->ApplySettings( pUpButtonKV );
	}
}
