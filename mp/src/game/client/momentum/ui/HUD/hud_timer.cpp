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
#include <vgui_controls/Label.h>
#include "vgui_controls/AnimationController.h"

#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"
#include "c_mom_replay_entity.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_timer, "1", FCVAR_ARCHIVE, "Toggle displaying the timer. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_timer_sound_fail_enable, "1", FCVAR_ARCHIVE, "Toggle sound on timer fail. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_timer_sound_start_enable, "1", FCVAR_ARCHIVE, "Toggle sound on timer start. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_timer_sound_stop_enable, "1", FCVAR_ARCHIVE, "Toggle sound on timer stop. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_timer_sound_finish_enable, "1", FCVAR_ARCHIVE, "Toggle sound on timer finish. 0 = OFF, 1 = ON\n");

class CHudTimer : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudTimer, EditablePanel);
    CHudTimer(const char *pElementName);
    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void LevelShutdown() OVERRIDE;
    void FireGameEvent(IGameEvent* event) OVERRIDE;
    void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;
    void MsgFunc_Timer_Event(bf_read &msg);
    void MsgFunc_Timer_Reset(bf_read &msg);

    CPanelAnimationVar(Color, m_StatusColor, "StatusColor", "Mom.Panel.Fg");

  private:
    void SetToNoTimer();

    Label *m_pMainStatusLabel, *m_pInfoLabel, *m_pSplitLabel, *m_pComparisonLabel;

    wchar_t m_wStageStart[BUFSIZELOCL];
    wchar_t m_wSavelocStatus[BUFSIZELOCL];
    wchar_t m_wNoTimer[BUFSIZELOCL];
    wchar_t m_wPracticeMode[BUFSIZELOCL];
    wchar_t m_wStageNum[BUFSIZELOCL];
    wchar_t m_wCheckpointNum[BUFSIZELOCL];

    bool m_bWasUsingSavelocMenu;
    int m_iSavelocCurrent, m_iSavelocCount;
    CMomRunStats *m_pRunStats;
    CMomRunEntityData *m_pRunData;
};

DECLARE_HUDELEMENT(CHudTimer);
DECLARE_HUD_MESSAGE(CHudTimer, Timer_Event);
DECLARE_HUD_MESSAGE(CHudTimer, Timer_Reset);

CHudTimer::CHudTimer(const char *pElementName): CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudTimer")
{
    // This is already set for HUD elements, but still...
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);
    m_bWasUsingSavelocMenu = false;
    m_iSavelocCurrent = 0;
    m_iSavelocCount = 0;

    ListenForGameEvent("saveloc_upd8");

    m_pMainStatusLabel = new Label(this, "MainStatusLabel", "");
    m_pInfoLabel = new Label(this, "InfoLabel", "");
    m_pSplitLabel = new Label(this, "SplitLabel", "");
    m_pComparisonLabel = new Label(this, "ComparisonLabel", "");

    LoadControlSettings("resource/ui/Timer.res");

    HOOK_HUD_MESSAGE(CHudTimer, Timer_Event);
    HOOK_HUD_MESSAGE(CHudTimer, Timer_Reset);
}

void CHudTimer::Init()
{
    m_pRunStats = nullptr;
    m_pRunData = nullptr;
}

void CHudTimer::Reset()
{
    // cache localization strings -- in here because Reset is called when the player respawns, allowing for easy reload of tokens
    FIND_LOCALIZATION(m_wStageStart, "#MOM_Stage_Start");
    FIND_LOCALIZATION(m_wSavelocStatus, "#MOM_SavedLocation");
    FIND_LOCALIZATION(m_wPracticeMode, "#MOM_PracticeMode");
    FIND_LOCALIZATION(m_wNoTimer, "#MOM_NoTimer");
    FIND_LOCALIZATION(m_wStageNum, "#MOM_Stage");
    FIND_LOCALIZATION(m_wCheckpointNum, "#MOM_Checkpoint");

    // ensure timer StatusColor is reset upon respawn
    g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerColorReset");
}

void CHudTimer::FireGameEvent(IGameEvent* event)
{
    // if (FStrEq(event->GetName(), "saveloc_upd8"))
    {
        const bool bUsing = event->GetBool("using");
        m_iSavelocCount = event->GetInt("count");
        m_iSavelocCurrent = event->GetInt("current", -1) + 1;

        if (bUsing != m_bWasUsingSavelocMenu)
        {
            m_bWasUsingSavelocMenu = bUsing;
        }
    }
}

void CHudTimer::ApplySchemeSettings(IScheme* pScheme)
{
    Panel::ApplySchemeSettings(pScheme);
    SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
}

// This void handles playing effects for run start and run stop
void CHudTimer::MsgFunc_Timer_Event(bf_read &msg)
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return;

    const int type = msg.ReadByte();

    switch(type)
    {
    case TIMER_EVENT_STARTED:
        if (mom_timer_sound_start_enable.GetBool())
        {
            pPlayer->EmitSound("Momentum.StartTimer");
        }
        break;
    case TIMER_EVENT_FINISHED:
        if (mom_timer_sound_finish_enable.GetBool())
        {
            pPlayer->EmitSound("Momentum.FinishTimer");
        }
        break;
    case TIMER_EVENT_STOPPED:
        SetToNoTimer();
        if (mom_timer_sound_stop_enable.GetBool())
        {
            pPlayer->EmitSound("Momentum.StopTimer");
        }
        break;
    case TIMER_EVENT_FAILED:
        if (mom_timer_sound_fail_enable.GetBool())
        {
            pPlayer->EmitSound("Momentum.FailedStartTimer");
        }
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerFailStart");
    default:
        break;
    }
}

void CHudTimer::MsgFunc_Timer_Reset(bf_read &msg) { Reset(); }

void CHudTimer::SetToNoTimer()
{
    m_pMainStatusLabel->SetText(m_wNoTimer);
    m_pInfoLabel->SetText("");
    m_pSplitLabel->SetText("");
    m_pComparisonLabel->SetText("");
}

void CHudTimer::OnThink()
{
    m_pMainStatusLabel->SetFgColor(m_StatusColor);

    const auto pLocal = C_MomentumPlayer::GetLocalMomPlayer();
    if (pLocal)
    {
        const auto pEnt = pLocal->GetCurrentUIEntity();
        m_pRunStats = pLocal->GetCurrentUIEntStats();
        m_pRunData = pLocal->GetCurrentUIEntData();

        if (m_pRunData)
        {
            /* Potential states:
             * 
             * No timer running (default) => "No Timer", "", "", ""
             * No timer running but using saveloc menu => "No Timer", "Saveloc X/Y", "", ""
             * No timer running but using practice mode => "Practice Mode", "", "", ""
             * No timer running but spectating while in practice mode => "No timer", "", "", ""
             * No timer running but the map is finished => "<Current Time>", "", "", ""
             * Timer running but using practice mode => "Practice Mode", "", "", ""
             * Timer running but spectating while using practice mode => "<Current replay/ghost timer>" and splits etc
             * Timer running => "<Current Time>", "<Last Stage>", "<Last Stage Split>"
             * MOM_TODO: Timer running but using practice mode and savelocs => "Practice Mode" and "Saveloc X/Y"
             */

            if (!m_pRunData->m_bTimerRunning)
            {
                if (pLocal->m_bHasPracticeMode && pEnt->GetEntType() == RUN_ENT_PLAYER)
                {
                    m_pMainStatusLabel->SetText(m_wPracticeMode);
                }
                else if (!m_pRunData->m_bMapFinished)
                {
                    m_pMainStatusLabel->SetText(m_wNoTimer);
                }
                else
                {
                    char curTime[BUFSIZETIME];
                    MomUtil::FormatTime(pEnt->GetCurrentRunTime(), curTime, 2);
                    m_pMainStatusLabel->SetText(curTime);
                }

                if (m_bWasUsingSavelocMenu)
                    m_pInfoLabel->SetText(CConstructLocalizedString(m_wSavelocStatus, m_iSavelocCurrent, m_iSavelocCount));
                else
                    m_pInfoLabel->SetText("");

                m_pComparisonLabel->SetText("");
                m_pSplitLabel->SetText("");
            }
            else
            {
                if (pLocal->m_bHasPracticeMode && pEnt->GetEntType() == RUN_ENT_PLAYER)
                {
                    m_pMainStatusLabel->SetText(m_wPracticeMode);
                    m_pInfoLabel->SetText("");
                    m_pComparisonLabel->SetText("");
                    m_pSplitLabel->SetText("");
                }
                else
                {
                    char curTime[BUFSIZETIME];
                    MomUtil::FormatTime(pEnt->GetCurrentRunTime(), curTime, 2);
                    m_pMainStatusLabel->SetText(curTime);

                    if (m_pRunData->m_iCurrentZone > 1)
                    {
                        // Set the info label
                        const auto bTrackLinear = pLocal->m_iLinearTracks.Get(m_pRunData->m_iCurrentTrack);
                        m_pInfoLabel->SetText(CConstructLocalizedString(bTrackLinear ? m_wCheckpointNum : m_wStageNum, m_pRunData->m_iCurrentZone - 1));

                        ConVarRef timeType("mom_comparisons_time_type");
                        Color compareColor = GetFgColor();
                        char actualANSI[BUFSIZELOCL], comparisonANSI[BUFSIZELOCL];
                        g_pMOMRunCompare->GetComparisonString(timeType.GetBool() ? ZONE_TIME : TIME_OVERALL, m_pRunStats,
                                                              m_pRunData->m_iCurrentZone - 1, actualANSI, comparisonANSI, &compareColor);

                        // Set our actual time
                        m_pSplitLabel->SetText(actualANSI);

                        // Set the comparison label
                        if (g_pMOMRunCompare->LoadedComparison())
                        {
                            m_pComparisonLabel->SetFgColor(compareColor);
                            m_pComparisonLabel->SetText(comparisonANSI);
                        }
                    }
                }
            }
        }
    }
}

bool CHudTimer::ShouldDraw()
{
    return mom_hud_timer.GetBool() && CHudElement::ShouldDraw();
}

void CHudTimer::LevelShutdown()
{
    m_pRunStats = nullptr;
    m_pRunData = nullptr;
}