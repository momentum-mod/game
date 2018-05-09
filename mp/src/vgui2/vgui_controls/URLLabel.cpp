//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>

#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/MouseCode.h"
#include "vgui/Cursor.h"
#include "KeyValues.h"

#include "vgui_controls/URLLabel.h"
#include "vgui_controls/TextImage.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

vgui::Panel *URLLabel_Factory()
{
	return new URLLabel(nullptr, nullptr, "URLLabel", nullptr);
}

DECLARE_BUILD_FACTORY_CUSTOM( URLLabel, URLLabel_Factory );
//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
URLLabel::URLLabel(Panel *parent, const char *panelName, const char *text, const char *pszURL) : Label(parent, panelName, text)
{
    InitSettings();
    m_pszURL = nullptr;
	m_bUnderline = false;
    if (pszURL && strlen(pszURL) > 0)
    {
        SetURL(pszURL);
    }
}

//-----------------------------------------------------------------------------
// Purpose: unicode constructor
//-----------------------------------------------------------------------------
URLLabel::URLLabel(Panel *parent, const char *panelName, const wchar_t *wszText, const char *pszURL) : Label(parent, panelName, wszText)
{
    InitSettings();
    m_pszURL = nullptr;
	m_bUnderline = false;
    if (pszURL && strlen(pszURL) > 0)
    {
        SetURL(pszURL);
    }
}

//-----------------------------------------------------------------------------
// Purpose: destructor
//-----------------------------------------------------------------------------
URLLabel::~URLLabel()
{
    m_pszURL.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: sets the URL
//-----------------------------------------------------------------------------
void URLLabel::SetURL(const char *pszURL)
{
    m_pszURL.Purge();
    m_pszURL = pszURL;
}

//-----------------------------------------------------------------------------
// Purpose: If we were left clicked on, launch the URL
//-----------------------------------------------------------------------------
void URLLabel::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        if (!m_pszURL.IsEmpty())
		{
	        system()->ShellExecute("open", m_pszURL);
		}
    }
}

//-----------------------------------------------------------------------------
// Purpose: Applies resouce settings
//-----------------------------------------------------------------------------
void URLLabel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	const char *pszURL = inResourceData->GetString("URLText", nullptr);
	if (pszURL)
	{
		if (pszURL[0] == '#')
		{
			// it's a localized url, look it up
			const wchar_t *ws = g_pVGuiLocalize->Find(pszURL + 1);
			if (ws)
			{
				char localizedUrl[512];
				g_pVGuiLocalize->ConvertUnicodeToANSI(ws, localizedUrl, sizeof(localizedUrl));
				SetURL(localizedUrl);
			}
		}
		else
		{
	        SetURL(pszURL);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: saves them to disk
//-----------------------------------------------------------------------------
void URLLabel::GetSettings( KeyValues *outResourceData )
{
	BaseClass::GetSettings(outResourceData);

	outResourceData->SetString("URLText", m_pszURL);
}

//-----------------------------------------------------------------------------
// Purpose: scheme settings
//-----------------------------------------------------------------------------
void URLLabel::ApplySchemeSettings(IScheme *pScheme)
{
	// set our font to be underlined by default
	// the Label::ApplySchemeSettings() will override it if override set in scheme file
	SetFont( pScheme->GetFont( "DefaultUnderline", IsProportional() ) );
    BaseClass::ApplySchemeSettings(pScheme);
    SetCursor(dc_hand);
}

void URLLabel::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"URLText", TYPE_STRING}
    END_PANEL_SETTINGS();
}

