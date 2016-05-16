#include "cbase.h"

#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "tier0/memdbgon.h"

void C_Momentum_EventListener::Init()
{
    //add listeners for all of our custom events
    ListenForGameEvent("timer_stopped");
    ListenForGameEvent("new_stage_enter");
    ListenForGameEvent("new_stage_exit");
    ListenForGameEvent("run_save");
    ListenForGameEvent("run_upload");
    ListenForGameEvent("timer_state");
    ListenForGameEvent("practice_mode");
    ListenForGameEvent("keypress");
}

void C_Momentum_EventListener::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp("timer_stopped", pEvent->GetName()))
    {
        m_flStageStrafeSyncAvg[0] = pEvent->GetFloat("avg_sync");
        m_flStageStrafeSync2Avg[0] = pEvent->GetFloat("avg_sync2");
        //3D
        m_flStageEnterSpeed[0][0] = pEvent->GetFloat("start_vel");
        m_flStageExitSpeed[0][0] = pEvent->GetFloat("end_vel");
        m_flStageVelocityAvg[0][0] = pEvent->GetFloat("avg_vel");
        m_flStageVelocityMax[0][0] = pEvent->GetFloat("max_vel");
        //2D
        m_flStageEnterSpeed[0][1] = pEvent->GetFloat("start_vel_2D");
        m_flStageExitSpeed[0][1] = pEvent->GetFloat("end_vel_2D");
        m_flStageVelocityAvg[0][1] = pEvent->GetFloat("avg_vel_2D");
        m_flStageVelocityMax[0][1] = pEvent->GetFloat("max_vel_2D");
    }
    else if (!Q_strcmp("new_stage_enter", pEvent->GetName()))
    {
        //NOTE: THE ONLY STAT BELOW THAT REQUIRES THE CURRENT STAGE GIVEN IN "stage_num" IS THE ENTER TIME!
        //EVERYTHING ELSE IS m_iCurrentStage - 1 !

        int currentStage = pEvent->GetInt("stage_num");
        //Note: stage_enter_time will NOT change upon multiple entries to the same stage trigger (only set once per run)
        m_flStageEnterTime[currentStage] = pEvent->GetFloat("stage_enter_time");
        
        if (currentStage > 1) //MOM_TODO: || m_iStageCount < 2 (linear maps use checkpoints?)
        {
            //The first stage doesn't have its time yet, we calculate it upon going into stage 2+
            m_flStageTime[currentStage - 1] = m_flStageEnterTime[currentStage] - m_flStageEnterTime[currentStage - 1];
        } 

        m_flStageStrafeSyncAvg[currentStage - 1] = pEvent->GetFloat("avg_sync");
        m_flStageStrafeSync2Avg[currentStage - 1] = pEvent->GetFloat("avg_sync2");

        m_flStageExitSpeed[currentStage - 1][0] = pEvent->GetFloat("stage_exit_vel");
        m_flStageVelocityAvg[currentStage - 1][0] = pEvent->GetFloat("avg_vel");
        m_flStageVelocityMax[currentStage - 1][0] = pEvent->GetFloat("max_vel");

        m_flStageExitSpeed[currentStage - 1][1] = pEvent->GetFloat("stage_exit_vel_2D");
        m_flStageVelocityAvg[currentStage - 1][1] = pEvent->GetFloat("avg_vel_2D");
        m_flStageVelocityMax[currentStage - 1][1] = pEvent->GetFloat("max_vel_2D");

        m_iStageJumps[currentStage - 1] = pEvent->GetInt("num_jumps");
        m_iStageStrafes[currentStage - 1] = pEvent->GetInt("num_strafes");

    }
    else if (!Q_strcmp("new_stage_exit", pEvent->GetName()))
    {
        int currentStage = pEvent->GetInt("stage_num");
        m_flStageEnterSpeed[currentStage][0] = pEvent->GetFloat("stage_enter_vel");
        m_flStageEnterSpeed[currentStage][1] = pEvent->GetFloat("stage_enter_vel_2D");
    }
    else if (!Q_strcmp("run_save", pEvent->GetName()))
    {
        m_bTimeDidSave = pEvent->GetBool("run_saved");
    }
    else if (!Q_strcmp("run_upload", pEvent->GetName()))
    {
        m_bTimeDidUpload = pEvent->GetBool("run_posted");
        Q_strncpy(m_szRunUploadStatus, pEvent->GetString("web_msg"), sizeof(m_szRunUploadStatus));
        //MOM_TODO: potentially have stuff like new rank or something?
    }
    else if (!Q_strcmp("timer_state", pEvent->GetName()))
    {
        m_bTimerIsRunning = pEvent->GetBool("is_running");
    }
    else if (!Q_strcmp("practice_mode", pEvent->GetName()))
    {
        m_bPlayerHasPracticeMode = pEvent->GetBool("has_practicemode");
    }
    else if (!Q_strcmp("keypress", pEvent->GetName()))
    {
        m_iStageJumps[0] = pEvent->GetInt("num_jumps");
        m_iStageStrafes[0] = pEvent->GetInt("num_strafes");
    }
}

//Interface this event listener to the DLL
static C_Momentum_EventListener s_momListener;
C_Momentum_EventListener *g_MOMEventListener = &s_momListener;