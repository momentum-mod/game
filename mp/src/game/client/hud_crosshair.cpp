//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_crosshair.h"
#include "iclientmode.h"
#include "view.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "ivrenderview.h"
#include "materialsystem/imaterialsystem.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "client_virtualreality.h"
#include "sourcevr/isourcevirtualreality.h"
#include "weapon/weapon_base.h"
#include "mom_player_shared.h"
#include "util/mom_util.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar crosshair( "crosshair", "1", FCVAR_ARCHIVE );
ConVar cl_observercrosshair( "cl_observercrosshair", "0", FCVAR_ARCHIVE );

MAKE_TOGGLE_CONVAR(cl_crosshair_alpha_enable, "1", FCVAR_ARCHIVE, "Toggle crosshair transparency. 0 = OFF, 1 = ON\n");

ConVar cl_crosshair_color("cl_crosshair_color", "FF0000FF", FCVAR_ARCHIVE,
    "Set the crosshair's color. Accepts HEX color value in format RRGGBBAA. if RRGGBB is supplied, Alpha is set to 0x4B.\n");

MAKE_TOGGLE_CONVAR(cl_crosshair_dot, "0", FCVAR_ARCHIVE, "Toggle crosshair dot. 0 = OFF, 1 = ON\n");

MAKE_TOGGLE_CONVAR(cl_crosshair_dynamic_fire, "1", FCVAR_ARCHIVE, "Toggle dynamic crosshair behaviour with weapon firing. 0 = OFF, 1 = ON\n");

MAKE_TOGGLE_CONVAR(cl_crosshair_dynamic_move, "1", FCVAR_ARCHIVE, "Toggle dynamic crosshair behaviour with player movement. 0 = OFF, 1 = ON\n");

ConVar cl_crosshair_file("cl_crosshair_file", "", FCVAR_ARCHIVE,
    "Set the name of the custom VTF texture defined in scripts/hud_textures.txt to be used as a crosshair. Takes effect on cl_crosshair_style 1.\n");

ConVar cl_crosshair_gap("cl_crosshair_gap", "4", FCVAR_ARCHIVE,
    "Set the minimum distance between two crosshair lines. Takes effect on cl_crosshair_style 1/2.\n", true, 0, false, 0); //could add cvar to split into horizontal and vertical

MAKE_TOGGLE_CONVAR(cl_crosshair_gap_use_weapon_value, "1", FCVAR_ARCHIVE, "Toggle using defined crosshair distances per weapon. 0 = OFF, 1 = ON\n");

MAKE_TOGGLE_CONVAR(cl_crosshair_outline_enable, "0", FCVAR_ARCHIVE, "Toggle using a black outline around the crosshair. Takes effect on cl_crosshair_style 0/1. 0 = OFF, 1 = ON\n");

ConVar cl_crosshair_outline_thickness("cl_crosshair_outline_thickness", "1", FCVAR_ARCHIVE,
    "Set the thickness of the crosshair's outline. Takes effect on cl_crosshair_outline_enable 1.\n", true, 0, false, 0);

ConVar cl_crosshair_scale("cl_crosshair_scale", "0", FCVAR_ARCHIVE,
    "Set the resolution to scale the crosshair to. Takes effect on cl_crosshair_style 0.\n", true, 0, false, 0);

MAKE_TOGGLE_CONVAR(cl_crosshair_scale_enable, "1", FCVAR_ARCHIVE, "Toggle scaling the crosshair to the resolution. Takes effect on cl_crosshair_style 0. 0 = OFF, 1 = ON\n");

ConVar cl_crosshair_size("cl_crosshair_size", "15", FCVAR_ARCHIVE,
    "Set the length of a crosshair line. Takes effect on cl_crosshair_style 1/2.\n", true, 0, false, 0);

ConVar cl_crosshair_style("cl_crosshair_style", "0", FCVAR_ARCHIVE,
    "Set crosshair style. 0 = CS:S, 1 = User CVars, 2 = Custom VTF\n", true, 0, true, 2);

MAKE_TOGGLE_CONVAR(cl_crosshair_t, "0", FCVAR_ARCHIVE, "Toggle T style crosshair. 0 = OFF, 1 = ON\n");

ConVar cl_crosshair_thickness("cl_crosshair_thickness", "1", FCVAR_ARCHIVE,
    "Set the thickness of a crosshair line. Takes effect on cl_crosshair_style 1.\n", true, 0, false, 0);

using namespace vgui;

bool ScreenTransform( const Vector& point, Vector& screen );
DECLARE_HUDELEMENT( CHudCrosshair );

CHudCrosshair::CHudCrosshair( const char *pElementName ) :
        CHudElement( pElementName ), BaseClass( NULL, "HudCrosshair" )
{
    vgui::Panel *pParent = g_pClientMode->GetViewport();
    SetParent( pParent );

    m_pCrosshair = 0;

    m_clrCrosshair = Color( 0, 0, 0, 0 );

    m_vecCrossHairOffsetAngle.Init();

    SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );
}

CHudCrosshair::~CHudCrosshair()
{
}

void CHudCrosshair::ApplySchemeSettings( IScheme *scheme )
{
    BaseClass::ApplySchemeSettings( scheme );

    m_pDefaultCrosshair = gHUD.GetIcon("crosshair_default");
    SetPaintBackgroundEnabled( false );

    SetSize( ScreenWidth(), ScreenHeight() );

    SetForceStereoRenderToFrameBuffer( true );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudCrosshair::ShouldDraw( void )
{
    bool bNeedsDraw;

    if ( m_bHideCrosshair )
        return false;

    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if ( !pPlayer )
        return false;

    C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
    if ( pWeapon && !pWeapon->ShouldDrawCrosshair() )
        return false;

#ifdef PORTAL
    C_Portal_Player *portalPlayer = ToPortalPlayer(pPlayer);
    if ( portalPlayer && portalPlayer->IsSuppressingCrosshair() )
        return false;
#endif // PORTAL

    /* disabled to avoid assuming it's an HL2 player.
    // suppress crosshair in zoom.
    if ( pPlayer->m_HL2Local.m_bZooming )
        return false;
    */

    // draw a crosshair only if alive or spectating in eye
    bNeedsDraw = m_pCrosshair && 
        crosshair.GetInt() &&
        !engine->IsDrawingLoadingImage() &&
        !engine->IsPaused() && 
        g_pClientMode->ShouldDrawCrosshair() &&
        !( pPlayer->GetFlags() & FL_FROZEN ) &&
        ( pPlayer->entindex() == render->GetViewEntity() ) &&
        !pPlayer->IsInVGuiInputMode() &&
        ( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );

    return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

#ifdef TF_CLIENT_DLL
extern ConVar cl_crosshair_red;
extern ConVar cl_crosshair_green;
extern ConVar cl_crosshair_blue;
extern ConVar cl_crosshair_scale;
#endif


void CHudCrosshair::GetDrawPosition ( float *pX, float *pY, bool *pbBehindCamera, QAngle angleCrosshairOffset )
{
    QAngle curViewAngles = CurrentViewAngles();
    Vector curViewOrigin = CurrentViewOrigin();

    int vx, vy, vw, vh;
    vgui::surface()->GetFullscreenViewport( vx, vy, vw, vh );

    float screenWidth = vw;
    float screenHeight = vh;

    float x = screenWidth / 2;
    float y = screenHeight / 2;

    bool bBehindCamera = false;

    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if ( ( pPlayer != NULL ) && ( pPlayer->GetObserverMode()==OBS_MODE_NONE ) )
    {
        bool bUseOffset = false;
        
        Vector vecStart;
        Vector vecEnd;

        if ( UseVR() )
        {
            // These are the correct values to use, but they lag the high-speed view data...
            vecStart = pPlayer->Weapon_ShootPosition();
            Vector vecAimDirection = pPlayer->GetAutoaimVector( 1.0f );
            // ...so in some aim modes, they get zapped by something completely up-to-date.
            g_ClientVirtualReality.OverrideWeaponHudAimVectors ( &vecStart, &vecAimDirection );
            vecEnd = vecStart + vecAimDirection * MAX_TRACE_LENGTH;

            bUseOffset = true;
        }

        if ( bUseOffset )
        {
            trace_t tr;
            UTIL_TraceLine( vecStart, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

            Vector screen;
            screen.Init();
            bBehindCamera = ScreenTransform(tr.endpos, screen) != 0;

            x = 0.5f * ( 1.0f + screen[0] ) * screenWidth + 0.5f;
            y = 0.5f * ( 1.0f - screen[1] ) * screenHeight + 0.5f;
        }
    }

    // MattB - angleCrosshairOffset is the autoaim angle.
    // if we're not using autoaim, just draw in the middle of the 
    // screen
    if ( angleCrosshairOffset != vec3_angle )
    {
        QAngle angles;
        Vector forward;
        Vector point, screen;

        // this code is wrong
        angles = curViewAngles + angleCrosshairOffset;
        AngleVectors( angles, &forward );
        VectorAdd( curViewOrigin, forward, point );
        ScreenTransform( point, screen );

        x += 0.5f * screen[0] * screenWidth + 0.5f;
        y += 0.5f * screen[1] * screenHeight + 0.5f;
    }

    *pX = x;
    *pY = y;
    *pbBehindCamera = bBehindCamera;
}

void CHudCrosshair::DrawCrosshair( CWeaponBase *weaponBase )
{
    if ( !IsCurrentViewAccessAllowed() )
        return;

    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();

    if (!pPlayer)
        return;

    // localplayer must be owner if not in Spec mode
    Assert((pPlayer == weaponBase->GetPlayerOwner()) || (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE));

    // Draw the targeting zone around the pCrosshair
    if (pPlayer->IsInVGuiInputMode())
        return;

    float x, y;
    bool bBehindCamera;
    GetDrawPosition(&x, &y, &bBehindCamera, m_vecCrossHairOffsetAngle);
    
    if( bBehindCamera )
        return;
    
    int iDistance = weaponBase->GetMomWpnData().m_iCrosshairMinDistance;        // The minimum distance the crosshair can achieve...
    int iDeltaDistance = weaponBase->GetMomWpnData().m_iCrosshairDeltaDistance; // Distance at which the crosshair shrinks at each step

    if (cl_crosshair_style.GetInt() != 0 && !cl_crosshair_gap_use_weapon_value.GetBool())
    {
        iDeltaDistance *= cl_crosshair_gap.GetInt() / iDistance; //scale delta to weapon's delta, could be a cvar
        iDistance = cl_crosshair_gap.GetInt();
    }

    if (cl_crosshair_dynamic_move.GetBool())
    {
        // min distance multiplied by constants
        if (!(pPlayer->GetFlags() & FL_ONGROUND))
            iDistance *= 2.0f;
        else if (pPlayer->GetFlags() & FL_DUCKING)
            iDistance *= 0.5f;
        else if (pPlayer->GetAbsVelocity().Length() > 100)
            iDistance *= 1.5f;
    }

    if (cl_crosshair_dynamic_fire.GetBool() && pPlayer->m_iShotsFired > weaponBase->m_iAmmoLastCheck) // shots firing
    {
        if (cl_crosshair_style.GetInt() == 0 || cl_crosshair_gap_use_weapon_value.GetBool())
            weaponBase->m_flCrosshairDistance = min(15, weaponBase->m_flCrosshairDistance + iDeltaDistance); // min of 15 (default crosshair size at 1080p) [but this is gap. not size] or (current distance) + delta
        else
            weaponBase->m_flCrosshairDistance = min(iDistance * 1.6, weaponBase->m_flCrosshairDistance + iDeltaDistance); //scale growth to crosshair gap, could be a cvar (1.6 is arbitrary)
    }
    else if (weaponBase->m_flCrosshairDistance > iDistance) // distance > min distance (defined at init or from if block above)
    {
        weaponBase->m_flCrosshairDistance -= 0.1f + weaponBase->m_flCrosshairDistance * 0.013; // decrease by 0.1 + 1.3% of current (decreases exponentially slow over time)
    }

    weaponBase->m_iAmmoLastCheck = pPlayer->m_iShotsFired;

    if (weaponBase->m_flCrosshairDistance < iDistance) //less than minimum/when m_flCrosshairDistance is initialized
        weaponBase->m_flCrosshairDistance = iDistance;

    // scale bar size to the resolution
    float scale;
    int iCrosshairDistance, iBarSize, iBarThickness;

    int vx, vy, vw, vh;
    vgui::surface()->GetFullscreenViewport( vx, vy, vw, vh );

    if (cl_crosshair_style.GetInt() == 0) //only CS:S uses crosshair scaling
    {
        int crosshairScale = cl_crosshair_scale.GetInt();

        if (crosshairScale < 1)
        {
            if (vh <= 600)
                crosshairScale = 600;
            else if (vh <= 768)
                crosshairScale = 768;
            else
                crosshairScale = 1200;
        }

        if (cl_crosshair_scale_enable.GetBool() == false)
            scale = 1.0f;
        else
            scale = float(vh) / float(crosshairScale);

        iCrosshairDistance = static_cast<int>(ceil(weaponBase->m_flCrosshairDistance * scale));
        iBarSize = XRES(5) + (iCrosshairDistance - iDistance) / 2;
        iBarSize = max(1, (int)((float)iBarSize * scale));
        iBarThickness = max(1, (int)floor(scale + 0.5f)); //thickness of 1 (or odd) causes off-center crosshairs
    }
    else //allow user cvars
    {
        scale = 1.0f;

        iCrosshairDistance = static_cast<int>(ceil(weaponBase->m_flCrosshairDistance * scale));
        iBarSize = cl_crosshair_size.GetInt();
        iBarThickness = cl_crosshair_thickness.GetInt(); //thickness of 1 (or odd) causes off-center crosshairs
    }
    
    Color crossColor;
    if (!MomUtil::GetColorFromHex(MomUtil::GetHexFromColor(cl_crosshair_color.GetString()), crossColor))
        crossColor.SetColor(255, 0, 0, 200);
    if (!cl_crosshair_alpha_enable.GetBool())
        crossColor.SetColor(crossColor.r(), crossColor.g(), crossColor.b(), 200);

    int iHalfScreenWidth = vw / 2;
    int iHalfScreenHeight = vh / 2;

    if (cl_crosshair_style.GetInt() != 2)
    {
        m_pCrosshair = gHUD.GetIcon("whiteAdditive");

        if (m_pCrosshair)
            weaponBase->m_iCrosshairTextureID = m_pCrosshair->textureId;
        
        int iLeft = iHalfScreenWidth - (iCrosshairDistance + iBarSize);
        int iRight = iHalfScreenWidth + iCrosshairDistance;
        int iFarLeft = iLeft + iBarSize;
        int iFarRight = iRight + iBarSize;

        int iTop = iHalfScreenHeight - (iCrosshairDistance + iBarSize);
        int iBottom = iHalfScreenHeight + iCrosshairDistance;
        int iFarTop = iTop + iBarSize;
        int iFarBottom = iBottom + iBarSize;

        int iHalfUpper = iHalfScreenHeight - iBarThickness / 2;
        int iHalfLower = iHalfScreenHeight + iBarThickness / 2 + iBarThickness % 2;
        int iHalfLefter = iHalfScreenWidth - iBarThickness / 2;
        int iHalfRighter = iHalfScreenWidth + iBarThickness / 2 + iBarThickness % 2;

        int iOutlineThickness = cl_crosshair_outline_thickness.GetInt();

        if (!cl_crosshair_alpha_enable.GetBool())
        {
            // Additive crosshair
            vgui::surface()->DrawSetTexture(weaponBase->m_iCrosshairTextureID);
            
            if (cl_crosshair_outline_enable.GetBool())
            {
                vgui::surface()->DrawSetColor(0, 0, 0, 200); // could add cvar for outline color/alpha
                vgui::surface()->DrawFilledRect(iLeft - iOutlineThickness, iHalfUpper - iOutlineThickness, iFarLeft + iOutlineThickness, iHalfLower + iOutlineThickness);
                vgui::surface()->DrawFilledRect(iRight - iOutlineThickness, iHalfUpper - iOutlineThickness, iFarRight + iOutlineThickness, iHalfLower + iOutlineThickness);

                if (!cl_crosshair_t.GetBool())
                    vgui::surface()->DrawFilledRect(iHalfLefter - iOutlineThickness, iTop - iOutlineThickness, iHalfRighter + iOutlineThickness, iFarTop + iOutlineThickness);
                vgui::surface()->DrawFilledRect(iHalfLefter - iOutlineThickness, iBottom - iOutlineThickness, iHalfRighter + iOutlineThickness, iFarBottom + iOutlineThickness);
            } // DrawTexturedRect has an alpha despite using 255 alpha, use DrawFilledRect

            vgui::surface()->DrawSetColor(crossColor);
            vgui::surface()->DrawTexturedRect(iLeft, iHalfUpper, iFarLeft, iHalfLower);
            vgui::surface()->DrawTexturedRect(iRight, iHalfUpper, iFarRight, iHalfLower);

            if (!cl_crosshair_t.GetBool())
                vgui::surface()->DrawTexturedRect(iHalfLefter, iTop, iHalfRighter, iFarTop);
            vgui::surface()->DrawTexturedRect(iHalfLefter, iBottom, iHalfRighter, iFarBottom);
            
            if (cl_crosshair_dot.GetBool())
            {
                if (cl_crosshair_outline_enable.GetBool())
                {
                    vgui::surface()->DrawSetColor(0, 0, 0, 200); // could add cvar for (dot) outline color/alpha
                    vgui::surface()->DrawFilledRect(iHalfScreenWidth - (1 + iOutlineThickness), iHalfScreenHeight - (1 + iOutlineThickness), iHalfScreenWidth + 1 + iOutlineThickness, iHalfScreenHeight + 1 + iOutlineThickness);
                } // DrawTexturedRect has an alpha despite using 255 alpha, use DrawFilledRect

                vgui::surface()->DrawSetColor(crossColor);
                vgui::surface()->DrawTexturedRect(iHalfScreenWidth - 1, iHalfScreenHeight - 1, iHalfScreenWidth + 1, iHalfScreenHeight + 1); // could add cvar for dot size
            }
        }
        else
        {
            // Alpha-blended crosshair
            if (cl_crosshair_outline_enable.GetBool())
            {
                vgui::surface()->DrawSetColor(0, 0, 0, crossColor.a()); // could add cvar for outline color/alpha
                vgui::surface()->DrawFilledRect(iLeft - iOutlineThickness, iHalfUpper - iOutlineThickness, iFarLeft + iOutlineThickness, iHalfLower + iOutlineThickness);
                vgui::surface()->DrawFilledRect(iRight - iOutlineThickness, iHalfUpper - iOutlineThickness, iFarRight + iOutlineThickness, iHalfLower + iOutlineThickness);

                if (!cl_crosshair_t.GetBool())
                    vgui::surface()->DrawFilledRect(iHalfLefter - iOutlineThickness, iTop - iOutlineThickness, iHalfRighter + iOutlineThickness, iFarTop + iOutlineThickness);
                vgui::surface()->DrawFilledRect(iHalfLefter - iOutlineThickness, iBottom - iOutlineThickness, iHalfRighter + iOutlineThickness, iFarBottom + iOutlineThickness);
            }

            vgui::surface()->DrawSetColor(crossColor);
            vgui::surface()->DrawFilledRect(iLeft, iHalfUpper, iFarLeft, iHalfLower);
            vgui::surface()->DrawFilledRect(iRight, iHalfUpper, iFarRight, iHalfLower);
            
            if (!cl_crosshair_t.GetBool())
                vgui::surface()->DrawFilledRect(iHalfLefter, iTop, iHalfRighter, iFarTop);
            vgui::surface()->DrawFilledRect(iHalfLefter, iBottom, iHalfRighter, iFarBottom);

            if (cl_crosshair_dot.GetBool())
            {
                if (cl_crosshair_outline_enable.GetBool())
                {
                    vgui::surface()->DrawSetColor(0, 0, 0, crossColor.a()); // could add cvar for (dot) outline color/alpha
                    vgui::surface()->DrawFilledRect(iHalfScreenWidth - (1 + iOutlineThickness), iHalfScreenHeight - (1 + iOutlineThickness), iHalfScreenWidth + 1 + iOutlineThickness, iHalfScreenHeight + 1 + iOutlineThickness);
                }

                vgui::surface()->DrawSetColor(crossColor); // could add cvar for dot color/alpha
                vgui::surface()->DrawFilledRect(iHalfScreenWidth - 1, iHalfScreenHeight - 1, iHalfScreenWidth + 1, iHalfScreenHeight + 1); // could add cvar for dot size
            }
        }
    }
    else
    {
        m_pCrosshair = nullptr;

        if (FStrEq(cl_crosshair_file.GetString(), "") || FStrEq(cl_crosshair_file.GetString(), "null"))
            m_pCrosshair = gHUD.GetIcon("whiteAdditive");
        else
            m_pCrosshair = gHUD.GetIcon(cl_crosshair_file.GetString());

        if (m_pCrosshair)
            weaponBase->m_iCrosshairTextureID = m_pCrosshair->textureId;

        vgui::surface()->DrawSetTexture(weaponBase->m_iCrosshairTextureID);

        // make sure dynamic behaviour is ok
        int iLeft = iHalfScreenWidth - (iBarSize + iCrosshairDistance) / 2;
        int iTop = iHalfScreenHeight - (iBarSize + iCrosshairDistance) / 2;
        int iRight = iHalfScreenWidth + (iBarSize + iCrosshairDistance) / 2 + (iBarSize + iCrosshairDistance) % 2;
        int iBottom = iHalfScreenHeight + (iBarSize + iCrosshairDistance) / 2 + (iBarSize + iCrosshairDistance) % 2;

        vgui::surface()->DrawTexturedRect(iLeft, iTop, iRight, iBottom);
    }
}

void CHudCrosshair::Paint( void )
{
    if ( !m_pCrosshair )
        return;

    if ( !IsCurrentViewAccessAllowed() )
        return;

    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if ( !pPlayer )
        return;

    float x, y;
    bool bBehindCamera;
    GetDrawPosition ( &x, &y, &bBehindCamera, m_vecCrossHairOffsetAngle );

    if( bBehindCamera )
        return;

    float flWeaponScale = 1.f;
    int iTextureW = m_pCrosshair->Width();
    int iTextureH = m_pCrosshair->Height();
    C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
    if ( pWeapon )
    {
        pWeapon->GetWeaponCrosshairScale( flWeaponScale );
    }

    float flPlayerScale = 1.0f;
#ifdef TF_CLIENT_DLL
    Color clr( cl_crosshair_red.GetInt(), cl_crosshair_green.GetInt(), cl_crosshair_blue.GetInt(), 255 );
    flPlayerScale = cl_crosshair_scale.GetFloat() / 32.0f;  // the player can change the scale in the options/multiplayer tab
#else
    Color clr = m_clrCrosshair;
#endif
    float flWidth = flWeaponScale * flPlayerScale * (float)iTextureW;
    float flHeight = flWeaponScale * flPlayerScale * (float)iTextureH;
    int iWidth = (int)( flWidth + 0.5f );
    int iHeight = (int)( flHeight + 0.5f );
    int iX = (int)( x + 0.5f );
    int iY = (int)( y + 0.5f );

    m_pCrosshair->DrawSelfCropped (
        iX-(iWidth/2), iY-(iHeight/2),
        0, 0,
        iTextureW, iTextureH,
        iWidth, iHeight,
        clr );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshairAngle( const QAngle& angle )
{
    VectorCopy( angle, m_vecCrossHairOffsetAngle );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshair( CHudTexture *texture, const Color& clr )
{
    m_pCrosshair = texture;
    m_clrCrosshair = clr;
}

//-----------------------------------------------------------------------------
// Purpose: Resets the crosshair back to the default
//-----------------------------------------------------------------------------
void CHudCrosshair::ResetCrosshair()
{
    SetCrosshair( m_pDefaultCrosshair, Color(255, 255, 255, 255) );
}
