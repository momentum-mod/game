//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_controls/pch_vgui_controls.h"
#include <vgui_controls/KeyRepeat.h>

// memdbgon must be the last include file in a .cpp file
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKeyRepeatHandler::KeyDown( vgui::KeyCode code )
{
	int iIndex = GetIndexForCode(code);
	if ( iIndex == -1 )
		return;

	if ( m_bAliasDown[ iIndex ] )
		return;

	Reset();
	m_bAliasDown[ iIndex ] = true;
	m_flNextKeyRepeat = system()->GetCurrentTime() + 0.4;
	m_bHaveKeyDown = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKeyRepeatHandler::KeyUp( vgui::KeyCode code )
{
	int iIndex = GetIndexForCode(code);
	if ( iIndex == -1 )
		return;

	m_bAliasDown[ GetIndexForCode(code) ] = false;

	m_bHaveKeyDown = false;
	for ( int i = 0; i < FM_NUM_KEYREPEAT_ALIASES; i++ )
	{
		if ( m_bAliasDown[i] )
		{
			m_bHaveKeyDown = true;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
vgui::KeyCode CKeyRepeatHandler::KeyRepeated( void )
{
	return BUTTON_CODE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKeyRepeatHandler::SetKeyRepeatTime( vgui::KeyCode code, float flRepeat )
{
	int iIndex = GetIndexForCode(code);
	Assert( iIndex != -1 );
	if ( iIndex == -1 )
		return;
	m_flRepeatTimes[ iIndex ] = flRepeat;
}