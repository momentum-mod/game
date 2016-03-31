#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include <math.h>
#include "vphysics_interface.h"
#include "mom_player_shared.h"
#include "hud_fillablebar.h"
#include "momentum/util/mom_util.h"

using namespace vgui;

static ConVar strafesync_draw("mom_showstrafesync", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Toggles displaying the strafesync data.\n", true, 0, true, 1);

static ConVar strafesync_colorize("mom_strafesync_colorize", "2", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Toggles strafesync data colorization type based on acceleration. 0 to disable\n", true, 0, true, 2);

//////////////////////////////////////////
//           CHudStrafeSyncDisplay        //
//////////////////////////////////////////
class CHudStrafeSyncDisplay : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudStrafeSyncDisplay, CHudNumericDisplay);

public:
    CHudStrafeSyncDisplay(const char *pElementName);
    void OnThink();
    bool ShouldDraw()
    {
        return strafesync_draw.GetBool() && CHudElement::ShouldDraw();
    }
    void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
        SetShouldDisplaySecondaryValue(true);
    }
    bool ShouldColorize()
    {
        return strafesync_colorize.GetInt() > 0;
    }
    void Paint();

private:
    float m_flNextColorizeCheck;
    float m_flLastStrafeSync;

    Color m_lastColor;
    Color m_currentColor;
    Color normalColor, increaseColor, decreaseColor;
};

DECLARE_HUDELEMENT(CHudStrafeSyncDisplay);

CHudStrafeSyncDisplay::CHudStrafeSyncDisplay(const char *pElementName) : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "CHudSyncMeter")
{
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    SetLabelText(L"Sync");
}
void CHudStrafeSyncDisplay::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    float clampedStrafeSync = clamp(pPlayer->m_flStrafeSync, 0, 100);

    switch (strafesync_colorize.GetInt())
    {
    case 1:
        if (m_flNextColorizeCheck <= gpGlobals->curtime)
        {
            if (m_flLastStrafeSync != 0)
            {
                m_currentColor = mom_UTIL.GetColorFromVariation(pPlayer->m_flStrafeSync - m_flLastStrafeSync, 1.0f, normalColor, increaseColor, decreaseColor);
                m_lastColor = m_currentColor;
            }
            else
            {
                m_currentColor = normalColor;
                m_lastColor = m_currentColor;
            }
            m_flLastStrafeSync = pPlayer->m_flStrafeSync;
            m_flNextColorizeCheck = gpGlobals->curtime + 0.1f; //we need to update color every 0.1 seconds
        }
        break;
    case 2:
        if (pPlayer->m_flStrafeSync > 90)
        {
            m_currentColor = increaseColor;
        }
        else if (pPlayer->m_flStrafeSync < 75)
        {
            m_currentColor = decreaseColor;
        }
        else
        {
            m_currentColor = normalColor;
        }
        break;
    case 0:
    default:
        SetFgColor(normalColor);
        m_currentColor = normalColor;
        break;
    }
    SetDisplayValue(clampedStrafeSync);
    SetSecondaryValue((clampedStrafeSync - Floor2Int(clampedStrafeSync)) * 100);
    SetFgColor(m_currentColor);

}
void CHudStrafeSyncDisplay::Paint()
{
    if (ShouldDraw())
        BaseClass::Paint();
}
//////////////////////////////////////////
//           CHudStrafeSyncBar          //
//////////////////////////////////////////
class CHudStrafeSyncBar : public CHudFillableBar
{
    DECLARE_CLASS_SIMPLE(CHudStrafeSyncBar, CHudFillableBar);
public:
    CHudStrafeSyncBar(const char *pElementName);
    void OnThink();
    void Paint();
    void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    }
    bool ShouldColorize()
    {
        return strafesync_colorize.GetInt() > 0;
    }

private:
    float m_flNextColorizeCheck;
    float m_flLastStrafeSync;

    Color m_lastColor;
    Color m_currentColor;
    Color normalColor, increaseColor, decreaseColor;
};

DECLARE_HUDELEMENT(CHudStrafeSyncBar);

CHudStrafeSyncBar::CHudStrafeSyncBar(const char *pElementName) : CHudFillableBar("CHudSyncBar")
{
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}
void CHudStrafeSyncBar::Paint()
{
    if (strafesync_draw.GetBool())
        BaseClass::Paint(m_currentColor);
}
void CHudStrafeSyncBar::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    switch (strafesync_colorize.GetInt())
    {
    case 1:
        if (m_flNextColorizeCheck <= gpGlobals->curtime)
        {
            if (m_flLastStrafeSync != 0)
            {
                m_currentColor = mom_UTIL.GetColorFromVariation(pPlayer->m_flStrafeSync - m_flLastStrafeSync, 1.0f, normalColor, increaseColor, decreaseColor);
                m_lastColor = m_currentColor;
            }
            else
            {
                m_currentColor = normalColor;
                m_lastColor = m_currentColor;
            }
            m_flLastStrafeSync = pPlayer->m_flStrafeSync;
            m_flNextColorizeCheck = gpGlobals->curtime + 0.1f; //we need to update color every 0.1 seconds
        }
        break;
    case 2:
        if (pPlayer->m_flStrafeSync > 90)
        {
            m_currentColor = increaseColor;
        }
        else if (pPlayer->m_flStrafeSync < 75)
        {
            m_currentColor = decreaseColor;
        }
        else
        {
            m_currentColor = normalColor;
        }
        break;
    case 0:
    default:
        SetFgColor(normalColor);
        m_currentColor = normalColor;
        break;
    }
    SetValue(pPlayer->m_flStrafeSync);
}
