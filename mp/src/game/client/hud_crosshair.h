//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

#include "hudelement.h"
#include <vgui_controls/Panel.h>

class C_WeaponBase;

namespace vgui
{
    class IScheme;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudCrosshair : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( CHudCrosshair, vgui::Panel );
public:
    CHudCrosshair( const char *pElementName );
    virtual ~CHudCrosshair();

    virtual void	SetCrosshairAngle( const QAngle& angle );
    virtual void	SetCrosshair( CHudTexture *texture, const Color& clr );
    virtual void	ResetCrosshair();
    virtual void	DrawCrosshair( C_WeaponBase *weaponBase, int iHalfScreenWidth, int iHalfScreenHeight );
    virtual bool	HasCrosshair( void ) { return ( m_pCrosshair != NULL ); }
    virtual bool	ShouldDraw();

    // any UI element that wants to be at the aim point can use this to figure out where to draw
    static void	GetDrawPosition ( float *pX, float *pY, bool *pbBehindCamera, QAngle angleCrosshairOffset = vec3_angle );
protected:
    virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
    virtual void	Paint();

    // Crosshair sprite and colors
    CHudTexture		*m_pCrosshair;
    CHudTexture		*m_pDefaultCrosshair;
    Color			m_clrCrosshair;
    QAngle			m_vecCrossHairOffsetAngle;

    CPanelAnimationVar( bool, m_bHideCrosshair, "never_draw", "false" );
};


// Enable/disable crosshair rendering.
extern ConVar crosshair;
