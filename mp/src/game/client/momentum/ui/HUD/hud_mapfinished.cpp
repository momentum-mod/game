#include "cbase.h"

#include "hud_mapfinished.h"
#include <vgui_controls/Label.h>
#include <game/client/iviewport.h>
#include "spectate/momSpectatorGUI.h"
#include "clientmode.h"
#include "mom_player_shared.h"

#include <vgui_controls/ImagePanel.h>
#include "vgui_controls/Tooltip.h"
#include <vgui/IInput.h>
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"

#include "mom_event_listener.h"
#include "util/mom_util.h"
#include "c_mom_replay_entity.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT_DEPTH(CHudMapFinishedDialog, 70);

//NOTE: The "CHudMapFinishedDialog" (main panel) control settings are found in MapFinishedDialog.res
CHudMapFinishedDialog::CHudMapFinishedDialog(const char *pElementName) : CHudElement(pElementName),
    BaseClass(g_pClientMode->GetViewport(), "CHudMapFinishedDialog"), m_pPlayer(nullptr)
{
    SetHiddenBits(HIDEHUD_LEADERBOARDS);
    SetProportional(true);
    SetSize(10, 10); // Fix "not sized yet" spew
    m_pRunStats = nullptr;
    m_bIsGhost = false;
    m_iCurrentPage = 0;

    ListenForGameEvent("timer_event");
    ListenForGameEvent("replay_save");
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
    if (FStrEq(pEvent->GetName(), "timer_event"))
    {
        const int type = pEvent->GetInt("type");
        //We only care when timer is stopped
        if (type == TIMER_EVENT_FINISHED)
        {
            if (m_pPlayer)
            {
                ConVarRef hvel("mom_hud_speedometer_hvel");
                m_iVelocityType = hvel.GetBool();

                C_MomentumReplayGhostEntity *pGhost = m_pPlayer->GetReplayEnt();
                if (pGhost)
                {
                    m_pRunStats = &pGhost->m_RunStats;
                    m_pRunData = pGhost->GetRunEntData();
                    m_bIsGhost = true;
                }
                else
                {
                    m_pRunStats = &m_pPlayer->m_RunStats;
                    m_pRunData = m_pPlayer->GetRunEntData();
                    m_bIsGhost = false;
                }

                m_pPlayReplayButton->SetVisible(!m_bIsGhost);
                m_pRunUploadStatus->SetVisible(!m_bIsGhost);
                m_pRunSaveStatus->SetVisible(!m_bIsGhost);
                m_pRepeatButton->GetTooltip()->SetText(m_bIsGhost ? "#MOM_MF_Restart_Replay" : "#MOM_MF_Restart_Map");

                CMOMSpectatorGUI *pPanel = dynamic_cast<CMOMSpectatorGUI*>(gViewPortInterface->FindPanelByName(PANEL_SPECGUI));
                if (pPanel && pPanel->IsVisible())
                    SetMouseInputEnabled(pPanel->IsMouseInputEnabled());
            }
        }
    }
    else if (FStrEq(pEvent->GetName(), "replay_save"))
    {
        SetRunSaved(pEvent->GetBool("save"));
        SetCurrentPage(m_iCurrentPage);
        // MOM_TODO: There's a file name parameter as well, do we want to use it here?
    }
    else if (FStrEq(pEvent->GetName(), "run_upload"))
    {
        SetRunUploaded(pEvent->GetBool("run_posted"));
    }
}

void CHudMapFinishedDialog::LevelInitPostEntity()
{
    m_pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
}

void CHudMapFinishedDialog::LevelShutdown()
{
    m_pPlayer = nullptr;
    m_pRunStats = nullptr;
}

void CHudMapFinishedDialog::SetMouseInputEnabled(bool state)
{
    BaseClass::SetMouseInputEnabled(state);
    m_pDetachMouseLabel->SetVisible(!state);
}

bool CHudMapFinishedDialog::ShouldDraw()
{
    bool shouldDrawLocal = false;
    if (m_pPlayer)
    {
        C_MomentumReplayGhostEntity *pGhost = m_pPlayer->GetReplayEnt();
        CMomRunEntityData *pData = (pGhost ? pGhost->GetRunEntData() : m_pPlayer->GetRunEntData());
        shouldDrawLocal = pData && pData->m_bMapFinished;
    }

    if (!shouldDrawLocal)
        SetMouseInputEnabled(false);

    return CHudElement::ShouldDraw() && shouldDrawLocal && m_pRunStats;
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

    m_pPlayReplayButton->GetTooltip()->SetText("#MOM_MF_PlayReplay");
    m_pNextZoneButton->GetTooltip()->SetText("#MOM_MF_Right_Arrow");
    m_pPrevZoneButton->GetTooltip()->SetText("#MOM_MF_Left_Arrow");
}

void CHudMapFinishedDialog::SetRunSaved(bool bState)
{
    m_bRunSaved = bState;
    m_pRunSaveStatus->SetText(m_bRunSaved ? "#MOM_MF_RunSaved" : "#MOM_MF_RunNotSaved");
    m_pRunSaveStatus->SetFgColor(m_bRunSaved ? COLOR_GREEN : COLOR_RED);
}

void CHudMapFinishedDialog::SetRunUploaded(bool bState)
{
    m_bRunUploaded = bState;
    //MOM_TODO: Should we have custom error messages here? One for server not responding, one for failed accept, etc
    m_pRunUploadStatus->SetText(m_bRunUploaded ? "#MOM_MF_RunUploaded" : "#MOM_MF_RunNotUploaded");
    m_pRunUploadStatus->SetFgColor(m_bRunUploaded ? COLOR_GREEN : COLOR_RED);
}

inline void FireMapFinishedClosedEvent(bool restart)
{
    IGameEvent *pClosePanel = gameeventmanager->CreateEvent("mapfinished_panel_closed");
    if (pClosePanel)
    {
        pClosePanel->SetBool("restart", restart);
        // Fire this event so other classes can get at this
        gameeventmanager->FireEvent(pClosePanel);
    }
}

void CHudMapFinishedDialog::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        VPANEL over = input()->GetMouseOver();
        if (over == m_pPlayReplayButton->GetVPanel())
        {
            SetMouseInputEnabled(false);
            engine->ServerCmd("mom_replay_play_loaded");
            SetRunSaved(false);
            SetRunUploaded(false);
        }
        else if (over == m_pNextZoneButton->GetVPanel())
        {
            //MOM_TODO (beta+): Play animations?
            SetCurrentPage((m_iCurrentPage + 1) % (g_MOMEventListener->m_iMapZoneCount + 1));
        }
        else if (over == m_pPrevZoneButton->GetVPanel())
        {
            //MOM_TODO: (beta+) play animations?
            int newPage = m_iCurrentPage - 1;
            SetCurrentPage(newPage < 0 ? g_MOMEventListener->m_iMapZoneCount : newPage);
        }
        else if (over == m_pRepeatButton->GetVPanel())
        {
            SetMouseInputEnabled(false);
            //The player either wants to repeat the replay (if spectating), or restart the map (not spec)
            engine->ServerCmd(m_bIsGhost ? "mom_replay_restart" : "mom_restart");
            FireMapFinishedClosedEvent(true);
            SetRunSaved(false);
            SetRunUploaded(false);
        }
        else if (over == m_pClosePanelButton->GetVPanel())
        {
            //This is where we unload comparisons, as well as the ghost if the player was speccing it
            SetMouseInputEnabled(false);
            FireMapFinishedClosedEvent(false);
            SetRunSaved(false);
            SetRunUploaded(false);
        }
    }
}

void CHudMapFinishedDialog::Init()
{
    Reset();
    // --- cache localization tokens ---
    // Stats
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
}

void CHudMapFinishedDialog::Reset()
{
    //default values
    Q_strncpy(m_pszEndRunTime, "00:00:00.000", sizeof(m_pszEndRunTime));
}

void CHudMapFinishedDialog::SetVisible(bool b)
{
    BaseClass::SetVisible(b);
    //We reset the page to 0 when this this panel is shown because Reset() is not always called.
    if (b)
        SetCurrentPage(0);
}

void CHudMapFinishedDialog::SetCurrentPage(int pageNum)
{
    m_iCurrentPage = pageNum;

    wchar_t unicodeTime[BUFSIZETIME];
    //"Time:" shows up when m_iCurrentPage  == 0
    if (m_iCurrentPage < 1)// == 0, but I'm lazy to do an else-if
    {
        m_pCurrentZoneLabel->SetText(m_pwCurrentPageOverall);

        g_pMomentumUtil->FormatTime(m_pRunData ? m_pRunData->m_flRunTime : 0.0f, m_pszEndRunTime);
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
        g_pMomentumUtil->FormatTime(m_pRunStats ? m_pRunStats->GetZoneTime(m_iCurrentPage) : 0.0f, ansiTime);
        ANSI_TO_UNICODE(ansiTime, unicodeTime);
        m_pZoneOverallTime->SetText(CConstructLocalizedString(m_pwZoneTime, unicodeTime));//"Zone time" (time for that zone)

        //"Zone Enter Time:" shows up when m_iCurrentPage > 1
        if (m_iCurrentPage > 1)
        {
            m_pZoneEnterTime->SetEnabled(true);
            m_pZoneEnterTime->SetVisible(true);
            g_pMomentumUtil->FormatTime(m_pRunStats ? m_pRunStats->GetZoneEnterTime(m_iCurrentPage) : 0.0f, ansiTime);
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

    m_pZoneVelEnter->SetText(CConstructLocalizedString(m_pwVelZoneEnter, m_pRunStats ? m_pRunStats->GetZoneEnterSpeed(m_iCurrentPage, m_iVelocityType) : 0.0f));

    m_pZoneVelExit->SetText(CConstructLocalizedString(m_pwVelZoneExit, m_pRunStats ? m_pRunStats->GetZoneExitSpeed(m_iCurrentPage, m_iVelocityType) : 0.0f));

    m_pZoneVelAvg->SetText(CConstructLocalizedString(m_pwVelAvg, m_pRunStats ? m_pRunStats->GetZoneVelocityAvg(m_iCurrentPage, m_iVelocityType) : 0.0f));

    m_pZoneVelMax->SetText(CConstructLocalizedString(m_pwVelMax, m_pRunStats ? m_pRunStats->GetZoneVelocityMax(m_iCurrentPage, m_iVelocityType) : 0.0f));
}