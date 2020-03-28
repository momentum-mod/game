#include "cbase.h"

#include "hud_mapfinished.h"
#include <game/client/iviewport.h>
#include "spectate/mom_spectator_gui.h"
#include "clientmode.h"
#include "mom_player_shared.h"

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "vgui_controls/Tooltip.h"
#include <vgui/IInput.h>
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"

#include "util/mom_util.h"
#include "c_mom_replay_entity.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_mapfinished_movement_enable, "0", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                          "Toggles being able to move. 0 = OFF, 1 = ON\n");

DECLARE_HUDELEMENT_DEPTH(CHudMapFinishedDialog, 70);

CHudMapFinishedDialog::CHudMapFinishedDialog(const char *pElementName) : CHudElement(pElementName),
    BaseClass(g_pClientMode->GetViewport(), "CHudMapFinishedDialog"),
    m_cvarVelType("mom_hud_velocity_type")
{
    SetHiddenBits(HIDEHUD_LEADERBOARDS);
    SetProportional(true);
    SetSize(10, 10); // Fix "not sized yet" spew
    m_pRunStats = nullptr;
    m_bIsGhost = false;
    m_bCanClose = false;
    m_iCurrentPage = 0;

    ListenForGameEvent("spec_start");
    ListenForGameEvent("spec_stop");
    ListenForGameEvent("replay_save");
    ListenForGameEvent("run_submit");
    ListenForGameEvent("run_upload");

    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    m_pClosePanelButton = new ImagePanel(this, "Close_Panel");
    m_pNextZoneButton = new ImagePanel(this, "Next_Zone");
    m_pPrevZoneButton = new ImagePanel(this, "Prev_Zone");
    m_pPlayReplayButton = new ImagePanel(this, "Replay_Icon");
    m_pRepeatButton = new ImagePanel(this, "Repeat_Button");
    m_pDetachMouseLabel = new Label(this, "Detach_Mouse", "#MOM_MF_DetachMouse");
    m_pCurrentZoneLabel = new Label(this, "Current_Zone", "#MOM_MF_OverallStats");
    m_pZoneOverallTime = new Label(this, "Zone_Overall_Time", "#MOM_MF_RunTime");
    m_pZoneEnterTime = new Label(this, "Zone_Enter_Time", "#MOM_MF_Zone_Enter");
    m_pZoneJumps = new Label(this, "Zone_Jumps", "#MOM_MF_Jumps");
    m_pZoneStrafes = new Label(this, "Zone_Strafes", "#MOM_MF_Strafes");
    m_pZoneVelEnter = new Label(this, "Zone_Vel_Enter", "#MOM_MF_Velocity_Enter");
    m_pZoneVelExit = new Label(this, "Zone_Vel_Exit", "#MOM_MF_Velocity_Exit");
    m_pZoneVelAvg = new Label(this, "Zone_Vel_Avg", "#MOM_MF_Velocity_Avg");
    m_pZoneVelMax = new Label(this, "Zone_Vel_Max", "#MOM_MF_Velocity_Max");
    m_pZoneSync1 = new Label(this, "Zone_Sync1", "#MOM_MF_Sync1");
    m_pZoneSync2 = new Label(this, "Zone_Sync2", "#MOM_MF_Sync2");
    m_pRunSaveStatus = new Label(this, "Run_Save_Status", "#MOM_MF_RunNotSaved");
    m_pRunUploadStatus = new Label(this, "Run_Upload_Status", "#MOM_MF_RunNotUploaded");
    m_pXPGainCosmetic = new Label(this, "XP_Gain_Cos", "#MOM_MF_XPGainCos");
    m_pXPGainRank = new Label(this, "XP_Gain_Rank", "#MOM_MF_XPGainRank");
    m_pLevelGain = new Label(this, "Cos_Level_Gain", "#MOM_MF_CosLvlGain");

    LoadControlSettings("resource/ui/MapFinishedDialog.res");

    m_pNextZoneButton->SetMouseInputEnabled(true);
    m_pNextZoneButton->InstallMouseHandler(this);
    m_pPrevZoneButton->SetMouseInputEnabled(true);
    m_pPrevZoneButton->InstallMouseHandler(this);
    m_pPlayReplayButton->SetMouseInputEnabled(true);
    m_pPlayReplayButton->InstallMouseHandler(this);
    m_pRepeatButton->SetMouseInputEnabled(true);
    m_pRepeatButton->InstallMouseHandler(this);
    m_pClosePanelButton->SetMouseInputEnabled(true);
    m_pClosePanelButton->InstallMouseHandler(this);

    SetPaintBackgroundEnabled(true);
    SetPaintBackgroundType(2);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
}

CHudMapFinishedDialog::~CHudMapFinishedDialog()
{
    m_pRunStats = nullptr;
}

void CHudMapFinishedDialog::FireGameEvent(IGameEvent* pEvent)
{
    if (FStrEq(pEvent->GetName(), "replay_save"))
    {
        m_bCanClose = true;

        SetRunSaved(pEvent->GetBool("save"));
        SetCurrentPage(m_iCurrentPage);
        // MOM_TODO: There's a file name parameter as well, do we want to use it here?
    }
    else if (FStrEq(pEvent->GetName(), "run_submit"))
    {
        SetRunSubmitted((RunSubmitState_t) pEvent->GetInt("state"));
    }
    else if (FStrEq(pEvent->GetName(), "run_upload"))
    {
        const auto bPosted = pEvent->GetBool("run_posted");
        SetRunUploaded(bPosted);
        if (bPosted)
        {
            const auto cXP = pEvent->GetInt("cos_xp");
            m_pXPGainCosmetic->SetVisible(cXP > 0);
            if (cXP)
            {
                m_pXPGainCosmetic->SetText(CConstructLocalizedString(m_wXPGainCos, cXP));
            }

            const auto lvlGain = pEvent->GetInt("lvl_gain");
            m_pLevelGain->SetVisible(lvlGain > 0);
            if (lvlGain)
            {
                m_pLevelGain->SetText(CConstructLocalizedString(m_wLevelGain, lvlGain));
            }

            const auto rXP = pEvent->GetInt("rank_xp");
            m_pXPGainRank->SetVisible(rXP > 0);
            if (rXP)
            {
                m_pXPGainRank->SetText(CConstructLocalizedString(m_wXPGainRank, rXP));
            }
        }
    }
    else // Spec start/stop
    {
        const bool bSpecStart = FStrEq(pEvent->GetName(), "spec_start");
        m_pRepeatButton->GetTooltip()->SetText(bSpecStart ? "#MOM_MF_Restart_Replay" : "#MOM_MF_Restart_Map");
    }
}

void CHudMapFinishedDialog::LevelShutdown()
{
    m_pRunStats = nullptr;
    m_pRunData = nullptr;
    m_bIsGhost = false;
    m_bCanClose = false;
}

void CHudMapFinishedDialog::OnThink()
{
    BaseClass::OnThink();

    m_pPlayReplayButton->SetVisible(!m_bIsGhost);
    m_pRunUploadStatus->SetVisible(!m_bIsGhost);
    m_pRunSaveStatus->SetVisible(!m_bIsGhost);
}

void CHudMapFinishedDialog::SetMouseInputEnabled(bool state)
{
    BaseClass::SetMouseInputEnabled(state);
    m_pDetachMouseLabel->SetVisible(!state);
}

bool CHudMapFinishedDialog::ShouldDraw()
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return false;

    m_pRunData = pPlayer->GetCurrentUIEntData();
    m_pRunStats = pPlayer->GetCurrentUIEntStats();
    m_bIsGhost = pPlayer->GetCurrentUIEntity()->GetEntType() >= RUN_ENT_GHOST;

    const bool shouldDrawLocal = m_pRunData && m_pRunData->m_bMapFinished && m_pRunStats;
    if (!shouldDrawLocal)
        SetMouseInputEnabled(false);
    return CHudElement::ShouldDraw() && shouldDrawLocal;
}

void CHudMapFinishedDialog::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetBgColor(GetSchemeColor("MOM.Panel.Bg", pScheme));
    m_pDetachMouseLabel->SetFont(m_hTextFont);
    m_pCurrentZoneLabel->SetFont(m_hTextFont);
    m_pZoneOverallTime->SetFont(m_hTextFont);
    m_pZoneEnterTime->SetFont(m_hTextFont);
    m_pZoneJumps->SetFont(m_hTextFont);
    m_pZoneStrafes->SetFont(m_hTextFont);
    m_pZoneVelEnter->SetFont(m_hTextFont);
    m_pZoneVelExit->SetFont(m_hTextFont);
    m_pZoneVelAvg->SetFont(m_hTextFont);
    m_pZoneVelMax->SetFont(m_hTextFont);
    m_pZoneSync1->SetFont(m_hTextFont);
    m_pZoneSync2->SetFont(m_hTextFont);
    m_pRunSaveStatus->SetFont(m_hTextFont);
    m_pRunUploadStatus->SetFont(m_hTextFont);
    m_pXPGainCosmetic->SetFont(m_hTextFont);
    m_pXPGainRank->SetFont(m_hTextFont);
    m_pLevelGain->SetFont(m_hTextFont);
    m_pLevelGain->SetFgColor(COLOR_GREEN);
}

void CHudMapFinishedDialog::SetRunSaved(bool bState)
{
    m_pRunSaveStatus->SetText(bState ? "#MOM_MF_RunSaved" : "#MOM_MF_RunNotSaved");
    m_pRunSaveStatus->SetFgColor(bState ? COLOR_GREEN : COLOR_RED);
}

void CHudMapFinishedDialog::SetRunUploaded(bool bState)
{
    m_pRunUploadStatus->SetText(bState ? "#MOM_MF_RunUploaded" : "#MOM_MF_RunNotUploaded");
    m_pRunUploadStatus->SetFgColor(bState ? COLOR_GREEN : COLOR_RED);

    // Visibility for these will be determined by the run_upload event
    m_pXPGainCosmetic->SetVisible(false);
    m_pXPGainRank->SetVisible(false);
    m_pLevelGain->SetVisible(false);
}

void CHudMapFinishedDialog::SetRunSubmitted(RunSubmitState_t state)
{
    if (state <= RUN_SUBMIT_UNKNOWN || state >= RUN_SUBMIT_COUNT)
        return;

    m_pRunUploadStatus->SetText(g_szSubmitStates[state]);
    m_pRunUploadStatus->SetFgColor(state == RUN_SUBMIT_SUCCESS ? COLOR_ORANGE : COLOR_RED);

    // Visibility for these will be determined by the run_upload event
    m_pXPGainCosmetic->SetVisible(false);
    m_pXPGainRank->SetVisible(false);
    m_pLevelGain->SetVisible(false);
}

bool CHudMapFinishedDialog::ClosePanel()
{
    if (!m_bCanClose && !m_bIsGhost)
        return false;

    SetMouseInputEnabled(false);
    SetRunSaved(false);
    SetRunUploaded(false);
    m_bCanClose = false;

    return true;
}

void CHudMapFinishedDialog::FirePanelClosedEvent(bool bRestartingMap)
{
    IGameEvent *pClosePanel = gameeventmanager->CreateEvent("mapfinished_panel_closed");
    if (pClosePanel)
    {
        pClosePanel->SetBool("restart", bRestartingMap);
        // Fire this event so other classes can get at this
        gameeventmanager->FireEvent(pClosePanel);
    }
}

void CHudMapFinishedDialog::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        const auto panelOver = input()->GetMouseOver();
        if (panelOver == m_pPlayReplayButton->GetVPanel())
        {
            engine->ClientCmd_Unrestricted("mom_replay_play_loaded\n");
            ClosePanel();
        }
        else if (panelOver == m_pNextZoneButton->GetVPanel())
        {
            //MOM_TODO (beta+): Play animations?
            const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
            if (pPlayer)
                SetCurrentPage((m_iCurrentPage + 1) % (pPlayer->m_iZoneCount[m_pRunData->m_iCurrentTrack] + 1));
        }
        else if (panelOver == m_pPrevZoneButton->GetVPanel())
        {
            //MOM_TODO: (beta+) play animations?
            const int newPage = m_iCurrentPage - 1;
            const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
            if (pPlayer)
                SetCurrentPage(newPage < 0 ? pPlayer->m_iZoneCount[m_pRunData->m_iCurrentTrack] : newPage);
        }
        else if (panelOver == m_pRepeatButton->GetVPanel())
        {
            engine->ServerCmd(m_bIsGhost ? "mom_replay_restart" : "mom_restart");
            if (ClosePanel())
                FirePanelClosedEvent(true);
        }
        else if (panelOver == m_pClosePanelButton->GetVPanel())
        {
            if (ClosePanel())
                FirePanelClosedEvent(false);
        }
    }
}

void CHudMapFinishedDialog::Reset()
{
    //default values
    Q_strncpy(m_pszEndRunTime, "00:00:00.000", sizeof(m_pszEndRunTime));
    SetRunSaved(false);
    SetRunUploaded(false);
    m_bCanClose = false;

    // --- cache localization tokens ---
    FIND_LOCALIZATION(m_pwCurrentPageOverall, "#MOM_MF_OverallStats");
    FIND_LOCALIZATION(m_pwCurrentPageZoneNum, "#MOM_MF_ZoneNum");
    FIND_LOCALIZATION(m_pwOverallTime, "#MOM_MF_RunTime");
    FIND_LOCALIZATION(m_pwZoneEnterTime, "#MOM_MF_Zone_Enter");
    FIND_LOCALIZATION(m_pwZoneTime, "#MOM_MF_Time_Zone");
    FIND_LOCALIZATION(m_pwVelAvg, "#MOM_MF_Velocity_Avg");
    FIND_LOCALIZATION(m_pwVelMax, "#MOM_MF_Velocity_Max");
    FIND_LOCALIZATION(m_pwVelZoneEnter, "#MOM_MF_Velocity_Enter");
    FIND_LOCALIZATION(m_pwVelZoneExit, "#MOM_MF_Velocity_Exit");
    FIND_LOCALIZATION(m_pwJumpsOverall, "#MOM_MF_JumpCount");
    FIND_LOCALIZATION(m_pwJumpsZone, "#MOM_MF_Jumps");
    FIND_LOCALIZATION(m_pwStrafesOverall, "#MOM_MF_StrafeCount");
    FIND_LOCALIZATION(m_pwStrafesZone, "#MOM_MF_Strafes");
    FIND_LOCALIZATION(m_pwSync1Overall, "#MOM_MF_AvgSync");
    FIND_LOCALIZATION(m_pwSync1Zone, "#MOM_MF_Sync1");
    FIND_LOCALIZATION(m_pwSync2Overall, "#MOM_MF_AvgSync2");
    FIND_LOCALIZATION(m_pwSync2Zone, "#MOM_MF_Sync2");
    FIND_LOCALIZATION(m_wXPGainCos, "#MOM_MF_XPGainCos");
    FIND_LOCALIZATION(m_wXPGainRank, "#MOM_MF_XPGainRank");
    FIND_LOCALIZATION(m_wLevelGain, "#MOM_MF_CosLvlGain");
}

void CHudMapFinishedDialog::SetVisible(bool bVisible)
{
    BaseClass::SetVisible(bVisible);
    //We reset the page to 0 when this this panel is shown because Reset() is not always called.
    if (bVisible)
    {
        SetCurrentPage(0);

        const auto pSpecUI = gViewPortInterface->FindPanelByName(PANEL_SPECGUI);
        if (pSpecUI && pSpecUI->IsVisible() && ipanel()->IsMouseInputEnabled(pSpecUI->GetVPanel()))
            SetMouseInputEnabled(true);
    }
}

void CHudMapFinishedDialog::SetCurrentPage(int pageNum)
{
    m_iCurrentPage = pageNum;

    wchar_t unicodeTime[BUFSIZETIME];
    //"Time:" shows up when m_iCurrentPage  == 0
    if (m_iCurrentPage < 1)// == 0, but I'm lazy to do an else-if
    {
        m_pCurrentZoneLabel->SetText(m_pwCurrentPageOverall);

        MomUtil::FormatTime(m_pRunData ? float(m_pRunData->m_iRunTime) * m_pRunData->m_flTickRate : 0.0f, m_pszEndRunTime);
        ANSI_TO_UNICODE(m_pszEndRunTime, unicodeTime);
        m_pZoneOverallTime->SetText(CConstructLocalizedString(m_pwOverallTime, unicodeTime));//"Time" (overall run time)

        m_pZoneEnterTime->SetVisible(false);
        m_pZoneEnterTime->SetEnabled(false);
    }
    else
    {
        m_pCurrentZoneLabel->SetText(CConstructLocalizedString(m_pwCurrentPageZoneNum, m_iCurrentPage));

        //"Zone Time:" shows up when m_iCurrentPage > 0
        char ansiTime[BUFSIZETIME];
        MomUtil::FormatTime(m_pRunStats ? float(m_pRunStats->GetZoneTicks(m_iCurrentPage)) * m_pRunData->m_flTickRate : 0.0f, ansiTime);
        ANSI_TO_UNICODE(ansiTime, unicodeTime);
        m_pZoneOverallTime->SetText(CConstructLocalizedString(m_pwZoneTime, unicodeTime));//"Zone time" (time for that zone)

        //"Zone Enter Time:" shows up when m_iCurrentPage > 1
        if (m_iCurrentPage > 1)
        {
            m_pZoneEnterTime->SetEnabled(true);
            m_pZoneEnterTime->SetVisible(true);
            MomUtil::FormatTime(m_pRunStats ? float(m_pRunStats->GetZoneEnterTick(m_iCurrentPage)) * m_pRunData->m_flTickRate : 0.0f, ansiTime);
            ANSI_TO_UNICODE(ansiTime, unicodeTime);
            m_pZoneEnterTime->SetText(CConstructLocalizedString(m_pwZoneEnterTime, unicodeTime));//"Zone enter time:" (time entered that zone)
        }
        else
        {
            m_pZoneEnterTime->SetVisible(false);
            m_pZoneEnterTime->SetEnabled(false);
        }
    }

    //MOM_TODO: Set every label's Y pos higher if there's no ZoneEnterTime visible

    m_pZoneJumps->SetText(CConstructLocalizedString(m_iCurrentPage == 0 ? m_pwJumpsOverall : m_pwJumpsZone,
                                                    m_pRunStats ? m_pRunStats->GetZoneJumps(m_iCurrentPage) : 0));

    m_pZoneStrafes->SetText(CConstructLocalizedString(m_iCurrentPage == 0 ? m_pwStrafesOverall : m_pwStrafesZone,
                                                      m_pRunStats ? m_pRunStats->GetZoneStrafes(m_iCurrentPage) : 0));

    m_pZoneSync1->SetText(CConstructLocalizedString(m_iCurrentPage == 0 ? m_pwSync1Overall : m_pwSync1Zone,
                                                    m_pRunStats ? m_pRunStats->GetZoneStrafeSyncAvg(m_iCurrentPage) : 0.0f));

    m_pZoneSync2->SetText(CConstructLocalizedString(m_iCurrentPage == 0 ? m_pwSync2Overall : m_pwSync2Zone,
                                                    m_pRunStats ? m_pRunStats->GetZoneStrafeSync2Avg(m_iCurrentPage) : 0.0f));

    int velType = m_cvarVelType.GetInt();
    m_pZoneVelEnter->SetText(CConstructLocalizedString(m_pwVelZoneEnter, m_pRunStats ? m_pRunStats->GetZoneEnterSpeed(m_iCurrentPage, velType) : 0.0f));

    m_pZoneVelExit->SetText(CConstructLocalizedString(m_pwVelZoneExit, m_pRunStats ? m_pRunStats->GetZoneExitSpeed(m_iCurrentPage, velType) : 0.0f));

    m_pZoneVelAvg->SetText(CConstructLocalizedString(m_pwVelAvg, m_pRunStats ? m_pRunStats->GetZoneVelocityAvg(m_iCurrentPage, velType) : 0.0f));

    m_pZoneVelMax->SetText(CConstructLocalizedString(m_pwVelMax, m_pRunStats ? m_pRunStats->GetZoneVelocityMax(m_iCurrentPage, velType) : 0.0f));
}