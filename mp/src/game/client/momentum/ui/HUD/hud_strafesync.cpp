#include "cbase.h"
#include "baseviewport.h"
#include "c_mom_replay_entity.h"
#include "hud_fillablebar.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "momentum/util/mom_util.h"
#include "vphysics_interface.h"
#include <math.h>

#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar strafesync_draw("mom_strafesync_draw", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                              "Toggles displaying the strafesync data. (1 = only timer , 2 = always (except practice mode)) \n",
                              true, 0, true, 2);

static ConVar strafesync_drawbar("mom_strafesync_drawbar", "1",
                                 FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                 "Toggles displaying the visual strafesync bar.\n", true, 0, true, 1);

static ConVar strafesync_type(
    "mom_strafesync_type", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "1: Sync1 (perfect strafe ticks / total strafe ticks)\n 2: Sync2 (accel ticks / total strafe ticks)\n", true, 1,
    true, 2);

static ConVar strafesync_colorize("mom_strafesync_colorize", "2",
                                  FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                  "Toggles strafesync data colorization type based on acceleration. 0 to disable\n",
                                  true, 0, true, 2);

#define SYNC_COLORIZE_DEADZONE 0
//////////////////////////////////////////
//           CHudStrafeSyncDisplay        //
//////////////////////////////////////////
class CHudStrafeSyncDisplay : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudStrafeSyncDisplay, CHudNumericDisplay);

    CHudStrafeSyncDisplay(const char *pElementName);
    void OnThink() OVERRIDE;
    bool ShouldDraw() OVERRIDE
    {
        IViewPortPanel *pLeaderboards = gViewPortInterface->FindPanelByName(PANEL_TIMES);
        if (pLeaderboards && pLeaderboards->IsVisible())
            return false;

        C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
        bool shouldDrawLocal = false;
        if (pPlayer)
        {
            if (pPlayer->IsWatchingReplay())
            {
                // MOM_TODO: Should we have a convar against this?
                C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
                shouldDrawLocal = pGhost->m_SrvData.m_RunData.m_bTimerRunning && !pGhost->m_SrvData.m_RunData.m_bMapFinished;
            }
            else
            {
                shouldDrawLocal = !pPlayer->m_SrvData.m_RunData.m_bMapFinished &&
                                  ((!pPlayer->m_SrvData.m_bHasPracticeMode &&
                                    strafesync_draw.GetInt() == 2) ||
                                   pPlayer->m_SrvData.m_RunData.m_bTimerRunning);
            }
        }
        return strafesync_draw.GetInt() && CHudElement::ShouldDraw() && shouldDrawLocal;
    }

    void Reset() OVERRIDE
    {
        m_flNextColorizeCheck = 0;
        m_flLastStrafeSync = 0;
        m_localStrafeSync = 0;
        m_lastColor = normalColor;
        m_currentColor = normalColor;
    }
    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
        SetBgColor(_bgColor);
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
        digit_xpos_initial = digit_xpos;
    }
    bool ShouldColorize() { return strafesync_colorize.GetInt() > 0; }
    void Paint() OVERRIDE;

  private:
    float m_flNextColorizeCheck;
    float m_flLastStrafeSync;

    float m_localStrafeSync;
    Color m_lastColor;
    Color m_currentColor;
    Color normalColor, increaseColor, decreaseColor;

    float digit_xpos_initial;

  protected:
    CPanelAnimationVar(Color, _bgColor, "BgColor", "Blank");
};

DECLARE_HUDELEMENT(CHudStrafeSyncDisplay);

CHudStrafeSyncDisplay::CHudStrafeSyncDisplay(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "CHudSyncMeter")
{
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}
void CHudStrafeSyncDisplay::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (!pPlayer)
        return;

    m_localStrafeSync = 0;

    C_MomentumReplayGhostEntity *pReplayEnt = pPlayer->GetReplayEnt();
    if (pReplayEnt)
    {
        if (strafesync_type.GetInt() == 1) // sync1
            m_localStrafeSync = pReplayEnt->m_SrvData.m_RunData.m_flStrafeSync;
        else if (strafesync_type.GetInt() == 2) // sync2
            m_localStrafeSync = pReplayEnt->m_SrvData.m_RunData.m_flStrafeSync2;
    }
    else
    {
        if (strafesync_type.GetInt() == 1) // sync1
            m_localStrafeSync = pPlayer->m_SrvData.m_RunData.m_flStrafeSync;
        else if (strafesync_type.GetInt() == 2) // sync2
            m_localStrafeSync = pPlayer->m_SrvData.m_RunData.m_flStrafeSync2;
    }

    float clampedStrafeSync = clamp(m_localStrafeSync, 0, 100);

    switch (strafesync_colorize.GetInt())
    {
    case 1:
        if (m_flNextColorizeCheck <= gpGlobals->curtime)
        {
            m_flLastStrafeSync != 0
                ? m_currentColor =
                      g_pMomentumUtil->GetColorFromVariation(m_localStrafeSync - m_flLastStrafeSync, SYNC_COLORIZE_DEADZONE,
                                                      normalColor, increaseColor, decreaseColor)
                : m_currentColor = normalColor;

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
        digit_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hNumberFont, "0") / 2;
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
    if (strafesync_type.GetInt() == 2)
    {
        SetLabelText(L"Sync 2");
    }
    else
    {
        SetLabelText(L"Sync");
    }
    text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, m_LabelText) / 2;
}
//////////////////////////////////////////
//           CHudStrafeSyncBar          //
//////////////////////////////////////////
class CHudStrafeSyncBar : public CHudElement, public CHudFillableBar
{
    DECLARE_CLASS_SIMPLE(CHudStrafeSyncBar, CHudFillableBar);

  public:
    CHudStrafeSyncBar(const char *pElementName);
    void OnThink() OVERRIDE;
    bool ShouldDraw() OVERRIDE
    {
        C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
        bool shouldDrawLocal = false;
        if (pPlayer)
        {
            if (pPlayer->IsWatchingReplay())
            {
                // MOM_TODO: Should we have a convar against this?
                C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
                shouldDrawLocal = pGhost->m_SrvData.m_RunData.m_bTimerRunning && !pGhost->m_SrvData.m_RunData.m_bMapFinished;
            }
            else
            {
                shouldDrawLocal = !pPlayer->m_SrvData.m_RunData.m_bMapFinished &&
                                  ((!pPlayer->m_SrvData.m_bHasPracticeMode &&
                                    strafesync_draw.GetInt() == 2) ||
                                   pPlayer->m_SrvData.m_RunData.m_bTimerRunning);
            }
        }
        return strafesync_drawbar.GetBool() && CHudElement::ShouldDraw() && shouldDrawLocal;
    }

    void Reset() OVERRIDE
    {
        m_flNextColorizeCheck = 0;
        m_flLastStrafeSync = 0;
        m_localStrafeSync = 0;
        m_lastColor = normalColor;
        m_currentColor = normalColor;
    }
    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("White", pScheme));
        normalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
        increaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
        decreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    }
    void Paint() OVERRIDE;
    bool ShouldColorize() { return strafesync_colorize.GetInt() > 0; }

  private:
    float m_flNextColorizeCheck;
    float m_flLastStrafeSync;

    float m_localStrafeSync;
    Color m_lastColor;
    Color m_currentColor;
    Color normalColor, increaseColor, decreaseColor;
};

DECLARE_NAMED_HUDELEMENT(CHudStrafeSyncBar, CHudSyncBar);

CHudStrafeSyncBar::CHudStrafeSyncBar(const char *pElementName) : CHudElement(pElementName), CHudFillableBar(g_pClientMode->GetViewport(), pElementName)
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
    if (pPlayer == nullptr)
        return;

    C_MomentumReplayGhostEntity *pReplayEnt = pPlayer->GetReplayEnt();
    if (pReplayEnt)
    {
        if (strafesync_type.GetInt() == 1) // sync1
            m_localStrafeSync = pReplayEnt->m_SrvData.m_RunData.m_flStrafeSync;
        else if (strafesync_type.GetInt() == 2) // sync2
            m_localStrafeSync = pReplayEnt->m_SrvData.m_RunData.m_flStrafeSync2;
    }
    else
    {
        if (strafesync_type.GetInt() == 1) // sync1
            m_localStrafeSync = pPlayer->m_SrvData.m_RunData.m_flStrafeSync;
        else if (strafesync_type.GetInt() == 2) // sync2
            m_localStrafeSync = pPlayer->m_SrvData.m_RunData.m_flStrafeSync2;
    }

    switch (strafesync_colorize.GetInt())
    {
    case 1:
        if (m_flNextColorizeCheck <= gpGlobals->curtime)
        {
            m_flLastStrafeSync != 0
                ? m_currentColor =
                      g_pMomentumUtil->GetColorFromVariation(m_localStrafeSync - m_flLastStrafeSync, SYNC_COLORIZE_DEADZONE,
                                                      normalColor, increaseColor, decreaseColor)
                : m_currentColor = normalColor;

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
