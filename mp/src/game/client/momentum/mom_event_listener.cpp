#include "cbase.h"

#include "mom_event_listener.h"
#include "tier0/memdbgon.h"

void C_Momentum_EventListener::Init()
{
    //add listeners for all of our custom events
    ListenForGameEvent("timer_stopped");
    ListenForGameEvent("zone_enter");
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("run_save");
    ListenForGameEvent("run_upload");
    ListenForGameEvent("timer_state");
    ListenForGameEvent("keypress");
    ListenForGameEvent("map_init");
    m_EntRunStats.SetLessFunc(DefLessFunc(int));
}

void C_Momentum_EventListener::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp("timer_stopped", pEvent->GetName()))
    {
        int entIndex = pEvent->GetInt("ent");
        RunStats_t *stats = GetRunStats(entIndex);

        stats->m_flZoneStrafeSyncAvg[0] = pEvent->GetFloat("avg_sync");
        stats->m_flZoneStrafeSync2Avg[0] = pEvent->GetFloat("avg_sync2");
        //3D
        stats->m_flZoneEnterSpeed[0][0] = pEvent->GetFloat("start_vel");
        stats->m_flZoneExitSpeed[0][0] = pEvent->GetFloat("end_vel");
        stats->m_flZoneVelocityAvg[0][0] = pEvent->GetFloat("avg_vel");
        stats->m_flZoneVelocityMax[0][0] = pEvent->GetFloat("max_vel");
        //2D
        stats->m_flZoneEnterSpeed[0][1] = pEvent->GetFloat("start_vel_2D");
        stats->m_flZoneExitSpeed[0][1] = pEvent->GetFloat("end_vel_2D");
        stats->m_flZoneVelocityAvg[0][1] = pEvent->GetFloat("avg_vel_2D");
        stats->m_flZoneVelocityMax[0][1] = pEvent->GetFloat("max_vel_2D");

        m_flLastRunTime = pEvent->GetFloat("time");
    }
    else if (!Q_strcmp("zone_enter", pEvent->GetName()))
    {
        //NOTE: THE ONLY STAT BELOW THAT REQUIRES THE CURRENT STAGE GIVEN IN "num" IS THE ENTER TIME!
        //EVERYTHING ELSE IS m_iCurrentZone - 1 !

        int currentZone = pEvent->GetInt("num");
        int entIndex = pEvent->GetInt("ent");

        RunStats_t *stats = GetRunStats(entIndex);

        if (currentZone <= m_iMapZoneCount)
        {
            //This if cheeck is needed for making sure the ending zone doesn't go out of bounds
            //Since the only stats we care about that the ending trigger sends are not needed in this check

            //Note: enter_time will NOT change upon multiple entries to the same stage trigger (only set once per run)
            stats->m_flZoneEnterTime[currentZone] = pEvent->GetFloat("enter_time");
            //Reset the stage enter speed for the speedometer
            stats->m_flZoneEnterSpeed[currentZone][0] = 0.0f;
            stats->m_flZoneEnterSpeed[currentZone][1] = 0.0f;
        }
        
        if (currentZone > 1) //MOM_TODO: || m_iStageCount < 2 (linear maps use checkpoints?)
        {
            //The first stage doesn't have its time yet, we calculate it upon going into stage 2+
            stats->m_flZoneTime[currentZone - 1] = stats->m_flZoneEnterTime[currentZone] - stats->m_flZoneEnterTime[currentZone - 1];
            //And the rest of the stats are about the previous stage anyways, not calculated during stage 1 (start)
            stats->m_flZoneStrafeSyncAvg[currentZone - 1] = pEvent->GetFloat("avg_sync");
            stats->m_flZoneStrafeSync2Avg[currentZone - 1] = pEvent->GetFloat("avg_sync2");

            stats->m_flZoneExitSpeed[currentZone - 1][0] = pEvent->GetFloat("exit_vel");
            stats->m_flZoneVelocityAvg[currentZone - 1][0] = pEvent->GetFloat("avg_vel");
            stats->m_flZoneVelocityMax[currentZone - 1][0] = pEvent->GetFloat("max_vel");

            stats->m_flZoneExitSpeed[currentZone - 1][1] = pEvent->GetFloat("exit_vel_2D");
            stats->m_flZoneVelocityAvg[currentZone - 1][1] = pEvent->GetFloat("avg_vel_2D");
            stats->m_flZoneVelocityMax[currentZone - 1][1] = pEvent->GetFloat("max_vel_2D");

            stats->m_iZoneJumps[currentZone - 1] = pEvent->GetInt("num_jumps");
            stats->m_iZoneStrafes[currentZone - 1] = pEvent->GetInt("num_strafes");
        }
    }
    else if (!Q_strcmp("zone_exit", pEvent->GetName()))
    {
        int currentZone = pEvent->GetInt("num");
        int entIndex = pEvent->GetInt("ent");
        //This happens when the player/ghost exits the ending trigger (despawn or restart)
        if (currentZone > m_iMapZoneCount)
        {
            m_EntRunStats.Remove(entIndex);
        } 
        else
        {
            //Set the stage enter speed upon exiting the trigger
            float enterVel = pEvent->GetFloat("enter_vel");
            float enterVel2D = pEvent->GetFloat("enter_vel_2D");
            RunStats_t *stats = GetRunStats(entIndex);

            for (int i = 0; i < 2; i++)
            {
                float vel = i == 0 ? enterVel : enterVel2D;
                stats->m_flZoneEnterSpeed[currentZone][i] = vel;
                if (currentZone == 1)
                    stats->m_flZoneEnterSpeed[currentZone - 1][i] = vel;//Set overall enter vel
            }
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
        //m_bTimerIsRunning = pEvent->GetBool("is_running");
    }
    else if (!Q_strcmp("keypress", pEvent->GetName()))
    {
        int entIndex = pEvent->GetInt("ent");
        RunStats_t *stats = GetRunStats(entIndex);
        stats->m_iZoneJumps[0] = pEvent->GetInt("num_jumps");
        stats->m_iZoneStrafes[0] = pEvent->GetInt("num_strafes");
    }
    else if (!Q_strcmp("map_init", pEvent->GetName()))
    {
        m_EntRunStats.PurgeAndDeleteElements();
        m_bMapIsLinear = pEvent->GetBool("is_linear");
        m_iMapZoneCount = pEvent->GetInt("num_zones");
    }
}

//Interface this event listener to the DLL
static C_Momentum_EventListener s_momListener;
C_Momentum_EventListener *g_MOMEventListener = &s_momListener;