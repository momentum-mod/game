#include "cbase.h"

#include "baseviewport.h"
#include "hud_comparisons.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>

#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar mom_timer("mom_timer", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
                        "Toggle displaying the timer. 0 = OFF, 1 = ON\n", true, 0, true, 1);

static ConVar timer_mode("mom_timer_mode", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                         "Set what type of timer you want.\n0 = Generic Timer (no splits)\n1 = Splits by Checkpoint\n");

class C_HudTimer : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(C_HudTimer, Panel);
    C_HudTimer(const char *pElementName);
    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void Paint() OVERRIDE;
    bool ShouldDraw() OVERRIDE
    {
        IViewPortPanel *pLeaderboards = gViewPortInterface->FindPanelByName(PANEL_TIMES);
        return mom_timer.GetBool() && CHudElement::ShouldDraw() && pLeaderboards && !pLeaderboards->IsVisible();
    }

    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
        m_TimeGain = GetSchemeColor("MOM.Timer.Gain", pScheme);
        m_TimeLoss = GetSchemeColor("MOM.Timer.Loss", pScheme);
    }
    void MsgFunc_Timer_State(bf_read &msg);
    void MsgFunc_Timer_Reset(bf_read &msg);
    float GetCurrentTime();
    bool m_bIsRunning;
    bool m_bTimerRan; // MOM_TODO: What is this used for?

    void OnSavelocUpdateEvent(KeyValues *pKv);

  protected:
    CPanelAnimationVar(float, m_flBlur, "Blur", "0");
    CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");
    CPanelAnimationVar(Color, m_Ammo2Color, "Ammo2Color", "FgColor");
    CPanelAnimationVar(Color, m_TimeGain, "TimeGainColor", "FgColor");
    CPanelAnimationVar(Color, m_TimeLoss, "TimeLossColor", "FgColor");

    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "HudHintTextLarge");
    CPanelAnimationVar(HFont, m_hTimerFont, "TimerFont", "HudNumbersSmallBold");
    CPanelAnimationVar(HFont, m_hSmallTextFont, "SmallTextFont", "HudNumbersSmall");

    CPanelAnimationVarAliasType(bool, center_time, "centerTime", "0", "BOOL");
    CPanelAnimationVarAliasType(int, time_xpos, "time_xpos", "0", "proportional_xpos");
    CPanelAnimationVarAliasType(int, time_ypos, "time_ypos", "2", "proportional_ypos");
    CPanelAnimationVarAliasType(bool, center_cps, "centerCps", "1", "BOOL");
    CPanelAnimationVarAliasType(int, cps_xpos, "cps_xpos", "50", "proportional_xpos");
    CPanelAnimationVarAliasType(int, cps_ypos, "cps_ypos", "19", "proportional_ypos");
    CPanelAnimationVarAliasType(bool, center_split, "centerSplit", "1", "BOOL");
    CPanelAnimationVarAliasType(int, split_xpos, "split_xpos", "50", "proportional_xpos");
    CPanelAnimationVarAliasType(int, split_ypos, "split_ypos", "19", "proportional_ypos");

  private:
    int m_iZoneCurrent, m_iZoneCount;
    int m_iTotalTicks, m_iStartTick, m_G_iStartTickD, m_G_iCurrentTick, m_iOldTickCount;
    int initialTall;
    bool m_bIsReplay;
    // float m_fCurrentTime;

    wchar_t m_pwCurrentTime[BUFSIZETIME];
    char m_pszString[BUFSIZETIME];
    wchar_t m_pwCurrentCheckpoints[BUFSIZELOCL];
    char m_pszStringCps[BUFSIZELOCL];
    wchar_t m_pwCurrentStages[BUFSIZELOCL];
    char m_pszStringStages[BUFSIZELOCL];
    wchar_t m_pwCurrentStatus[BUFSIZELOCL];
    char m_pszStringStatus[BUFSIZELOCL];
    wchar_t m_pwStageTime[BUFSIZETIME];
    char m_pszStageTimeString[BUFSIZETIME];
    wchar_t m_pwStageTimeLabel[BUFSIZELOCL];
    char m_pszStageTimeLabelString[BUFSIZELOCL];

    wchar_t m_pwStageTimeComparison[BUFSIZETIME];
    char m_pszStageTimeComparisonANSI[BUFSIZETIME], m_pszStageTimeComparisonLabel[BUFSIZELOCL];

    wchar_t m_pwStageStartString[BUFSIZELOCL], m_pwStageStartLabel[BUFSIZELOCL];

    bool m_bPlayerInZone;
    bool m_bWereCheatsActivated;
    bool m_bPlayerHasPracticeMode;
    bool m_bShowSavelocs;
    bool m_bPlayerUsingSavelocMenu;
    bool m_bMapFinished;
    bool m_bMapIsLinear;
    int m_iSavelocCount, m_iSavelocCurrent, m_iPlayerSavelocCurrent, m_iPlayerSavelocCount;
    CMomRunStats *m_pRunStats;
    char stLocalized[BUFSIZELOCL], cpLocalized[BUFSIZELOCL], linearLocalized[BUFSIZELOCL],
        startZoneLocalized[BUFSIZELOCL], mapFinishedLocalized[BUFSIZELOCL], practiceModeLocalized[BUFSIZELOCL],
        noTimerLocalized[BUFSIZELOCL], savelocLocalized[BUFSIZELOCL];
};

DECLARE_HUDELEMENT(C_HudTimer);
DECLARE_HUD_MESSAGE(C_HudTimer, Timer_State);
DECLARE_HUD_MESSAGE(C_HudTimer, Timer_Reset);

C_HudTimer::C_HudTimer(const char *pElementName)
    : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "HudTimer")
{
    // This is already set for HUD elements, but still...
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    m_bIsReplay = false;

    g_pModuleComms->ListenForEvent("saveloc_upd8", UtlMakeDelegate(this, &C_HudTimer::OnSavelocUpdateEvent));
}

void C_HudTimer::Init()
{
    // We reset only if it was a run not a replay -> lets check if shared was valid first
    m_iTotalTicks = 0;
    m_iOldTickCount = 0;
    m_iStartTick = 0;
    m_G_iCurrentTick = 0;
    m_G_iStartTickD = 0;
    HOOK_HUD_MESSAGE(C_HudTimer, Timer_State);
    HOOK_HUD_MESSAGE(C_HudTimer, Timer_Reset);
    initialTall = 48;
    m_iZoneCount = 0;
    m_pRunStats = nullptr;
    // Reset();

    // cache localization strings
    FIND_LOCALIZATION(m_pwStageStartString, "#MOM_Stage_Start");
    LOCALIZE_TOKEN(Checkpoint, "#MOM_Checkpoint", cpLocalized);
    LOCALIZE_TOKEN(SaveLoc, "#MOM_SavedLocation", savelocLocalized);
    LOCALIZE_TOKEN(Stage, "#MOM_Stage", stLocalized);
    LOCALIZE_TOKEN(Linear, "#MOM_Linear", linearLocalized);
    LOCALIZE_TOKEN(InsideStart, "#MOM_InsideStartZone", startZoneLocalized);
    LOCALIZE_TOKEN(MapFinished, "#MOM_MapFinished", mapFinishedLocalized);
    LOCALIZE_TOKEN(PracticeMode, "#MOM_PracticeMode", practiceModeLocalized);
    LOCALIZE_TOKEN(NoTimer, "#MOM_NoTimer", noTimerLocalized);
}

void C_HudTimer::Reset()
{
    // We reset only if it was a run not a replay -> lets check if shared was valid first
    m_iTotalTicks = 0;
    m_iStartTick = 0;
    m_G_iCurrentTick = 0;
    m_G_iStartTickD = 0;
    m_iOldTickCount = 0;
    m_bIsRunning = false;
    m_bTimerRan = false;
    m_iZoneCurrent = 1;
    m_bShowSavelocs = false;
    m_bPlayerUsingSavelocMenu = false;
    m_bWereCheatsActivated = false;
    m_bPlayerHasPracticeMode = false;
    m_bPlayerInZone = false;
    m_bMapFinished = false;
    m_bMapIsLinear = false;
    m_iSavelocCount = m_iPlayerSavelocCount = m_iSavelocCurrent = m_iSavelocCurrent = 0;
    m_pRunStats = nullptr;
}

// This void handles playing effects for run start and run stop
void C_HudTimer::MsgFunc_Timer_State(bf_read &msg)
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (!pPlayer)
        return;

    bool started = msg.ReadOneBit();

    if (started)
    {
        // VGUI_ANIMATE("TimerStart");
        // Checking again, even if we just checked 8 lines before

        pPlayer->EmitSound("Momentum.StartTimer");
        m_bTimerRan = true;
    }
    else // stopped
    {
        // Compare times.
        if (m_bWereCheatsActivated) // EY, CHEATER, STOP
        {
            DevWarning("sv_cheats was set to 1, thus making the run not valid \n");
        }
        else // He didn't cheat, we can carry on
        {
            // m_iTotalTicks = gpGlobals->tickcount - m_iStartTick;
            // DevMsg("Ticks upon exit: %i and total seconds: %f\n", m_iTotalTicks, gpGlobals->interval_per_tick);
            // Paint();
            // DevMsg("%s \n", m_pszString);
        }

        // VGUI_ANIMATE("TimerStop");
        m_bTimerRan = true;
        pPlayer->EmitSound("Momentum.StopTimer");

        // MOM_TODO: (Beta+) show scoreboard animation with new position on leaderboards?
    }
}

void C_HudTimer::MsgFunc_Timer_Reset(bf_read &msg) { Reset(); }

float C_HudTimer::GetCurrentTime()
{
    // HACKHACK: The client timer stops 1 tick behind the server timer for unknown reasons,
    // so we add an extra tick here to make them line up again

    // Done, I've shouldn't have checked if tickcount wasn't the same for only one frame, but for all the frames that
    // paint is getting called.

    if (gpGlobals->tickcount != m_iOldTickCount && !m_bIsReplay)
    {
        m_iTotalTicks = m_bIsRunning ? (gpGlobals->tickcount - m_iStartTick) : 0;
    }

    if (m_bIsReplay)
    {
        m_iTotalTicks = m_G_iCurrentTick - m_G_iStartTickD;
    }

    m_iOldTickCount = gpGlobals->tickcount;

    return static_cast<float>(m_iTotalTicks) * gpGlobals->interval_per_tick;
}

void C_HudTimer::OnSavelocUpdateEvent(KeyValues* pKv)
{
    m_bPlayerUsingSavelocMenu = pKv->GetBool("using");
    m_iPlayerSavelocCount = pKv->GetInt("count");
    m_iPlayerSavelocCurrent = pKv->GetInt("current", -1) + 1;
}

void C_HudTimer::OnThink()
{
    C_MomentumPlayer *pLocal = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (pLocal && g_MOMEventListener)
    {
        C_MomentumReplayGhostEntity *pGhost = pLocal->GetReplayEnt();
        C_MOMRunEntityData *runData;
        if (pGhost)
        {
            m_bShowSavelocs = false;
            m_iSavelocCurrent = 0;
            m_iSavelocCount = 0;
            m_pRunStats = &pGhost->m_RunStats;
            m_bIsReplay = true;
            m_bPlayerHasPracticeMode = false;
            m_G_iCurrentTick = pGhost->m_SrvData.m_iCurrentTick;
            m_G_iStartTickD = pGhost->m_SrvData.m_RunData.m_iStartTickD;
            runData = &pGhost->m_SrvData.m_RunData;
        }
        else
        {
            m_bIsReplay = false;
            m_bShowSavelocs = m_bPlayerUsingSavelocMenu;
            m_iSavelocCurrent = m_iPlayerSavelocCurrent;
            m_iSavelocCount = m_iPlayerSavelocCount;
            m_bPlayerHasPracticeMode = pLocal->m_SrvData.m_bHasPracticeMode;
            m_pRunStats = &pLocal->m_RunStats;
            runData = &pLocal->m_SrvData.m_RunData;
        }

        m_bIsRunning = runData->m_bTimerRunning;
        m_iStartTick = runData->m_iStartTick;
        m_iZoneCurrent = runData->m_iCurrentZone;
        m_bPlayerInZone = runData->m_bIsInZone;
        m_bMapFinished = runData->m_bMapFinished;
        m_iZoneCount = g_MOMEventListener->m_iMapZoneCount;
        m_bMapIsLinear = g_MOMEventListener->m_bMapIsLinear;
    }
}

void C_HudTimer::Paint(void)
{
    // Format the run's time
    g_pMomentumUtil->FormatTime(GetCurrentTime(), m_pszString, 2);
    ANSI_TO_UNICODE(m_pszString, m_pwCurrentTime);

    if (m_bShowSavelocs)
    {
        Q_snprintf(m_pszStringCps, sizeof(m_pszStringCps), "%s %i/%i",
                   savelocLocalized,     // Saveloc localization
                   m_iSavelocCurrent, // CurrentCP
                   m_iSavelocCount    // CPCount
                   );

        ANSI_TO_UNICODE(m_pszStringCps, m_pwCurrentCheckpoints);
    }

    char prevStageString[BUFSIZELOCL], comparisonANSI[BUFSIZELOCL];
    wchar_t prevStageStringUnicode[BUFSIZELOCL];
    Color compareColor = GetFgColor();

    if (m_iZoneCurrent > 1)
    {
        Q_snprintf(prevStageString, BUFSIZELOCL, "%s %i",
                   m_bMapIsLinear ? cpLocalized : stLocalized, // Stage localization ("Checkpoint:" if linear)
                   m_iZoneCurrent - 1);                        // Last stage number

        ANSI_TO_UNICODE(prevStageString, prevStageStringUnicode);

        ConVarRef timeType("mom_comparisons_time_type");
        // This void works even if there is no comparison loaded
        g_MOMRunCompare->GetComparisonString(timeType.GetBool() ? ZONE_TIME : TIME_OVERALL, m_pRunStats,
                                             m_iZoneCurrent - 1, m_pszStageTimeString, comparisonANSI, &compareColor);

        // Convert the split to Unicode
        ANSI_TO_UNICODE(m_pszStageTimeString, m_pwStageTimeLabel);
    }

    // find out status of timer (no timer/practice mode)
    if (!m_bIsRunning)
    {
        Q_strncpy(m_pszStringStatus, m_bPlayerHasPracticeMode ? practiceModeLocalized : noTimerLocalized,
                  sizeof(m_pszStringStatus));
        ANSI_TO_UNICODE(m_pszStringStatus, m_pwCurrentStatus);
    }

    // Draw the text label.
    surface()->DrawSetTextFont(m_bIsRunning ? m_hTimerFont : m_hTextFont);
    surface()->DrawSetTextColor(GetFgColor());

    int dummy, totalWide;
    // Draw current time.
    GetSize(totalWide, dummy);

    if (center_time)
    {
        int timeWide;
        surface()->GetTextSize(m_bIsRunning ? m_hTimerFont : m_hTextFont,
                               m_bIsRunning ? m_pwCurrentTime : m_pwCurrentStatus, timeWide, dummy);
        int offsetToCenter = ((totalWide - timeWide) / 2);
        surface()->DrawSetTextPos(offsetToCenter, time_ypos);
    }
    else
    {
        surface()->DrawSetTextPos(time_xpos, time_ypos);
    }

    // draw either timer display or the timer status
    // If the timer isn't running, it'll print "No timer" or "Practice mode"
    surface()->DrawPrintText(m_bIsRunning ? m_pwCurrentTime : m_pwCurrentStatus,
                             m_bIsRunning ? wcslen(m_pwCurrentTime) : wcslen(m_pwCurrentStatus));

    surface()->DrawSetTextFont(m_hSmallTextFont);

    if (m_bShowSavelocs)
    {
        if (center_cps)
        {
            int cpsWide;
            surface()->GetTextSize(m_hSmallTextFont, m_pwCurrentCheckpoints, cpsWide, dummy);
            int offsetToCenter = ((totalWide - cpsWide) / 2);
            surface()->DrawSetTextPos(offsetToCenter, cps_ypos);
        }
        else
            surface()->DrawSetTextPos(cps_xpos, cps_ypos);

        surface()->DrawPrintText(m_pwCurrentCheckpoints, wcslen(m_pwCurrentCheckpoints));
    }
    // don't draw stages when drawing checkpoints, and vise versa.
    else if (m_iZoneCurrent > 1 && m_bIsRunning)
    {
        // only draw split timer if we are on stage/checkpoint 2 (not start, which is 1) or above.
        bool hasComparison = g_MOMRunCompare->LoadedComparison();
        int prevStageXPos = split_xpos, stageSplitXPos = split_xpos, splitY = split_ypos;
        int yToIncrement = surface()->GetFontTall(m_hSmallTextFont);
        if (center_split)
        {
            prevStageXPos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hSmallTextFont, prevStageString) / 2;

            // Inline the comparison (affects split xpos)
            stageSplitXPos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hSmallTextFont, m_pszStageTimeString) / 2;
        }

        // Print the previous stage
        surface()->DrawSetTextPos(prevStageXPos, splitY);
        surface()->DrawPrintText(prevStageStringUnicode, wcslen(prevStageStringUnicode));

        // Go down a line
        splitY += yToIncrement;

        // Print the split
        surface()->DrawSetTextPos(stageSplitXPos, splitY);
        surface()->DrawPrintText(m_pwStageTimeLabel, wcslen(m_pwStageTimeLabel));

        // Draw the comparison to the split, if existent
        if (hasComparison)
        {
            // Convert to unicode.
            wchar_t comparisonUnicode[BUFSIZELOCL];
            ANSI_TO_UNICODE(comparisonANSI, comparisonUnicode);

            // This will be right below where the time begins to print
            int compare_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hSmallTextFont, comparisonANSI) / 2;
            splitY += yToIncrement;

            // Print the comparison
            surface()->DrawSetTextPos(compare_xpos, splitY);
            surface()->DrawSetTextColor(compareColor);
            surface()->DrawPrintText(comparisonUnicode, wcslen(comparisonUnicode));
        }
    }
}