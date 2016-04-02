#include "cbase.h"
#include "hud_fillablebar.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "mom_player_shared.h"
#include "momentum/util/mom_util.h"
#include "vphysics_interface.h"
#include <math.h>

using namespace vgui;

static ConVar strafesync_draw("mom_drawstrafesync", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                              "Toggles displaying the strafesync data.\n", true, 0, true, 1);

static ConVar strafesync_type(
    "mom_strafesync_type", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "1: Sync1 (perfect strafe ticks / total strafe ticks)\n 2: Sync2 (accel ticks / total strafe ticks)\n", true, 1,
    true, 2);

static ConVar strafesync_colorize("mom_strafesync_colorize", "2",
                                  FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                  "Toggles strafesync data colorization type based on acceleration. 0 to disable\n",
                                  true, 0, true, 2);

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
        C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return pPlayer && strafesync_draw.GetBool() && CHudElement::ShouldDraw() && pPlayer->m_bTimerIsRunning;
    }
    void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
        digit_xpos_initial = digit_xpos;
    }
    bool ShouldColorize() { return strafesync_colorize.GetInt() > 0; }
    void Paint();

  private:
    float m_flNextColorizeCheck;
    float m_flLastStrafeSync;

    float m_localStrafeSync;
    Color m_lastColor;
    Color m_currentColor;
    Color normalColor, increaseColor, decreaseColor;

    float digit_xpos_initial;
};

DECLARE_HUDELEMENT(CHudStrafeSyncDisplay);

CHudStrafeSyncDisplay::CHudStrafeSyncDisplay(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "CHudSyncMeter")
{
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    SetLabelText(L"Sync");
}
void CHudStrafeSyncDisplay::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (strafesync_type.GetInt() == 1) // sync1
        m_localStrafeSync = pPlayer->m_flStrafeSync;
    else if (strafesync_type.GetInt() == 2) // sync2
        m_localStrafeSync = pPlayer->m_flStrafeSync2;
    else
        m_localStrafeSync = 0;

    float clampedStrafeSync = clamp(m_localStrafeSync, 0, 100);

    switch (strafesync_colorize.GetInt())
    {
    case 1:
        if (m_flNextColorizeCheck <= gpGlobals->curtime)
        {
            if (m_flLastStrafeSync != 0)
            {
                m_currentColor = mom_UTIL.GetColorFromVariation(m_localStrafeSync - m_flLastStrafeSync, 1.0f,
                                                                normalColor, increaseColor, decreaseColor);
            }
            else
            {
                m_currentColor = normalColor;
            }
            m_lastColor = m_currentColor;
            m_flLastStrafeSync = m_localStrafeSync;
            m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
        }
        break;
    case 2:
        if (m_localStrafeSync == 0)
        {
            m_currentColor = normalColor;
        }
        else if (m_localStrafeSync > 90)
        {
            m_currentColor = increaseColor;
        }
        else if (m_localStrafeSync < 75)
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
        m_currentColor = normalColor;
        break;
    }
    SetDisplayValue(clampedStrafeSync);

    SetSecondaryValue((clampedStrafeSync - Floor2Int(clampedStrafeSync)) * 100);

    SetShouldDisplaySecondaryValue(clampedStrafeSync != 0 && clampedStrafeSync != 100);

    if (clampedStrafeSync == 0)
    {
        digit_xpos = GetWide() / 2 - 5;
    }
    else
    {
        digit_xpos = digit_xpos_initial;
    }

    m_PrimaryValueColor = m_SecondaryValueColor = m_currentColor;
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
    bool ShouldDraw()
    {
        C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return (pPlayer && strafesync_draw.GetBool() && CHudElement::ShouldDraw() && pPlayer->m_bTimerIsRunning);
    }
    void Paint();
    void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    }
    bool ShouldColorize() { return strafesync_colorize.GetInt() > 0; }

  private:
    float m_flNextColorizeCheck;
    float m_flLastStrafeSync;

    float m_localStrafeSync;
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
    if (ShouldDraw())
        BaseClass::Paint(m_currentColor);
}
void CHudStrafeSyncBar::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (strafesync_type.GetInt() == 1) // sync1
        m_localStrafeSync = pPlayer->m_flStrafeSync;
    else if (strafesync_type.GetInt() == 2) // sync2
        m_localStrafeSync = pPlayer->m_flStrafeSync2;
    switch (strafesync_colorize.GetInt())
    {
    case 1:
        if (m_flNextColorizeCheck <= gpGlobals->curtime)
        {
            if (m_flLastStrafeSync != 0)
            {
                m_currentColor = mom_UTIL.GetColorFromVariation(m_localStrafeSync - m_flLastStrafeSync, 1.0f,
                                                                normalColor, increaseColor, decreaseColor);
            }
            else
            {
                m_currentColor = normalColor;
            }
            m_lastColor = m_currentColor;
            m_flLastStrafeSync = m_localStrafeSync;
            m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
        }
        break;
    case 2:
        if (m_localStrafeSync == 0)
        {
            m_currentColor = normalColor;
        }
        if (m_localStrafeSync > 90)
        {
            m_currentColor = increaseColor;
        }
        else if (m_localStrafeSync < 75)
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
    SetValue(m_localStrafeSync);
}
