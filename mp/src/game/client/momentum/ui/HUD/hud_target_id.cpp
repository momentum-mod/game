//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"

#include "mom_player_shared.h"
#include "c_mom_online_ghost.h"
#include "c_mom_replay_entity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_showtargetid( "hud_showtargetid", "1" );

using namespace vgui;

class CTargetID : public CHudElement, public Panel
{

	DECLARE_CLASS_SIMPLE(CTargetID, Panel);

public:
	CTargetID(const char *pElementName);

	void Init(void) OVERRIDE;
    bool ShouldDraw() OVERRIDE{ return CHudElement::ShouldDraw(); }

	void ApplySchemeSettings( vgui::IScheme *scheme ) OVERRIDE;
	void Paint(void) OVERRIDE;
	void VidInit( void ) OVERRIDE;

protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");
    CPanelAnimationVar(int, m_ypos, "text_ypos", "200");

private:
	int				m_iLastEntIndex;
	float			m_flLastChangeTime;
};

DECLARE_HUDELEMENT( CTargetID );

CTargetID::CTargetID( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "CTargetID" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

    m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void CTargetID::Init( void )
{
};

void CTargetID::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CTargetID::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}
//-----------------------------------------------------------------------------
// Purpose: Draw function for the element
//-----------------------------------------------------------------------------
void CTargetID::Paint()
{
#define MAX_ID_STRING 256
	wchar_t sIDString[ MAX_ID_STRING ];
	sIDString[0] = 0;

    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());

	if ( !pPlayer )
		return;

	// Get our target's ent index
	int iEntIndex = pPlayer->GetIDTarget();
	// Didn't find one?
	if ( !iEntIndex )
	{
		// Check to see if we should clear our ID
		if ( m_flLastChangeTime && (gpGlobals->curtime > (m_flLastChangeTime + 0.5)) )
		{
			m_flLastChangeTime = 0;
			sIDString[0] = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	if ( iEntIndex )
	{
        C_BaseEntity *pEnt = cl_entitylist->GetEnt(iEntIndex);

		wchar_t wszGhostName[ MAX_PLAYER_NAME_LENGTH ];
        bool bShowGhostName = false;
		// Some entities we always want to check, cause the text may change
		// even while we're looking at it

        C_MomentumGhostBaseEntity *pGhost = dynamic_cast<C_MomentumGhostBaseEntity*>(pEnt);

        // Stop crashing when it's not a ghost.
        if (!pGhost) return;

        if (pGhost->IsOnlineGhost())
        {
            C_MomentumOnlineGhostEntity *pOnlineGhost = dynamic_cast<C_MomentumOnlineGhostEntity*>(pGhost);
            bShowGhostName = true;
            g_pVGuiLocalize->ConvertANSIToUnicode(pOnlineGhost->m_pszGhostName, wszGhostName, sizeof(wszGhostName));

        }
        else if (pGhost->IsReplayGhost())
        {
            //C_MomentumReplayGhostEntity *pReplayGhost = dynamic_cast<C_MomentumReplayGhostEntity*>(pGhost);
            // .. do stuff here?
        }
        if (bShowGhostName)
        {
            g_pVGuiLocalize->ConstructString(sIDString, sizeof(sIDString), L"%s1", 1, wszGhostName);
        }
        else
        {
            g_pVGuiLocalize->ConstructString(sIDString, sizeof(sIDString), L"%s1", 0);
        }

		if ( sIDString[0] )
		{
            int wide = 0, tall = 0;
            surface()->GetTextSize(m_hTextFont, sIDString, wide, tall);
            int xpos = (ScreenWidth() - wide) / 2;
            int ypos = YRES(m_ypos);

            SetSize(wide + 10, tall + 10); //padding

            SetPos(xpos, ypos);
            wide = UTIL_ComputeStringWidth(m_hTextFont, sIDString);

            surface()->DrawSetTextFont(m_hTextFont);
            surface()->DrawSetTextPos(0, 0);
			surface()->DrawSetTextColor( Color(255,255,255,200) );
			surface()->DrawPrintText( sIDString, wcslen(sIDString) );
		}
	}
}
