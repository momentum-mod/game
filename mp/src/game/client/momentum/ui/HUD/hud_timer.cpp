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
#include "vgui_controls/AnimationController.h"

#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"
#include "c_mom_replay_entity.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_timer, "1", FCVAR_ARCHIVE, "Toggle displaying the timer. 0 = OFF, 1 = ON\n");

class CHudTimer : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudTimer, EditablePanel);
    CHudTimer(const char *pElementName);
    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void FireGameEvent(IGameEvent* event) OVERRIDE;

    void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;
    void MsgFunc_Timer_Event(bf_read &msg);
    void MsgFunc_Timer_Reset(bf_read &msg);

    CPanelAnimationVar(Color, m_StatusColor, "StatusColor", "Mom.Panel.Fg");

  private:
    void SetToNoTimer();

    int m_iZoneCurrent;
    int m_iCurrentSpecTargetEntIndx;
    C_MomentumReplayGhostEntity *m_pSpecTarget;

    Label *m_pMainStatusLabel, *m_pInfoLabel, *m_pSplitLabel, *m_pComparisonLabel;

    wchar_t m_wStageStart[BUFSIZELOCL];
    wchar_t m_wSavelocStatus[BUFSIZELOCL];
    wchar_t m_wNoTimer[BUFSIZELOCL];
    wchar_t m_wPracticeMode[BUFSIZELOCL];
    wchar_t m_wStageNum[BUFSIZELOCL];
    wchar_t m_wCheckpointNum[BUFSIZELOCL];

    bool m_bWasUsingSavelocMenu;
    bool m_bInPracticeMode;
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

    ListenForGameEvent("saveloc_upd8");
    ListenForGameEvent("zone_enter");
    ListenForGameEvent("spec_target_updated");
    ListenForGameEvent("spec_stop");
    ListenForGameEvent("practice_mode");
    ListenForGameEvent("player_spawn");
    ListenForGameEvent("mapfinished_panel_closed");

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
    m_iCurrentSpecTargetEntIndx = -1;
    m_pRunStats = nullptr;
    m_pRunData = nullptr;
    m_pSpecTarget = nullptr;
}

void CHudTimer::Reset()
{
    m_iCurrentSpecTargetEntIndx = -1;
    m_bInPracticeMode = false;
    m_iZoneCurrent = 1;
    m_pSpecTarget = nullptr;

    // cache localization strings -- in here because Reset is called when the player respawns, allowing for easy reload of tokens
    FIND_LOCALIZATION(m_wStageStart, "#MOM_Stage_Start");
    FIND_LOCALIZATION(m_wSavelocStatus, "#MOM_SavedLocation");
    FIND_LOCALIZATION(m_wPracticeMode, "#MOM_PracticeMode");
    FIND_LOCALIZATION(m_wNoTimer, "#MOM_NoTimer");
    FIND_LOCALIZATION(m_wStageNum, "#MOM_Stage");
    FIND_LOCALIZATION(m_wCheckpointNum, "#MOM_Checkpoint");

    if (!(m_pRunData && m_pRunData->m_bTimerRunning))
    {
        SetToNoTimer();
    }
}

void CHudTimer::FireGameEvent(IGameEvent* event)
{
    const char *pName = event->GetName();
    if (FStrEq(pName, "zone_enter"))
    {
        const int entIndx = event->GetInt("ent");

        if (entIndx == engine->GetLocalPlayer() || entIndx == m_iCurrentSpecTargetEntIndx)
        {
            const int zoneNum = event->GetInt("num");
            const bool bChanged = m_iZoneCurrent != zoneNum;
            m_iZoneCurrent = zoneNum;

            if (zoneNum == 1 && !m_bInPracticeMode) // Start trigger
                SetToNoTimer();

            if (m_pRunData && m_pRunData->m_bTimerRunning && bChanged && m_iZoneCurrent > 1)
            {
                // Set the info label
                m_pInfoLabel->SetText(CConstructLocalizedString(g_MOMEventListener->m_bMapIsLinear ? m_wCheckpointNum : m_wStageNum, m_iZoneCurrent - 1));

                ConVarRef timeType("mom_comparisons_time_type");
                // This void works even if there is no comparison loaded
                Color compareColor = GetFgColor();
                char actualANSI[BUFSIZELOCL], comparisonANSI[BUFSIZELOCL];
                g_pMOMRunCompare->GetComparisonString(timeType.GetBool() ? ZONE_TIME : TIME_OVERALL, m_pRunStats,
                                                     m_iZoneCurrent - 1, actualANSI, comparisonANSI, &compareColor);

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
    else if (FStrEq(pName, "practice_mode"))
    {
        m_bInPracticeMode = event->GetBool("enabled");
        if (m_bInPracticeMode)
            m_pMainStatusLabel->SetText(m_wPracticeMode);
        else if (!m_pRunData->m_bTimerRunning)
            SetToNoTimer();
    }
    else if (FStrEq(pName, "spec_target_updated"))
    {
        const auto pLocal = C_MomentumPlayer::GetLocalMomPlayer();
        // Note: this has to be delayed until OnThink can get at it, because the client-side Ghost ent isn't created yet...
        m_iCurrentSpecTargetEntIndx = pLocal->GetSpecEntIndex();
        // Default it all to nullptr for now, just in case they're spectating an online ghost anyways
        m_pSpecTarget = nullptr;
        m_pRunStats = nullptr;
        m_pRunData = nullptr;
        m_pInfoLabel->SetText("");
        m_pSplitLabel->SetText("");
        m_pComparisonLabel->SetText("");
    }
    else if (FStrEq(pName, "spec_stop") || FStrEq(pName, "player_spawn"))
    {
        const auto pLocal = C_MomentumPlayer::GetLocalMomPlayer();
        m_pRunData = pLocal->GetRunEntData();
        m_pRunStats = &pLocal->m_RunStats;
        if (!m_pRunData->m_bTimerRunning)
            SetToNoTimer();
    }
    else if (FStrEq(pName, "mapfinished_panel_closed"))
    {
        Reset();
    }
    else if (FStrEq(pName, "saveloc_upd8"))
    {
        const bool bUsing = event->GetBool("using");
        const int count = event->GetInt("count");
        const int current = event->GetInt("current", -1) + 1;

        if (bUsing != m_bWasUsingSavelocMenu)
        {
            m_bWasUsingSavelocMenu = bUsing;

            m_pMainStatusLabel->SetText(m_wNoTimer);

            if (!bUsing)
                m_pInfoLabel->SetText("");
        }

        if (m_bWasUsingSavelocMenu)
            m_pInfoLabel->SetText(CConstructLocalizedString(m_wSavelocStatus, current, count));
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

    if (type == TIMER_EVENT_STARTED)
    {
        pPlayer->EmitSound("Momentum.StartTimer");
    }
    else if (type == TIMER_EVENT_FINISHED)
    {
        pPlayer->EmitSound("Momentum.FinishTimer");
    }
    else if (type == TIMER_EVENT_STOPPED)
    {
        SetToNoTimer();
        pPlayer->EmitSound("Momentum.StopTimer");
    }
    else if (type == TIMER_EVENT_FAILED)
    {
        pPlayer->EmitSound("Momentum.FailedStartTimer");
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerFailStart");
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
        if (m_iCurrentSpecTargetEntIndx != -1 && !m_pSpecTarget)
        {
            if (pEnt->GetEntType() == RUN_ENT_REPLAY)
            {
                m_pSpecTarget = static_cast<C_MomentumReplayGhostEntity*>(pEnt);
                m_pRunStats = pEnt->GetRunStats();
                m_pRunData = pEnt->GetRunEntData();
            }
        }

        // Format the run's time
        if (m_pRunData && !m_bInPracticeMode)
        {
            if (m_pRunData->m_bTimerRunning || m_pRunData->m_bMapFinished)
            {
                char curTime[BUFSIZETIME];
                g_pMomentumUtil->FormatTime(pEnt->GetCurrentRunTime(), curTime, 2);
                m_pMainStatusLabel->SetText(curTime);
            }
        }
    }
}

bool CHudTimer::ShouldDraw()
{
    return mom_hud_timer.GetBool() && CHudElement::ShouldDraw();
}