#include "cbase.h"

#include "mom_event_listener.h"
#include "tier0/memdbgon.h"

void C_Momentum_EventListener::Init()
{
    //add listeners for all of our custom events
    if (gameeventmanager)
    {
        gameeventmanager->AddListener(this, "timer_stopped", false);
        gameeventmanager->AddListener(this, "new_stage_enter", false);
        gameeventmanager->AddListener(this, "new_stage_exit", false);
        gameeventmanager->AddListener(this, "run_save", false);
        gameeventmanager->AddListener(this, "run_upload", false);
        gameeventmanager->AddListener(this, "timer_started", false);
        gameeventmanager->AddListener(this, "player_inside_mapzone", false);
        gameeventmanager->AddListener(this, "practice_mode", false);
        gameeventmanager->AddListener(this, "keypress", false);
    }
}

void C_Momentum_EventListener::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp("timer_stopped", pEvent->GetName()))
    {
        m_flStageStrafeSyncAvg[0] = pEvent->GetFloat("avg_sync");
        m_flStageStrafeSync2Avg[0] = pEvent->GetFloat("avg_sync2");
        //3D
        m_flStageExitSpeed[0][0] = pEvent->GetFloat("start_vel");
        m_flStageStartSpeed[0][0] = pEvent->GetFloat("end_vel");
        m_flStageVelocityAvg[0][0] = pEvent->GetFloat("avg_vel");
        m_flStageVelocityMax[0][0] = pEvent->GetFloat("max_vel");
        //2D
        m_flStageExitSpeed[0][1] = pEvent->GetFloat("start_vel_2D");
        m_flStageStartSpeed[0][1] = pEvent->GetFloat("end_vel_2D");
        m_flStageVelocityAvg[0][1] = pEvent->GetFloat("avg_vel_2D");
        m_flStageVelocityMax[0][1] = pEvent->GetFloat("max_vel_2D");
    }
    if (!Q_strcmp("new_stage_enter", pEvent->GetName()))
    {
        m_iCurrentStage = pEvent->GetInt("stage_num");
        m_flStageTime[m_iCurrentStage] = pEvent->GetFloat("stage_time");
        m_flStageStrafeSyncAvg[m_iCurrentStage] = pEvent->GetFloat("avg_sync");
        m_flStageStrafeSync2Avg[m_iCurrentStage] = pEvent->GetFloat("avg_sync2");

        m_flStageStartSpeed[m_iCurrentStage][0] = pEvent->GetFloat("stage_enter_vel");
        m_flStageVelocityAvg[m_iCurrentStage][0] = pEvent->GetFloat("avg_vel");
        m_flStageVelocityMax[m_iCurrentStage][0] = pEvent->GetFloat("max_vel");

        m_flStageStartSpeed[m_iCurrentStage][1] = pEvent->GetFloat("stage_enter_vel_2D");
        m_flStageVelocityAvg[m_iCurrentStage][1] = pEvent->GetFloat("avg_vel_2D");
        m_flStageVelocityMax[m_iCurrentStage][1] = pEvent->GetFloat("max_vel_2D");
    }
    if (!Q_strcmp("new_stage_exit", pEvent->GetName()))
    {
        m_flStageExitSpeed[m_iCurrentStage][0] = pEvent->GetFloat("stage_exit_vel");
        m_flStageExitSpeed[m_iCurrentStage][1] = pEvent->GetFloat("stage_exit_vel_2D");
    }
    if (!Q_strcmp("run_save", pEvent->GetName()))
    {
        m_bTimeDidSave = pEvent->GetBool("run_saved");
    }
    if (!Q_strcmp("run_upload", pEvent->GetName()))
    {
        m_bTimeDidUpload = pEvent->GetBool("run_posted");
        Q_strncpy(m_szRunUploadStatus, pEvent->GetString("web_msg"), sizeof(m_szRunUploadStatus));
        //MOM_TODO: potentially have stuff like new rank or something?
    }
    if (!Q_strcmp("timer_started", pEvent->GetName()))
    {
        m_bTimerIsRunning = pEvent->GetBool("timer_isrunning");
    }
    if (!Q_strcmp("player_inside_mapzone", pEvent->GetName()))
    {
        m_bPlayerInsideStartZone = pEvent->GetBool("inside_startzone");
        m_bPlayerInsideEndZone = pEvent->GetBool("inside_endzone");
        m_bMapFinished = pEvent->GetBool("map_finished"); //different from "inside endzone", this is only fired if the player finished when their timer was running
    }
    if (!Q_strcmp("practice_mode", pEvent->GetName()))
    {
        m_bPlayerHasPracticeMode = pEvent->GetBool("has_practicemode");
    }
    if (!Q_strcmp("keypress", pEvent->GetName()))
    {
        m_iStageJumps[0] = pEvent->GetInt("num_jumps");
        m_iStageStrafes[0] = pEvent->GetInt("num_strafes");
    }
}

//Interface this event listener to the DLL
static C_Momentum_EventListener s_momListener;
C_Momentum_EventListener *g_MOMEventListener = &s_momListener;