//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "CvarNegateCheckButton.h"
#include "EngineInterface.h"
#include <vgui/IVGui.h>
#include "IGameUIFuncs.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

CvarNegateCheckButton::CvarNegateCheckButton( Panel *parent, const char *panelName, const char *text, 
	const char *cvarname )
 : CheckButton( parent, panelName, text ), m_cvarRef(cvarname)
{
	Reset();
	AddActionSignalTarget( this );
}

CvarNegateCheckButton::~CvarNegateCheckButton()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarNegateCheckButton::Paint()
{
	if ( !m_cvarRef.IsValid())
	{
		BaseClass::Paint();
		return;
	}

	float value = m_cvarRef.GetFloat();
		
	if ( value < 0 )
	{
		if ( !m_bStartState )
		{
			SetSelected( true );
			m_bStartState = true;
		}
	}
	else
	{
		if ( m_bStartState )
		{
			SetSelected( false );
			m_bStartState = false;
		}
	}
	BaseClass::Paint();
}

void CvarNegateCheckButton::Reset()
{
	if ( !m_cvarRef.IsValid() )
		return;

	float value = m_cvarRef.GetFloat();
		
	if ( value < 0 )
	{
		m_bStartState = true;
	}
	else
	{
		m_bStartState = false;
	}
	SetSelected(m_bStartState);
}

bool CvarNegateCheckButton::HasBeenModified()
{
	return IsSelected() != m_bStartState;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *panel - 
//-----------------------------------------------------------------------------
void CvarNegateCheckButton::SetSelected( bool state )
{
	BaseClass::SetSelected( state );
}

void CvarNegateCheckButton::ApplyChanges()
{
	if ( !m_cvarRef.IsValid() ) 
		return;

	float value = m_cvarRef.GetFloat();
	
	value = (float)fabs( value );
	if (value < 0.00001)
	{
		// correct the value if it's not set
		value = 0.022f;
	}

	m_bStartState = IsSelected();
	value = -value;

	float ans = m_bStartState ? value : -value;
    m_cvarRef.SetValue( ans );
}


void CvarNegateCheckButton::OnButtonChecked()
{
	if (HasBeenModified())
	{
		PostActionSignal(new KeyValues("ControlModified"));
	}
}
