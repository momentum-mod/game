#include "cbase.h"
#include "hud_comparisons.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "baseviewport.h"
#include "menu.h"
#include "utlvector.h"
#include "vgui_helpers.h"
#include "view.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Frame.h>
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

class C_Timer : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(C_Timer, Panel);

  public:
    C_Timer(const char *pElementName);
    void OnThink() override;
    void Init() override;
    void Reset() override;
    void Paint() override;
    bool ShouldDraw() override
    { 
        IViewPortPanel *pLeaderboards = gViewPortInterface->FindPanelByName(PANEL_TIMES);
        return mom_timer.GetBool() && CHudElement::ShouldDraw() && pLeaderboards && !pLeaderboards->IsVisible();
    }

    void ApplySchemeSettings(IScheme *pScheme) override
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
    bool m_bTimerRan;//MOM_TODO: What is this used for?
    int m_iStartTick;

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
    int initialTall;

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
    bool m_bShowCheckpoints;
    bool m_bMapFinished;
    bool m_bMapIsLinear;
    int m_iCheckpointCount, m_iCheckpointCurrent;
    CMomRunStats *m_pRunStats;
    char stLocalized[BUFSIZELOCL], cpLocalized[BUFSIZELOCL], linearLocalized[BUFSIZELOCL],
        startZoneLocalized[BUFSIZELOCL], mapFinishedLocalized[BUFSIZELOCL], practiceModeLocalized[BUFSIZELOCL],
        noTimerLocalized[BUFSIZELOCL];
};

DECLARE_HUDELEMENT(C_Timer);
DECLARE_HUD_MESSAGE(C_Timer, Timer_State);
DECLARE_HUD_MESSAGE(C_Timer, Timer_Reset);

C_Timer::C_Timer(const char *pElementName) : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "HudTimer")
{
    // This is already set for HUD elements, but still...
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}

void C_Timer::Init()
{
    HOOK_HUD_MESSAGE(C_Timer, Timer_State);
    HOOK_HUD_MESSAGE(C_Timer, Timer_Reset);
    initialTall = 48;
	if (shared)
		shared->m_iTotalTicksT = 0;
    m_iZoneCount = 0;
    m_pRunStats = nullptr;
    // Reset();

    // cache localization strings
    FIND_LOCALIZATION(m_pwStageStartString, "#MOM_Stage_Start");
    LOCALIZE_TOKEN(Checkpoint, "#MOM_Checkpoint", cpLocalized);
    LOCALIZE_TOKEN(Stage, "#MOM_Stage", stLocalized);
    LOCALIZE_TOKEN(Linear, "#MOM_Linear", linearLocalized);
    LOCALIZE_TOKEN(InsideStart, "#MOM_InsideStartZone", startZoneLocalized);
    LOCALIZE_TOKEN(MapFinished, "#MOM_MapFinished", mapFinishedLocalized);
    LOCALIZE_TOKEN(PracticeMode, "#MOM_PracticeMode", practiceModeLocalized);
    LOCALIZE_TOKEN(NoTimer, "#MOM_NoTimer", noTimerLocalized);
}

void C_Timer::Reset()
{
    m_bIsRunning = false;
    m_bTimerRan = false;
	if (shared)
		shared->m_iTotalTicksT = 0;
    m_iZoneCurrent = 1;
    m_bShowCheckpoints = false;
    m_bWereCheatsActivated = false;
    m_bPlayerHasPracticeMode = false;
    m_bPlayerInZone = false;
    m_bMapFinished = false;
    m_bMapIsLinear = false;
    m_iCheckpointCount = 0;
    m_iCheckpointCurrent = 0;
    m_pRunStats = nullptr;
}

// This void handles playing effects for run start and run stop
void C_Timer::MsgFunc_Timer_State(bf_read &msg)
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (!pPlayer)
        return;

    bool started = msg.ReadOneBit();

    if (started)
    {
        // VGUI_ANIMATE("TimerStart");
        // Checking again, even if we just checked 8 lines before
        if (pPlayer != nullptr)
        {
            pPlayer->EmitSound("Momentum.StartTimer");
            m_bTimerRan = true;
        }
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
        if (pPlayer != nullptr)
        {
			  m_bTimerRan = true;
            pPlayer->EmitSound("Momentum.StopTimer");
        }

        // MOM_TODO: (Beta+) show scoreboard animation with new position on leaderboards?
    }
}

void C_Timer::MsgFunc_Timer_Reset(bf_read &msg) { Reset(); }

float C_Timer::GetCurrentTime()
{
    // HACKHACK: The client timer stops 1 tick behind the server timer for unknown reasons,
    // so we add an extra tick here to make them line up again

	//If we even loaded the shared DLL
    if (shared)
    {
        shared->m_iTotalTicksT = m_bIsRunning ? gpGlobals->tickcount - m_iStartTick + 1 : 0;
    }
    else
    {
        return 0.0f;
    }
	

	return static_cast<float>(shared->m_iTotalTicksT) * gpGlobals->interval_per_tick;
}

// Calculations should be done in here. Paint is for drawing what the calculations have done.
void C_Timer::OnThink()
{
    C_MomentumPlayer *pLocal = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (pLocal && g_MOMEventListener)
    {
        C_MomentumReplayGhostEntity *pGhost = pLocal->GetReplayEnt();
        C_MOMRunEntityData *runData;
        if (pGhost)
        {
            m_bShowCheckpoints = false;
            m_iCheckpointCurrent = 0;
            m_iCheckpointCount = 0;
            m_pRunStats = &pGhost->m_RunStats;
            m_bPlayerHasPracticeMode = false;
            runData = &pGhost->m_RunData;
        }
        else
        {
            m_bShowCheckpoints = pLocal->m_bUsingCPMenu;
            m_iCheckpointCurrent = pLocal->m_iCurrentStepCP + 1;
            m_iCheckpointCount = pLocal->m_iCheckpointCount;
            m_bPlayerHasPracticeMode = pLocal->m_bHasPracticeMode;
            m_pRunStats = &pLocal->m_RunStats;
            runData = &pLocal->m_RunData;
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

void C_Timer::Paint(void)
{
    // Format the run's time
    mom_UTIL->FormatTime(GetCurrentTime(), m_pszString, 2);
    ANSI_TO_UNICODE(m_pszString, m_pwCurrentTime);

    if (m_bShowCheckpoints)
    {
        Q_snprintf(m_pszStringCps, sizeof(m_pszStringCps), "%s %i/%i",
                   cpLocalized,          // Checkpoint localization
                   m_iCheckpointCurrent, // CurrentCP
                   m_iCheckpointCount    // CPCount
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
                   m_iZoneCurrent - 1); // Last stage number

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
        if (m_bPlayerHasPracticeMode) // In practice mode
        {
            Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), practiceModeLocalized);
        }
        else // no timer
        {
            Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), noTimerLocalized);
        }
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
    if (m_bShowCheckpoints)
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

            //Inline the comparison (affects split xpos)
            int extra = hasComparison ? UTIL_ComputeStringWidth(m_hSmallTextFont, comparisonANSI) : 0;
            stageSplitXPos = GetWide() / 2 - (UTIL_ComputeStringWidth(m_hSmallTextFont, m_pszStageTimeString) + extra) / 2;
        }

        //Print the previous stage
        surface()->DrawSetTextPos(prevStageXPos, splitY);
        surface()->DrawPrintText(prevStageStringUnicode, wcslen(prevStageStringUnicode));

        //Go down a line
        splitY += yToIncrement;

        //Print the split
        surface()->DrawSetTextPos(stageSplitXPos, splitY);
        surface()->DrawPrintText(m_pwStageTimeLabel, wcslen(m_pwStageTimeLabel));

        //Draw the comparison to the split, if existent
        if (hasComparison)
        {
            // Convert to unicode.
            wchar_t comparisonUnicode[BUFSIZELOCL];
            ANSI_TO_UNICODE(comparisonANSI, comparisonUnicode);

            // This will be right below where the time begins to print, but is unwanted
            //int compare_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hSmallTextFont, comparisonANSI) / 2;
            //splitY += yToIncrement;

            //Find the xpos of the comparison string
            int compare_xpos = stageSplitXPos + UTIL_ComputeStringWidth(m_hSmallTextFont, m_pszStageTimeString) + 2;
            //Print the comparison
            surface()->DrawSetTextPos(compare_xpos, splitY);
            surface()->DrawSetTextColor(compareColor);
            surface()->DrawPrintText(comparisonUnicode, wcslen(comparisonUnicode));
        }
    }
}