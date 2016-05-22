#include "cbase.h"

#include "mom_event_listener.h"
#include "tier0/memdbgon.h"

void C_Momentum_EventListener::Init()
{
    //add listeners for all of our custom events
    ListenForGameEvent("timer_stopped");
    ListenForGameEvent("stage_enter");
    ListenForGameEvent("stage_exit");
    ListenForGameEvent("run_save");
    ListenForGameEvent("run_upload");
    ListenForGameEvent("timer_state");
    ListenForGameEvent("keypress");
    ListenForGameEvent("map_init");
}

void C_Momentum_EventListener::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp("timer_stopped", pEvent->GetName()))
    {
        stats.m_flStageStrafeSyncAvg[0] = pEvent->GetFloat("avg_sync");
        stats.m_flStageStrafeSync2Avg[0] = pEvent->GetFloat("avg_sync2");
        //3D
        stats.m_flStageEnterSpeed[0][0] = pEvent->GetFloat("start_vel");
        stats.m_flStageExitSpeed[0][0] = pEvent->GetFloat("end_vel");
        stats.m_flStageVelocityAvg[0][0] = pEvent->GetFloat("avg_vel");
        stats.m_flStageVelocityMax[0][0] = pEvent->GetFloat("max_vel");
        //2D
        stats.m_flStageEnterSpeed[0][1] = pEvent->GetFloat("start_vel_2D");
        stats.m_flStageExitSpeed[0][1] = pEvent->GetFloat("end_vel_2D");
        stats.m_flStageVelocityAvg[0][1] = pEvent->GetFloat("avg_vel_2D");
        stats.m_flStageVelocityMax[0][1] = pEvent->GetFloat("max_vel_2D");
    }
    else if (!Q_strcmp("stage_enter", pEvent->GetName()))
    {
        //NOTE: THE ONLY STAT BELOW THAT REQUIRES THE CURRENT STAGE GIVEN IN "stage_num" IS THE ENTER TIME!
        //EVERYTHING ELSE IS m_iCurrentStage - 1 !

        int currentStage = pEvent->GetInt("stage_num");
        //Note: stage_enter_time will NOT change upon multiple entries to the same stage trigger (only set once per run)
        stats.m_flStageEnterTime[currentStage] = pEvent->GetFloat("stage_enter_time");
        //Reset the stage enter speed for the speedometer
        stats.m_flStageEnterSpeed[currentStage][0] = 0.0f;
        stats.m_flStageEnterSpeed[currentStage][1] = 0.0f;

        if (currentStage > 1) //MOM_TODO: || m_iStageCount < 2 (linear maps use checkpoints?)
        {
            //The first stage doesn't have its time yet, we calculate it upon going into stage 2+
            stats.m_flStageTime[currentStage - 1] = stats.m_flStageEnterTime[currentStage] - stats.m_flStageEnterTime[currentStage - 1];
            //And the rest of the stats are about the previous stage anyways, not calculated during stage 1 (start)
            stats.m_flStageStrafeSyncAvg[currentStage - 1] = pEvent->GetFloat("avg_sync");
            stats.m_flStageStrafeSync2Avg[currentStage - 1] = pEvent->GetFloat("avg_sync2");

            stats.m_flStageExitSpeed[currentStage - 1][0] = pEvent->GetFloat("stage_exit_vel");
            stats.m_flStageVelocityAvg[currentStage - 1][0] = pEvent->GetFloat("avg_vel");
            stats.m_flStageVelocityMax[currentStage - 1][0] = pEvent->GetFloat("max_vel");

            stats.m_flStageExitSpeed[currentStage - 1][1] = pEvent->GetFloat("stage_exit_vel_2D");
            stats.m_flStageVelocityAvg[currentStage - 1][1] = pEvent->GetFloat("avg_vel_2D");
            stats.m_flStageVelocityMax[currentStage - 1][1] = pEvent->GetFloat("max_vel_2D");

            stats.m_iStageJumps[currentStage - 1] = pEvent->GetInt("num_jumps");
            stats.m_iStageStrafes[currentStage - 1] = pEvent->GetInt("num_strafes");
        } 
    }
    else if (!Q_strcmp("stage_exit", pEvent->GetName()))
    {
        int currentStage = pEvent->GetInt("stage_num");
        //Set the stage enter speed upon exiting the trigger
        float enterVel = pEvent->GetFloat("stage_enter_vel");
        float enterVel2D = pEvent->GetFloat("stage_enter_vel_2D");
        for (int i = 0; i < 2; i++)
        {
            float vel = i == 0 ? enterVel : enterVel2D;
            stats.m_flStageEnterSpeed[currentStage][i] = vel;
            if (currentStage == 1)
                stats.m_flStageEnterSpeed[currentStage - 1][i] = vel;//Set overall enter vel
        }
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
    else if (!Q_strcmp("keypress", pEvent->GetName()))
    {
        stats.m_iStageJumps[0] = pEvent->GetInt("num_jumps");
        stats.m_iStageStrafes[0] = pEvent->GetInt("num_strafes");
    }
    else if (!Q_strcmp("map_init", pEvent->GetName()))
    {
        m_bMapIsLinear = pEvent->GetBool("is_linear");
        m_iMapCheckpointCount = pEvent->GetInt("num_checkpoints");
    }
}

//Interface this event listener to the DLL
static C_Momentum_EventListener s_momListener;
C_Momentum_EventListener *g_MOMEventListener = &s_momListener;