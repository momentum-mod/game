#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "hud_macros.h"
#include "utlvector.h"
#include "KeyValues.h"
#include "iclientmode.h"
#include "view.h"
#include "menu.h"
#include "vgui_helpers.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>

#include "mom_event_listener.h"
#include "momentum/util/mom_util.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar mom_timer("mom_timer", "1",
    FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
    "Turn the timer display on/off\n");

static ConVar timer_mode("mom_timer_mode", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Set what type of timer you want.\n0 = Generic Timer (no splits)\n1 = Splits by Checkpoint\n");

class C_Timer : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(C_Timer, Panel);
public:
    C_Timer();
    C_Timer(const char *pElementName);
    virtual void Init();
    virtual void Reset();
    virtual void OnThink();
    virtual bool ShouldDraw()
    {
        return mom_timer.GetBool() && CHudElement::ShouldDraw();
    }
    virtual void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
        m_TimeGain = GetSchemeColor("MOM.Timer.Gain", pScheme);
        m_TimeLoss = GetSchemeColor("MOM.Timer.Loss", pScheme);
    }
    void MsgFunc_Timer_State(bf_read &msg);
    void MsgFunc_Timer_Reset(bf_read &msg);
    void MsgFunc_Timer_Checkpoint(bf_read &msg);
    void MsgFunc_Timer_Stage(bf_read &msg);
    void MsgFunc_Timer_StageCount(bf_read&);
    virtual void Paint();
    float GetCurrentTime();
    bool m_bIsRunning;
    bool m_bTimerRan;
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

    CPanelAnimationVarAliasType(bool, center_time, "centerTime", "0",
        "BOOL");
    CPanelAnimationVarAliasType(float, time_xpos, "time_xpos", "0",
        "proportional_float");
    CPanelAnimationVarAliasType(float, time_ypos, "time_ypos", "2",
        "proportional_float");
    CPanelAnimationVarAliasType(bool, center_cps, "centerCps", "1",
        "BOOL");
    CPanelAnimationVarAliasType(float, cps_xpos, "cps_xpos", "50",
        "proportional_float");
    CPanelAnimationVarAliasType(float, cps_ypos, "cps_ypos", "25",
        "proportional_float");
    CPanelAnimationVarAliasType(bool, center_stage, "centerStage", "1",
        "BOOL");
    CPanelAnimationVarAliasType(float, stage_xpos, "stage_xpos", "50",
        "proportional_float");
    CPanelAnimationVarAliasType(float, stage_ypos, "stage_ypos", "40",
        "proportional_float");
private:
    int m_iStageCurrent;
    int m_iStageCount;
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
    char m_pszStageTimeComparisonANSI[BUFSIZETIME], m_pszStageTimeComparisonLabel[BUFSIZETIME];

    KeyValues *m_kvBestTimeBuffer, *m_kvBestTime;
    CUtlVector<float> m_vecBestTimes;

    int m_iTotalTicks;
    bool m_bWereCheatsActivated = false;
    bool m_bShowCheckpoints;
    int m_iCheckpointCount;
    int m_iCheckpointCurrent;
    Color panelColor;
    char stLocalized[BUFSIZELOCL], cpLocalized[BUFSIZELOCL], linearLocalized[BUFSIZELOCL],
        startZoneLocalized[BUFSIZELOCL], mapFinishedLocalized[BUFSIZELOCL], practiceModeLocalized[BUFSIZELOCL], 
        noTimerLocalized[BUFSIZELOCL];
};

DECLARE_HUDELEMENT(C_Timer);
// MOM_TODO add more for checkpoints and ending
DECLARE_HUD_MESSAGE(C_Timer, Timer_State);
DECLARE_HUD_MESSAGE(C_Timer, Timer_Reset);
DECLARE_HUD_MESSAGE(C_Timer, Timer_Checkpoint);
DECLARE_HUD_MESSAGE(C_Timer, Timer_Stage);
DECLARE_HUD_MESSAGE(C_Timer, Timer_StageCount);

C_Timer::C_Timer(const char *pElementName) :
CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "HudTimer")
{
    // This is already set for HUD elements, but still...
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    m_kvBestTime = nullptr;
    m_kvBestTimeBuffer = nullptr;
}

void C_Timer::Init()
{
    HOOK_HUD_MESSAGE(C_Timer, Timer_State);
    HOOK_HUD_MESSAGE(C_Timer, Timer_Reset);
    HOOK_HUD_MESSAGE(C_Timer, Timer_Checkpoint);
    HOOK_HUD_MESSAGE(C_Timer, Timer_Stage);
    HOOK_HUD_MESSAGE(C_Timer, Timer_StageCount);
    initialTall = 48;
    m_iTotalTicks = 0;
    //Reset();

    //cache localization strings
    wchar_t *uCPUnicode = g_pVGuiLocalize->Find("#MOM_Checkpoint");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uCPUnicode ? uCPUnicode : L"#MOM_Checkpoint", cpLocalized, BUFSIZELOCL);

    wchar_t *uStageUnicode = g_pVGuiLocalize->Find("#MOM_Stage");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uStageUnicode ? uStageUnicode : L"#MOM_Stage", stLocalized, BUFSIZELOCL);

    wchar_t *uLinearUnicode = g_pVGuiLocalize->Find("#MOM_Linear");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uLinearUnicode ? uLinearUnicode : L"#MOM_Linear", linearLocalized, BUFSIZELOCL);

    wchar_t *uStartZoneUnicode = g_pVGuiLocalize->Find("#MOM_InsideStartZone");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uStartZoneUnicode ? uStartZoneUnicode : L"#MOM_InsideStartZone", startZoneLocalized, BUFSIZELOCL);

    wchar_t *uMapFinishedUnicode = g_pVGuiLocalize->Find("#MOM_MapFinished");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uMapFinishedUnicode ? uMapFinishedUnicode : L"#MOM_MapFinished", mapFinishedLocalized, BUFSIZELOCL);

    wchar_t *uPracticeModeUnicode = g_pVGuiLocalize->Find("#MOM_PracticeMode");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uPracticeModeUnicode ? uPracticeModeUnicode : L"#MOM_PracticeMode", practiceModeLocalized, BUFSIZELOCL);

    wchar_t *uNoTimerUnicode = g_pVGuiLocalize->Find("#MOM_NoTimer");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uNoTimerUnicode ? uNoTimerUnicode : L"#MOM_NoTimer", noTimerLocalized, BUFSIZELOCL);
}

void C_Timer::Reset()
{
    m_bIsRunning = false;
    m_bTimerRan = false;
    m_iTotalTicks = 0;
    m_iStageCount = 0;
    m_iStageCurrent = 0;
    m_bShowCheckpoints = false;
    m_iCheckpointCount = 0;
    m_iCheckpointCurrent = 0;
}

void C_Timer::OnThink()
{
    if (m_iStageCount == 0)
        engine->ServerCmd("hud_timer_request_stages");
    // Cheat detection moved to server Timer.cpp
}

void C_Timer::MsgFunc_Timer_State(bf_read &msg)
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (!pPlayer)
        return;

    bool started = msg.ReadOneBit();
    m_bIsRunning = started;
    m_iStartTick = (int) msg.ReadLong();

    
    if (started)
    {
        
    } 
    else
    {

    }
    
    
    if (started)
    {
        //VGUI_ANIMATE("TimerStart");
        // Checking again, even if we just checked 8 lines before
        if (pPlayer != nullptr)
        {
            pPlayer->EmitSound("Momentum.StartTimer");
            m_bTimerRan = true;
        }
        const char* szMapName = g_pGameRules ? g_pGameRules->MapName() : nullptr;
        if (szMapName)
        {
            m_kvBestTimeBuffer = new KeyValues(szMapName);
            m_kvBestTime = mom_UTIL->GetBestTime(m_kvBestTimeBuffer, szMapName, gpGlobals->interval_per_tick, pPlayer->m_iRunFlags);
            if (m_kvBestTime)
                mom_UTIL->GetBestStageTimes(m_kvBestTime, &m_vecBestTimes);
        }
    }
    else // stopped
    {
        // Compare times.
        if (m_kvBestTimeBuffer) m_kvBestTimeBuffer->deleteThis();
        m_kvBestTime = nullptr;
        m_kvBestTimeBuffer = nullptr;
        m_vecBestTimes.RemoveAll();

        if (m_bWereCheatsActivated) //EY, CHEATER, STOP
        {
            DevWarning("sv_cheats was set to 1, thus making the run not valid \n");
        }
        else //He didn't cheat, we can carry on
        {
            //m_iTotalTicks = gpGlobals->tickcount - m_iStartTick;
            //DevMsg("Ticks upon exit: %i and total seconds: %f\n", m_iTotalTicks, gpGlobals->interval_per_tick);
            //Paint();
            //DevMsg("%s \n", m_pszString);
        }

        //VGUI_ANIMATE("TimerStop");
        if (pPlayer != nullptr)
        {
            pPlayer->EmitSound("Momentum.StopTimer");
            pPlayer->m_flLastRunTime = static_cast<float>(gpGlobals->tickcount - m_iStartTick) * gpGlobals->interval_per_tick;
        }

        //MOM_TODO: (Beta+) show scoreboard animation with new position on leaderboards?
    }
}

void C_Timer::MsgFunc_Timer_Reset(bf_read &msg)
{
    Reset();
}

void C_Timer::MsgFunc_Timer_Checkpoint(bf_read &msg)
{
    m_bShowCheckpoints = msg.ReadOneBit();
    m_iCheckpointCurrent = static_cast<int>(msg.ReadLong());
    m_iCheckpointCount = static_cast<int>(msg.ReadLong());
}

void C_Timer::MsgFunc_Timer_Stage(bf_read &msg)
{
    m_iStageCurrent = static_cast<int>(msg.ReadLong());
    //g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("MenuPulse");
}

void C_Timer::MsgFunc_Timer_StageCount(bf_read &msg)
{
    m_iStageCount = static_cast<int>(msg.ReadLong());
}
float C_Timer::GetCurrentTime()
{
    //HACKHACK: The client timer stops 1 tick behind the server timer for unknown reasons,
    //so we add an extra tick here to make them line up again
    if (m_bIsRunning)
        m_iTotalTicks = gpGlobals->tickcount - m_iStartTick + 1;
    return static_cast<float>(m_iTotalTicks) * gpGlobals->interval_per_tick;
}

void C_Timer::Paint(void)
{
    mom_UTIL->FormatTime(GetCurrentTime(), m_pszString, 2);
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszString, m_pwCurrentTime, sizeof(m_pwCurrentTime));

    //find out status of checkpoints (linear vs checkpoints)
    if (m_bShowCheckpoints)
    {
        Q_snprintf(m_pszStringCps, sizeof(m_pszStringCps), "%s %i/%i",
            cpLocalized, // Checkpoint localization
            m_iCheckpointCurrent, //CurrentCP
            m_iCheckpointCount //CPCount
            );
        g_pVGuiLocalize->ConvertANSIToUnicode(
            m_pszStringCps, m_pwCurrentCheckpoints, sizeof(m_pwCurrentCheckpoints));
    }

    bool losingTime = false, hasComparison = false;
    float diff = 0;

    if (m_iStageCount > 1)
    {
        Q_snprintf(m_pszStringStages, sizeof(m_pszStringStages), "%s %i/%i",
            stLocalized, // Stage localization
            m_iStageCurrent, // Current Stage
            m_iStageCount // Total number of stages
            );
        if (m_iStageCurrent > 1)
        {            
            //Local PB comparison
            if (!m_vecBestTimes.IsEmpty())
            {
                hasComparison = true;
                diff = g_MOMEventListener->m_flStageTime[g_MOMEventListener->m_iCurrentStage] - m_vecBestTimes[g_MOMEventListener->m_iCurrentStage - 1];

                //MOM_TODO: what if the diff == 0? (probably unlikely)
                losingTime = (diff > 0);//If diff > 0, that means you're falling behind your PB!

                mom_UTIL->FormatTime(diff, m_pszStageTimeComparisonANSI);

                Q_snprintf(m_pszStageTimeComparisonLabel, sizeof(m_pszStageTimeComparisonLabel),
                    "(%c %s)", losingTime ? '+' : '-', m_pszStageTimeComparisonANSI);

                g_pVGuiLocalize->ConvertANSIToUnicode(m_pszStageTimeComparisonLabel, m_pwStageTimeComparison, 
                    sizeof(m_pwStageTimeComparison));
            }
            //MOM_TODO: calculate diff from WR (online)
            
            mom_UTIL->FormatTime(g_MOMEventListener->m_flStageTime[g_MOMEventListener->m_iCurrentStage], m_pszStageTimeString);
            Q_snprintf(m_pszStageTimeLabelString, sizeof(m_pszStageTimeLabelString), "(%s)",
                m_pszStageTimeString,
                m_pszStageTimeLabelString
                );
            g_pVGuiLocalize->ConvertANSIToUnicode(
                m_pszStageTimeLabelString, m_pwStageTimeLabel, sizeof(m_pwStageTimeLabel));
        }
    }
    else //it's a linear map
    {
        Q_snprintf(m_pszStringStages, sizeof(m_pszStringStages), linearLocalized);
    }
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringStages, m_pwCurrentStages, sizeof(m_pwCurrentStages));

    //find out status of timer (start zone/end zone/practice mode)
    if (g_MOMEventListener->m_bPlayerInsideStartZone)
    {
        Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), startZoneLocalized);
    }
    else if (g_MOMEventListener->m_bPlayerInsideEndZone && g_MOMEventListener->m_bMapFinished) //player finished map with timer running
    {
        Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), mapFinishedLocalized);
    }
    else if (g_MOMEventListener->m_bPlayerHasPracticeMode)
    {
        Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), practiceModeLocalized);
    }
    else //no timer
    {
        Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), noTimerLocalized);
    }
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringStatus, m_pwCurrentStatus, sizeof(m_pwCurrentStatus));
    
    // Draw the text label.
    surface()->DrawSetTextFont(m_bIsRunning ? m_hTimerFont : m_hTextFont);
    surface()->DrawSetTextColor(GetFgColor());

    int dummy, totalWide;
    // Draw current time.
    GetSize(totalWide, dummy);

    if (center_time)
    {
        int timeWide;
        surface()->GetTextSize(m_bIsRunning ? m_hTimerFont : m_hTextFont, m_bIsRunning ? m_pwCurrentTime : m_pwCurrentStatus, timeWide, dummy);
        int offsetToCenter = ((totalWide - timeWide) / 2);
        surface()->DrawSetTextPos(offsetToCenter, time_ypos);
    }
    else
    {
        surface()->DrawSetTextPos(time_xpos, time_ypos);
    }

    //draw either timer display or the timer status
    surface()->DrawPrintText(m_bIsRunning ? m_pwCurrentTime : m_pwCurrentStatus, m_bIsRunning ? wcslen(m_pwCurrentTime) : wcslen(m_pwCurrentStatus));

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
    else //don't draw stages when drawing checkpoints, and vise versa
    {
        // MOM_TODO: Print this only if map gamemode is supported
        if (center_stage)
        {
            int stageWide;
            surface()->GetTextSize(m_hSmallTextFont, m_pwCurrentStages, stageWide, dummy);
            int offsetToCenter = ((totalWide - stageWide) / 2);
            surface()->DrawSetTextPos(offsetToCenter, stage_ypos);
        }
        else
            surface()->DrawSetTextPos(stage_xpos, stage_ypos);

        surface()->DrawPrintText(m_pwCurrentStages, wcslen(m_pwCurrentStages));

        if (m_iStageCurrent > 1) //only draw stage timer if we are on stage 2 or above.
        {
            int text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hSmallTextFont, m_pwStageTimeLabel) / 2;
            
            surface()->DrawSetTextPos(text_xpos, cps_ypos);
            surface()->DrawPrintText(m_pwStageTimeLabel, wcslen(m_pwStageTimeLabel));
            if (hasComparison)
            {
                text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hSmallTextFont, m_pwStageTimeComparison) / 2;
                int tall = surface()->GetFontTall(m_hSmallTextFont);
                surface()->DrawSetTextColor(losingTime ? m_TimeLoss : m_TimeGain);//MOM_TODO: possibly handle ties?
                surface()->DrawSetTextPos(text_xpos, cps_ypos + tall);
                surface()->DrawPrintText(m_pwStageTimeComparison, wcslen(m_pwStageTimeComparison));
            }
        }
    }
}