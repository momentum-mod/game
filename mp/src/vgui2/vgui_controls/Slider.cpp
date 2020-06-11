//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/MouseCode.h>
#include <KeyValues.h>

#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "vgui/IBorder.h"

#include <vgui_controls/Slider.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/TextImage.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY( Slider );

#define DEFAULT_THUMB_WIDTH 8.0f
#define DEFAULT_TRACK_OFFSET_Y 8
#define DEFAULT_TRACK_HEIGHT 4
#define DEFAULT_TICK_OFFSET 4
#define DEFAULT_TICK_HEIGHT 5

//-----------------------------------------------------------------------------
// Purpose: Create a slider bar with ticks underneath it
//-----------------------------------------------------------------------------
Slider::Slider(Panel *parent, const char *panelName ) : BaseClass(parent, panelName)
{
    InitSettings();
	m_bIsDragOnRepositionNob = false;
	_dragging = false;
	_value = 0;
	_range[0] = 0;
	_range[1] = 0;
	_buttonOffset = 0;
	_sliderBorder = nullptr;
	_insetBorder = nullptr;
	m_nNumTicks = 10;
	_leftCaption = nullptr;
	_rightCaption = nullptr;

	_subrange[ 0 ] = 0;
	_subrange[ 1 ] = 0;
	m_bUseSubRange = false;
	m_bInverted = false;

	_trackOffsetX = 0;
	_trackOffsetY = GetScaledVal(DEFAULT_TRACK_OFFSET_Y);
	_trackHeight = GetScaledVal(DEFAULT_TRACK_HEIGHT);
	_tickOffset = GetScaledVal(DEFAULT_TICK_OFFSET);
	_tickHeight = GetScaledVal(DEFAULT_TICK_HEIGHT);

	SetThumbWidth( DEFAULT_THUMB_WIDTH );
	RecomputeNobPosFromValue();
	AddActionSignalTarget(this);
	SetBlockDragChaining( true );
}

// This allows the slider to behave like it's larger than what's actually being drawn
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bEnable - 
//			0 - 
//			100 - 
//-----------------------------------------------------------------------------
void Slider::SetSliderThumbSubRange( bool bEnable, int nMin /*= 0*/, int nMax /*= 100*/ )
{
	m_bUseSubRange = bEnable;
	_subrange[ 0 ] = nMin;
	_subrange[ 1 ] = nMax;
}

//-----------------------------------------------------------------------------
// Purpose: Set the size of the slider bar.
//			Warning less than 30 pixels tall and everything probably won't fit.
//-----------------------------------------------------------------------------
void Slider::OnSizeChanged(int wide,int tall)
{
	BaseClass::OnSizeChanged(wide,tall);

	RecomputeNobPosFromValue();
}

//-----------------------------------------------------------------------------
// Purpose: Set the value of the slider to one of the ticks.
//-----------------------------------------------------------------------------
void Slider::SetValue(int value, bool bTriggerChangeMessage)
{
	int oldValue=_value;

	if ( _range[0] < _range[1] )
	{
		if(value<_range[0])
		{
			value=_range[0];
		}
		if(value>_range[1])
		{
			value=_range[1];	
		}
	}
	else
	{
		if(value<_range[1])
		{
			value=_range[1];
		}
		if(value>_range[0])
		{
			value=_range[0];	
		}
	}

	_value = value;
	
	RecomputeNobPosFromValue();

	if (_value != oldValue && bTriggerChangeMessage)
	{
		SendSliderMovedMessage();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return the value of the slider
//-----------------------------------------------------------------------------
int Slider::GetValue()
{
	return _value;
}

//-----------------------------------------------------------------------------
// Purpose: Layout the slider before drawing it on screen.
//-----------------------------------------------------------------------------
void Slider::PerformLayout()
{
	BaseClass::PerformLayout();
	RecomputeNobPosFromValue();

	if (_leftCaption)
	{
		_leftCaption->ResizeImageToContent();
	}
	if (_rightCaption)
	{
		_rightCaption->ResizeImageToContent();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Move the nob on the slider in response to changing its value.
//-----------------------------------------------------------------------------
void Slider::RecomputeNobPosFromValue()
{
	//int wide,tall;
	//GetPaintSize(wide,tall);
	int x, y, wide, tall;
	GetTrackRect( x, y, wide, tall );

	float usevalue = _value;
	int *userange = _range;
	if ( m_bUseSubRange )
	{
		userange = _subrange;
		usevalue = clamp( _value, _subrange[ 0 ], _subrange[ 1 ] );
	}

	float fwide = (float)wide;
	float frange = (float) (userange[1] -userange[0]);
	float fvalue = (float) (usevalue -userange[0]);
	float fper = (frange != 0.0f) ? fvalue / frange : 0.0f;

	if ( m_bInverted )
		fper = 1.0f - fper;

	float freepixels = fwide - _thumbWidth;
	float leftpixel = (float)x;
	float firstpixel = leftpixel + freepixels * fper + 0.5f;

	_nobPos[0]=(int)( firstpixel );
	_nobPos[1]=(int)( firstpixel + _thumbWidth );


	int rightEdge = x + wide;

	if(_nobPos[1]> rightEdge )
	{
		_nobPos[0]=rightEdge-((int)_thumbWidth);
		_nobPos[1]=rightEdge;
	}
	
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Sync the slider's value up with the nob's position.
//-----------------------------------------------------------------------------
void Slider::RecomputeValueFromNobPos()
{
	int value = EstimateValueAtPos( _nobPos[ 0 ], 0 );
	SetValue( value );
}

int Slider::EstimateValueAtPos( int localMouseX, int /*localMouseY*/ )
{
	int x, y, wide, tall;
	GetTrackRect( x, y, wide, tall );

	int *userange = &_range[ 0 ];
	if ( m_bUseSubRange )
	{
		userange = &_subrange[ 0 ];
	}

	float fwide = (float)wide;
	float fvalue = (float)( _value - userange[0] );
	float fnob = (float)( localMouseX - x );
	float freepixels = fwide - _thumbWidth;

	// Map into reduced range
	fvalue = freepixels != 0.0f ? fnob / freepixels : 0.0f;

	return (int) (RemapVal( fvalue, 0.0, 1.0, userange[0], userange[1] ));
}

void Slider::SetInverted( bool bInverted )
{
	m_bInverted = bInverted;
}


//-----------------------------------------------------------------------------
// Purpose: Send a message to interested parties when the slider moves
//-----------------------------------------------------------------------------
void Slider::SendSliderMovedMessage()
{	
	// send a changed message
	KeyValues *pParams = new KeyValues("SliderMoved", "position", _value);
	pParams->SetPtr( "panel", this );
	PostActionSignal( pParams );
}

//-----------------------------------------------------------------------------
// Purpose: Send a message to interested parties when the user begins dragging the slider
//-----------------------------------------------------------------------------
void Slider::SendSliderDragStartMessage()
{	
	// send a message
	KeyValues *pParams = new KeyValues("SliderDragStart", "position", _value);
	pParams->SetPtr( "panel", this );
	PostActionSignal( pParams );
}

//-----------------------------------------------------------------------------
// Purpose: Send a message to interested parties when the user ends dragging the slider
//-----------------------------------------------------------------------------
void Slider::SendSliderDragEndMessage()
{	
	// send a message
	KeyValues *pParams = new KeyValues("SliderDragEnd", "position", _value);
	pParams->SetPtr( "panel", this );
	PostActionSignal( pParams );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Slider::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetFgColor(GetSchemeColor("Slider.NobColor", pScheme));
    SetBgColor(GetSchemeColor("Slider.BgColor", pScheme));

	m_TickColor = pScheme->GetColor( "Slider.TextColor", GetFgColor() );
	m_TrackColor = pScheme->GetColor( "Slider.TrackColor", GetFgColor() );

	m_DisabledTextColor1 = pScheme->GetColor( "Slider.DisabledTextColor1", GetFgColor() );
	m_DisabledTextColor2 = pScheme->GetColor( "Slider.DisabledTextColor2", GetFgColor() );

	_sliderBorder = pScheme->GetBorder("ButtonBorder");
	_insetBorder = pScheme->GetBorder("ButtonDepressedBorder");

	if ( _leftCaption )
	{
		_leftCaption->SetFont(pScheme->GetFont("DefaultVerySmall", IsProportional() ));
	}

	if ( _rightCaption )
	{
		_rightCaption->SetFont(pScheme->GetFont("DefaultVerySmall", IsProportional() ));
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Slider::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);
	
	char buf[256];
	if (_leftCaption)
	{
		_leftCaption->GetUnlocalizedText(buf, sizeof(buf));
		outResourceData->SetString("leftText", buf);
	}
    else
        outResourceData->SetString("leftText", "");
	
	if (_rightCaption)
	{
		_rightCaption->GetUnlocalizedText(buf, sizeof(buf));
		outResourceData->SetString("rightText", buf);
	}
    else
	    outResourceData->SetString("rightText", "");

    outResourceData->SetInt("numTicks", m_nNumTicks);
    outResourceData->SetFloat("thumbwidth", _thumbWidth);

    outResourceData->SetInt("rangeMin", _range[0]);
    outResourceData->SetInt("rangeMax", _range[1]);

	outResourceData->SetInt("trackoffset_x", _trackOffsetX);
	outResourceData->SetInt("trackoffset_y", _trackOffsetY);
	outResourceData->SetInt("trackheight", _trackHeight);

	outResourceData->SetInt("tickheight", _tickHeight);
	outResourceData->SetInt("tickoffset", _tickOffset);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Slider::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	float thumbWidth = inResourceData->GetFloat("thumbwidth", DEFAULT_THUMB_WIDTH);
	if (thumbWidth > 0.0f)
	{
		SetThumbWidth(thumbWidth);
	}

	const char *left = inResourceData->GetString("leftText", nullptr);
	const char *right = inResourceData->GetString("rightText", nullptr);
	SetTickCaptions(left, right);

	int nNumTicks = inResourceData->GetInt( "numTicks", 10 );
	if ( nNumTicks >= 0 )
	{
		SetNumTicks( nNumTicks );
	}

	_trackOffsetX = GetScaledVal(inResourceData->GetInt("trackoffset_x"));
	_trackOffsetY = GetScaledVal(inResourceData->GetInt("trackoffset_y", DEFAULT_TRACK_OFFSET_Y));
	_trackHeight = GetScaledVal(inResourceData->GetInt("trackheight", DEFAULT_TRACK_HEIGHT));

	_tickHeight = GetScaledVal(inResourceData->GetInt("tickheight", DEFAULT_TICK_HEIGHT));
	_tickOffset = GetScaledVal(inResourceData->GetInt("tickoffset", DEFAULT_TICK_OFFSET));

	const char *pRangeMin = inResourceData->GetString( "rangeMin" );
	const char *pRangeMax = inResourceData->GetString( "rangeMax" );
	bool bDoClamp = false;
	if ( *pRangeMin )
	{
		_range[0] = Q_atoi(pRangeMin);
		bDoClamp = true;
	}
	if ( *pRangeMax )
	{
		_range[1] = Q_atoi(pRangeMax);
		bDoClamp = true;
	}

	if ( bDoClamp )
	{
		ClampRange();
	}
}

void Slider::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"leftText", TYPE_STRING},
    {"rightText", TYPE_STRING},
    {"thumbwidth", TYPE_FLOAT},
    {"numTicks", TYPE_INTEGER},
    {"rangeMin", TYPE_INTEGER},
    {"rangeMax", TYPE_INTEGER},
	{"trackheight", TYPE_INTEGER},
	{"trackoffset_x", TYPE_INTEGER},
	{"trackoffset_y", TYPE_INTEGER},
	{"tickoffset", TYPE_INTEGER},
	{"tickheight", TYPE_INTEGER},
    END_PANEL_SETTINGS();
}

//-----------------------------------------------------------------------------
// Purpose: Get the rectangle to draw the slider track in.
//-----------------------------------------------------------------------------
void Slider::GetTrackRect( int& x, int& y, int& w, int& h )
{
	int wide, tall;
	GetPaintSize( wide, tall );

	x = _trackOffsetX;
	y = _trackOffsetY;
	w = wide - (int)_thumbWidth;
	h = _trackHeight;
}

//-----------------------------------------------------------------------------
// Purpose: Draw everything on screen
//-----------------------------------------------------------------------------
void Slider::Paint()
{
	DrawTicks();

	DrawTickLabels();

	// Draw nob last so it draws over ticks.
	DrawNob();
}

//-----------------------------------------------------------------------------
// Purpose: Draw the ticks below the slider.
//-----------------------------------------------------------------------------
void Slider::DrawTicks()
{
	int x, y;
	int wide,tall;
	GetTrackRect( x, y, wide, tall );

	// Figure out how to draw the ticks
//	GetPaintSize( wide, tall );

	float fwide  = (float)wide;
	float freepixels = fwide - _thumbWidth;

	float leftpixel = _thumbWidth / 2.0f;

	float pixelspertick = freepixels / ( m_nNumTicks );

	y += _trackHeight + _tickOffset;

    if (IsEnabled())
    {
        surface()->DrawSetColor( m_TickColor );
    	for ( int i = 0; i <= m_nNumTicks; i++ )
    	{
    		int xpos = (int)( leftpixel + i * pixelspertick );
    
    		surface()->DrawFilledRect( xpos, y, xpos + 1, y + _tickHeight );
    	}
    }
    else
    {
        surface()->DrawSetColor( m_DisabledTextColor1 );
    	for ( int i = 0; i <= m_nNumTicks; i++ )
    	{
    		int xpos = (int)( leftpixel + i * pixelspertick );
    		surface()->DrawFilledRect( xpos+1, y+1, xpos + 2, y + _tickHeight + 1 );
    	}
        surface()->DrawSetColor( m_DisabledTextColor2 );
    	for ( int i = 0; i <= m_nNumTicks; i++ )
    	{
    		int xpos = (int)( leftpixel + i * pixelspertick );
    		surface()->DrawFilledRect( xpos, y, xpos + 1, y + _tickHeight );
    	}
    }
}

//-----------------------------------------------------------------------------
// Purpose: Draw Tick labels under the ticks.
//-----------------------------------------------------------------------------
void Slider::DrawTickLabels()
{
	int x, y;
	int wide,tall;
	GetTrackRect( x, y, wide, tall );

	// Figure out how to draw the ticks
//	GetPaintSize( wide, tall );
	y += _trackHeight + _tickOffset + _tickHeight;

	// Draw Start and end range values
    if (IsEnabled())
	    surface()->DrawSetTextColor( m_TickColor );
    else
	    surface()->DrawSetTextColor( m_DisabledTextColor1 );


	if ( _leftCaption != nullptr )
	{
		_leftCaption->SetPos(0, y);
        if (IsEnabled())
		{
		    _leftCaption->SetColor( m_TickColor ); 
		}
        else
		{
		    _leftCaption->SetColor( m_DisabledTextColor1 ); 
		}

		_leftCaption->Paint();
	}

	if ( _rightCaption != nullptr)
	{
		int rwide, rtall;
		_rightCaption->GetSize(rwide, rtall);
		_rightCaption->SetPos((int)(wide - rwide) , y);
        if (IsEnabled())
		{
    		_rightCaption->SetColor( m_TickColor );
		}
		else
		{
    		_rightCaption->SetColor( m_DisabledTextColor1 );
		}

		_rightCaption->Paint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw the nob part of the slider.
//-----------------------------------------------------------------------------
void Slider::DrawNob()
{
	// horizontal nob
	int x, y;
	int wide,tall;
	GetTrackRect( x, y, wide, tall );
	Color col = GetFgColor();
	surface()->DrawSetColor(col);

	int nobheight = GetScaledVal(16);

	surface()->DrawFilledRect(
		_nobPos[0], 
		y + tall / 2 - nobheight / 2, 
		_nobPos[1], 
		y + tall / 2 + nobheight / 2);
	// border
	if (_sliderBorder)
	{
		_sliderBorder->Paint(
			_nobPos[0], 
			y + tall / 2 - nobheight / 2, 
			_nobPos[1], 
			y + tall / 2 + nobheight / 2);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the text labels of the Start and end ticks.
//-----------------------------------------------------------------------------
void Slider::SetTickCaptions( const char *left, const char *right )
{
	if (left)
	{
		if (_leftCaption)
		{
			_leftCaption->SetText(left);
		}
		else
		{
			_leftCaption = new TextImage(left);
		}
	}
	if (right)
	{
		if (_rightCaption)
		{
			_rightCaption->SetText(right);
		}
		else
		{
			_rightCaption = new TextImage(right);
		}
	}
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Set the text labels of the Start and end ticks.
//-----------------------------------------------------------------------------
void Slider::SetTickCaptions( const wchar_t *left, const wchar_t *right )
{
	if (left)
	{
		if (_leftCaption)
		{
			_leftCaption->SetText(left);
		}
		else
		{
			_leftCaption = new TextImage(left);
		}
	}
	if (right)
	{
		if (_rightCaption)
		{
			_rightCaption->SetText(right);
		}
		else
		{
			_rightCaption = new TextImage(right);
		}
	}
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Draw the slider track
//-----------------------------------------------------------------------------
void Slider::PaintBackground()
{
	BaseClass::PaintBackground();
	
	int x, y;
	int wide,tall;

	GetTrackRect( x, y, wide, tall );

	surface()->DrawSetColor( m_TrackColor ); 
	surface()->DrawFilledRect( x, y, x + wide, y + tall );
	if (_insetBorder)
	{
		_insetBorder->Paint( x, y, x + wide, y + tall );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the range of the slider.
//-----------------------------------------------------------------------------
void Slider::SetRange(int min,int max)
{
	_range[0]=min;
	_range[1]=max;

	ClampRange();
}

//-----------------------------------------------------------------------------
// Purpose: Sanity check and clamp the range if necessary.
//-----------------------------------------------------------------------------
void Slider::ClampRange()
{
	if ( _range[0] < _range[1] )
	{
		if(_value<_range[0])
		{
			SetValue( _range[0], false );
		}
		else if( _value>_range[1])
		{
			SetValue( _range[1], false );
		}
	}
	else
	{
		if(_value<_range[1])
		{
			SetValue( _range[1], false );
		}
		else if( _value>_range[0])
		{
			SetValue( _range[0], false );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the max and min values of the slider
//-----------------------------------------------------------------------------
void Slider::GetRange(int& min,int& max)
{
	min=_range[0];
	max=_range[1];
}

//-----------------------------------------------------------------------------
// Purpose: Respond when the cursor is moved in our window if we are clicking
// and dragging.
//-----------------------------------------------------------------------------
void Slider::OnCursorMoved(int x,int y)
{
	if(!_dragging)
	{
		return;
	}

//	input()->GetCursorPos(x,y);
	input()->GetCursorPosition( x, y );
	ScreenToLocal(x,y);

//	int wide,tall;
//	GetPaintSize(wide,tall);
	int _x, _y, wide, tall;
	GetTrackRect( _x, _y, wide, tall );

	_nobPos[0]=_nobDragStartPos[0]+(x-_dragStartPos[0]);
	_nobPos[1]=_nobDragStartPos[1]+(x-_dragStartPos[0]);

	int rightEdge = _x +wide;
	int unclamped = _nobPos[ 0 ];

	if(_nobPos[1]>rightEdge)
	{
		_nobPos[0]=rightEdge-(_nobPos[1]-_nobPos[0]);
		_nobPos[1]=rightEdge;
	}
		
	if(_nobPos[0]<_x)
	{
		int offset = _x - _nobPos[0];
		_nobPos[1]=_nobPos[1]-offset;
		_nobPos[0]=0;
	}

	int value = EstimateValueAtPos( unclamped, 0 );
	SetValue( value );

	// RecomputeValueFromNobPos();
	Repaint();
	SendSliderMovedMessage();
}

//-----------------------------------------------------------------------------
// Purpose: If you click on the slider outside of the nob, the nob jumps
// to the click position, and if this setting is enabled, the nob
// is then draggable from the new position until the mouse is released
// Input  : state - 
//-----------------------------------------------------------------------------
void Slider::SetDragOnRepositionNob( bool state )
{
	m_bIsDragOnRepositionNob = state;
}

bool Slider::IsDragOnRepositionNob() const
{
	return m_bIsDragOnRepositionNob;
}

bool Slider::IsDragged( void ) const
{
	return _dragging;
}

//-----------------------------------------------------------------------------
// Purpose: Respond to mouse presses. Trigger Record staring positon.
//-----------------------------------------------------------------------------
void Slider::OnMousePressed(MouseCode code)
{
	int x,y;

    if (!IsEnabled())
        return;

//	input()->GetCursorPos(x,y);
	input()->GetCursorPosition( x, y );

	ScreenToLocal(x,y);
    RequestFocus();

	bool startdragging = false, bPostDragStartSignal = false;

	if ((x >= _nobPos[0]) && (x < _nobPos[1]))
	{
		startdragging = true;
		bPostDragStartSignal = true;
	}
	else
	{
		// we clicked elsewhere on the slider; move the nob to that position
		int min, max;
		GetRange(min, max);
		if ( m_bUseSubRange )
		{
			min = _subrange[ 0 ];
			max = _subrange[ 1 ];
		}

//		int wide = GetWide();
		int _x, _y, wide, tall;
		GetTrackRect( _x, _y, wide, tall );
		if ( wide > 0 )
		{
			float frange = ( float )( max - min );
			float clickFrac = clamp( ( float )( x - _x ) / (float)( wide - 1 ), 0.0f, 1.0f );

			float value = (float)min + clickFrac * frange;

			startdragging = IsDragOnRepositionNob();

			if ( startdragging )
			{
				_dragging = true; // Required when as
				SendSliderDragStartMessage();
			}

			SetValue( ( int )( value + 0.5f ) );
		}
	}

	if ( startdragging )
	{
		// drag the nob
		_dragging = true;
		input()->SetMouseCapture(GetVPanel());
		_nobDragStartPos[0] = _nobPos[0];
		_nobDragStartPos[1] = _nobPos[1];
		_dragStartPos[0] = x;
		_dragStartPos[1] = y;
	}

	if ( bPostDragStartSignal )
		SendSliderDragStartMessage();
}

//-----------------------------------------------------------------------------
// Purpose: Just handle double presses like mouse presses
//-----------------------------------------------------------------------------
void Slider::OnMouseDoublePressed(MouseCode code)
{
	OnMousePressed(code);
}

//-----------------------------------------------------------------------------
// Purpose: Handle key presses
//-----------------------------------------------------------------------------
void Slider::OnKeyCodeTyped(KeyCode code)
{
	switch (code)
	{
		// for now left and right arrows just open or close submenus if they are there.
        case KEY_LEFT:
        case KEY_DOWN:
            {
                int val = GetValue();
                SetValue(val-1);
                break;
            }
    	case KEY_RIGHT:
        case KEY_UP:
    		{
                int val = GetValue();
                SetValue(val+1);
    			break;
    		}
        case KEY_PAGEDOWN:
            {
                int min, max;
                GetRange(min, max);
                float range = (float) max-min;
                float pertick = range/m_nNumTicks;
                int val = GetValue();
                SetValue(val - (int) pertick);
                break;
            }
        case KEY_PAGEUP:
            {
                int min, max;
                GetRange(min, max);
                float range = (float) max-min;
                float pertick = range/m_nNumTicks;
                int val = GetValue();
                SetValue(val + (int) pertick);
                break;
            }
        case KEY_HOME:
            {
                int min, max;
                GetRange(min, max);
                SetValue(min);
    			break;
            }
        case KEY_END:
            {
                int min, max;
                GetRange(min, max);
                SetValue(max);
    			break;
            }
    	default:
    		BaseClass::OnKeyCodeTyped(code);
    		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop dragging when the mouse is released.
//-----------------------------------------------------------------------------
void Slider::OnMouseReleased(MouseCode code)
{
	if ( _dragging )
	{
		_dragging=false;
		input()->SetMouseCapture(null);
	}

	if ( IsEnabled() )
	{
		SendSliderDragEndMessage();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the nob's position (the ends of each side of the nob)
//-----------------------------------------------------------------------------
void Slider::GetNobPos(int& min, int& max)
{
	min=_nobPos[0];
	max=_nobPos[1];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Slider::SetButtonOffset(int buttonOffset)
{
	_buttonOffset=buttonOffset;
}

void Slider::SetThumbWidth( float width )
{
	_thumbWidth = width;
}


//-----------------------------------------------------------------------------
// Purpose: Set the number of ticks that appear under the slider.
//-----------------------------------------------------------------------------
void Slider::SetNumTicks( int ticks )
{
	m_nNumTicks = ticks;
}
