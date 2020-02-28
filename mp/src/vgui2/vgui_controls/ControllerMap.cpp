//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_controls/ControllerMap.h"
#include "vgui/ISurface.h"
#include "vgui/KeyCode.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

struct keystring_t
{
	int code;
	const char *name;
};

//-----------------------------------------------------------------------------
// Purpose: for the UtlMap
//-----------------------------------------------------------------------------
bool lessFunc( const int &lhs, const int &rhs )
{
	return lhs < rhs;
}

//-----------------------------------------------------------------------------
// Purpose: converts a button name string to the equivalent keycode
//-----------------------------------------------------------------------------
int StringToButtonCode( const char *name )
{
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: intercepts the keycode from its parent, and handles it according to
//			the button map.  If the keycode isn't handled, it gets passed on to the parent.
//-----------------------------------------------------------------------------
void CControllerMap::OnKeyCodeTyped( vgui::KeyCode code )
{
	int idx = m_buttonMap.Find( code );
	if ( idx != m_buttonMap.InvalidIndex() )
	{
		GetParent()->OnCommand( m_buttonMap[idx].cmd.String() );
	}
	else
	{
		// Disable input before forwarding the message
		// so it doesn't feed back here again.
		SetKeyBoardInputEnabled( false );
		GetParent()->OnKeyCodeTyped( code );
		SetKeyBoardInputEnabled( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
CControllerMap::CControllerMap( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	m_buttonMap.SetLessFunc( lessFunc );
}

//-----------------------------------------------------------------------------
// Purpose: sets up the button/command bindings
//-----------------------------------------------------------------------------
void CControllerMap::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	// loop through all the data adding items to the menu
	for (KeyValues *dat = inResourceData->GetFirstSubKey(); dat != NULL; dat = dat->GetNextKey())
	{
		if ( !Q_stricmp( dat->GetName(), "button" ) )
		{
			const char *buttonName = dat->GetString( "name", "" );
			int keycode = StringToButtonCode( buttonName );
			if ( keycode != -1 )
			{
				button_t b;
				b.cmd = CUtlSymbol( dat->GetString( "command", "" ) );

				// text and icon are optional - their existence means this button
				// should be displayed in the footer panel.
				const char *helpText = dat->GetString( "text", NULL );
				if ( helpText )
				{
					b.text = CUtlSymbol( helpText );
					b.icon = CUtlSymbol( dat->GetString( "icon", NULL ) );
				}

				m_buttonMap.Insert( keycode, b );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: gets the help text for a binding, if it exists
//-----------------------------------------------------------------------------
const char *CControllerMap::GetBindingText( int idx )
{
	CUtlSymbol s = m_buttonMap[idx].text;
	if ( s.IsValid() )
	{
		return s.String();
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: gets the icon for a binding, if it exists
//-----------------------------------------------------------------------------
const char *CControllerMap::GetBindingIcon( int idx )
{
	CUtlSymbol s = m_buttonMap[idx].icon;
	if ( s.IsValid() )
	{
		return s.String();
	}
	return NULL;
}

DECLARE_BUILD_FACTORY( CControllerMap );

