//========= Copyright 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Hud element that indicates the direction of damage taken by the player
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include "ClientEffectPrecacheSystem.h"
#include "hud.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "ieffects.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMesh.h"
#include "materialsystem/imaterialvar.h"
#include "vguimatsurface/IMatSystemSurface.h"
#include "view.h"
#include "util/mom_util.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Convars for customizing damage indicator
static ConVar enabled("mom_hud_damageindicator", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                      "Enable or disable damage indicator.\n");
static ConVar minwidth("mom_hud_damageindicator_minwidth", "20", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                       "Minimum width of the damage indicator.\n");
static ConVar maxwidth("mom_hud_damageindicator_maxwidth", "20", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                       "Minimum width of the damage indicator \n");
static ConVar minheight("mom_hud_damageindicator_minheight", "60", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                        "Minimum height of the damage indicator.\n");
static ConVar maxheight("mom_hud_damageindicator_maxheight", "120", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                        "Minimum height of the damage indicator \n");
static ConVar radius("mom_hud_damageindicator_radius", "120", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                     "How far away the damage indicators are from the crosshair.\n");
static ConVar lifetime("mom_hud_damageindicator_lifetime", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                       "How far away the damage indicators are from the crosshair.\n");

static ConVar dmgcolor("mom_hud_damageindicator_color", "980000", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                       "Color of the damage indicator.\n");


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CHudDamageIndicator : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudDamageIndicator, vgui::Panel);

  public:
    CHudDamageIndicator(const char *pElementName);
    void Init(void);
    void VidInit(void);
    void Reset(void);
    virtual bool ShouldDraw(void);

    // Handler for our message
    void MsgFunc_DamageIndicator(bf_read &msg);

  private:
    virtual void OnThink();
    virtual void Paint();
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

    // Painting
    void GetDamagePosition(const Vector &vecDelta, float flRadius, float *xpos, float *ypos, float *flRotation);
    void DrawDamageIndicator(int x0, int y0, int x1, int y1, float alpha, float flRotation);

  private:
    // Indication times
    CPanelAnimationVar(float, m_iMaximumDamage, "MaximumDamage", "100");
    CPanelAnimationVar(float, m_flTravelTime, "TravelTime", ".1");
    CPanelAnimationVar(float, m_flFadeOutPercentage, "FadeOutPercentage", "0.7");

    float m_flMinimumWidth;
    float m_flMaximumWidth;
    float m_flMinimumHeight;
    float m_flMaximumHeight;
    float m_flRadius;
    float m_flMinimumTime;
    float m_flMaximumTime;

    // List of damages we've taken
    struct damage_t
    {
        int iScale;
        float flLifeTime;
        float flStartTime;
        Vector vecDelta; // Damage origin relative to the player
    };
    CUtlVector<damage_t> m_vecDamages;

    CMaterialReference m_WhiteAdditiveMaterial;
};

DECLARE_HUDELEMENT(CHudDamageIndicator);
DECLARE_HUD_MESSAGE(CHudDamageIndicator, DamageIndicator);

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CHudDamageIndicator::CHudDamageIndicator(const char *pElementName)
    : CHudElement(pElementName), BaseClass(NULL, "HudDamageIndicator")
{
    vgui::Panel *pParent = g_pClientMode->GetViewport();
    SetParent(pParent);

    m_WhiteAdditiveMaterial.Init("vgui/damage", TEXTURE_GROUP_VGUI);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Init(void)
{
    HOOK_HUD_MESSAGE(CHudDamageIndicator, DamageIndicator);
    Reset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Reset(void) { m_vecDamages.Purge(); }

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudDamageIndicator::VidInit(void) {}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudDamageIndicator::OnThink() {}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHudDamageIndicator::ShouldDraw(void)
{
    if (!CHudElement::ShouldDraw() || !enabled.GetBool())
        return false;

    // Don't draw if we don't have any damage to indicate
    if (!m_vecDamages.Count())
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Convert a damage position in world units to the screen's units
//-----------------------------------------------------------------------------
void CHudDamageIndicator::GetDamagePosition(const Vector &vecDelta, float flRadius, float *xpos, float *ypos,
                                            float *flRotation)
{
    // Player Data
    Vector playerPosition = MainViewOrigin();
    QAngle playerAngles = MainViewAngles();

    Vector forward, right, up(0, 0, 1);
    AngleVectors(playerAngles, &forward, NULL, NULL);
    forward.z = 0;
    VectorNormalize(forward);
    CrossProduct(up, forward, right);
    float front = DotProduct(vecDelta, forward);
    float side = DotProduct(vecDelta, right);
    *xpos = flRadius * -side;
    *ypos = flRadius * -front;

    // Get the rotation (yaw)
    *flRotation = atan2(*xpos, *ypos) + M_PI;
    *flRotation *= 180 / M_PI;

    float yawRadians = -(*flRotation) * M_PI / 180.0f;
    float ca = cos(yawRadians);
    float sa = sin(yawRadians);

    // Rotate it around the circle
    *xpos = (int)((ScreenWidth() / 2) + (flRadius * sa));
    *ypos = (int)((ScreenHeight() / 2) - (flRadius * ca));
}

//-----------------------------------------------------------------------------
// Purpose: Draw a single damage indicator
//-----------------------------------------------------------------------------
void CHudDamageIndicator::DrawDamageIndicator(int x0, int y0, int x1, int y1, float alpha, float flRotation)
{
    CMatRenderContextPtr pRenderContext(materials);
    IMesh *pMesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, m_WhiteAdditiveMaterial);

    // Get the corners, since they're being rotated
    int wide = x1 - x0;
    int tall = y1 - y0;
    Vector2D vecCorners[4];
    Vector2D center(x0 + (wide * 0.5f), y0 + (tall * 0.5f));
    float yawRadians = -flRotation * M_PI / 180.0f;
    Vector2D axis[2];
    axis[0].x = cos(yawRadians);
    axis[0].y = sin(yawRadians);
    axis[1].x = -axis[0].y;
    axis[1].y = axis[0].x;
    Vector2DMA(center, -0.5f * wide, axis[0], vecCorners[0]);
    Vector2DMA(vecCorners[0], -0.5f * tall, axis[1], vecCorners[0]);
    Vector2DMA(vecCorners[0], wide, axis[0], vecCorners[1]);
    Vector2DMA(vecCorners[1], tall, axis[1], vecCorners[2]);
    Vector2DMA(vecCorners[0], tall, axis[1], vecCorners[3]);

    // Draw the sucker
    CMeshBuilder meshBuilder;
    meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

    int iAlpha = alpha * 255;
    Color IndicatorColor;
    MomUtil::GetColorFromHex(dmgcolor.GetString(), IndicatorColor);
    int iIndicatorRed = IndicatorColor.r();
    int iIndicatorGreen = IndicatorColor.g();
    int iIndicatorBlue = IndicatorColor.b();

    meshBuilder.Color4ub(iIndicatorRed, iIndicatorGreen, iIndicatorBlue, iAlpha);
    meshBuilder.TexCoord2f(0, 0, 0);
    meshBuilder.Position3f(vecCorners[0].x, vecCorners[0].y, 0);
    meshBuilder.AdvanceVertex();

    meshBuilder.Color4ub(iIndicatorRed, iIndicatorGreen, iIndicatorBlue, iAlpha);
    meshBuilder.TexCoord2f(0, 1, 0);
    meshBuilder.Position3f(vecCorners[1].x, vecCorners[1].y, 0);
    meshBuilder.AdvanceVertex();

    meshBuilder.Color4ub(iIndicatorRed, iIndicatorGreen, iIndicatorBlue, iAlpha);
    meshBuilder.TexCoord2f(0, 1, 1);
    meshBuilder.Position3f(vecCorners[2].x, vecCorners[2].y, 0);
    meshBuilder.AdvanceVertex();

    meshBuilder.Color4ub(iIndicatorRed, iIndicatorGreen, iIndicatorBlue, iAlpha);
    meshBuilder.TexCoord2f(0, 0, 1);
    meshBuilder.Position3f(vecCorners[3].x, vecCorners[3].y, 0);
    meshBuilder.AdvanceVertex();

    meshBuilder.End();
    pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Paint()
{
    // Iterate backwards, because we might remove them as we go
    int iSize = m_vecDamages.Count();
    for (int i = iSize - 1; i >= 0; i--)
    {
        // Scale size to the damage
        int clampedDamage = clamp(m_vecDamages[i].iScale, 0, m_iMaximumDamage);

        int iWidth = RemapVal(clampedDamage, 0, m_iMaximumDamage, m_flMinimumWidth, m_flMaximumWidth) * 0.5;
        int iHeight = RemapVal(clampedDamage, 0, m_iMaximumDamage, m_flMinimumHeight, m_flMaximumHeight) * 0.5;

        // Find the place to draw it
        float xpos, ypos;
        float flRotation;
        float flTimeSinceStart = (gpGlobals->curtime - m_vecDamages[i].flStartTime);
        GetDamagePosition(m_vecDamages[i].vecDelta, m_flRadius, &xpos, &ypos, &flRotation);

        // Calculate life left
        float flLifeLeft = (m_vecDamages[i].flLifeTime - gpGlobals->curtime);
        if (flLifeLeft > 0)
        {
            float flPercent = flTimeSinceStart / (m_vecDamages[i].flLifeTime - m_vecDamages[i].flStartTime);
            float alpha;
            if (flPercent <= m_flFadeOutPercentage)
            {
                alpha = 1.0;
            }
            else
            {
                alpha = 1.0 - RemapVal(flPercent, m_flFadeOutPercentage, 1.0, 0.0, 1.0);
            }
            DrawDamageIndicator(xpos - iWidth, ypos - iHeight, xpos + iWidth, ypos + iHeight, alpha, flRotation);
        }
        else
        {
            m_vecDamages.Remove(i);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudDamageIndicator::MsgFunc_DamageIndicator(bf_read &msg)
{
    // Update the convars
    m_flMinimumWidth = maxwidth.GetFloat();
    m_flMaximumWidth = maxwidth.GetFloat();
    m_flMinimumHeight = minheight.GetFloat();
    m_flMaximumHeight = maxheight.GetFloat();
    m_flRadius = radius.GetFloat();
    m_flMinimumTime = lifetime.GetFloat();
    m_flMaximumTime = lifetime.GetFloat();

    damage_t damage;
    damage.iScale = msg.ReadByte();
    if (damage.iScale > m_iMaximumDamage)
    {
        damage.iScale = m_iMaximumDamage;
    }
    Vector vecOrigin;
    msg.ReadBitVec3Coord(vecOrigin);
    damage.flStartTime = gpGlobals->curtime;
    damage.flLifeTime =
        gpGlobals->curtime + RemapVal(damage.iScale, 0, m_iMaximumDamage, m_flMinimumTime, m_flMaximumTime);
    if (vecOrigin == vec3_origin)
    {
        vecOrigin = MainViewOrigin();
    }

    damage.vecDelta = (vecOrigin - MainViewOrigin());
    VectorNormalize(damage.vecDelta);

    m_vecDamages.AddToTail(damage);
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudDamageIndicator::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetPaintBackgroundEnabled(false);

    // set our size
    int screenWide, screenTall;
    int x, y;
    GetPos(x, y);
    GetHudSize(screenWide, screenTall);
    SetBounds(0, y, screenWide, screenTall - y);
}
