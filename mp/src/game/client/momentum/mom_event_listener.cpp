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
        CMomRunStats *stats = GetRunStats(entIndex);

        stats->SetZoneStrafeSyncAvg(0, pEvent->GetFloat("avg_sync"));
        stats->SetZoneStrafeSync2Avg(0, pEvent->GetFloat("avg_sync2"));

        stats->SetZoneEnterSpeed(0, pEvent->GetFloat("start_vel"), pEvent->GetFloat("start_vel_2D"));
        stats->SetZoneExitSpeed(0, pEvent->GetFloat("end_vel"), pEvent->GetFloat("end_vel_2D"));
        stats->SetZoneVelocityAvg(0, pEvent->GetFloat("avg_vel"), pEvent->GetFloat("avg_vel_2D"));
        stats->SetZoneVelocityMax(0, pEvent->GetFloat("max_vel"), pEvent->GetFloat("max_vel_2D"));

        m_flLastRunTime = pEvent->GetFloat("time");
    }
    else if (!Q_strcmp("zone_enter", pEvent->GetName()))
    {
        //NOTE: THE ONLY STAT BELOW THAT REQUIRES THE CURRENT STAGE GIVEN IN "num" IS THE ENTER TIME!
        //EVERYTHING ELSE IS m_iCurrentZone - 1 !

        int currentZone = pEvent->GetInt("num");
        int entIndex = pEvent->GetInt("ent");

        CMomRunStats *stats = GetRunStats(entIndex);

        if (currentZone <= m_iMapZoneCount)
        {
            //This if cheeck is needed for making sure the ending zone doesn't go out of bounds
            //Since the only stats we care about that the ending trigger sends are not needed in this check

            //Note: enter_time will NOT change upon multiple entries to the same stage trigger (only set once per run)
            stats->SetZoneEnterTime(currentZone, pEvent->GetFloat("enter_time"));
            //Reset the stage enter speed for the speedometer
            //stats->m_flZoneEnterSpeed[currentZone][0] = 0.0f;
            //stats->m_flZoneEnterSpeed[currentZone][1] = 0.0f;
        }
        
        if (currentZone > 1) //MOM_TODO: || m_iStageCount < 2 (linear maps use checkpoints?)
        {
            //The first stage doesn't have its time yet, we calculate it upon going into stage 2+
            stats->SetZoneTime(currentZone - 1, stats->GetZoneEnterTime(currentZone) - stats->GetZoneEnterTime(currentZone - 1));

            //And the rest of the stats are about the previous stage anyways, not calculated during stage 1 (start)
            stats->SetZoneStrafeSyncAvg(currentZone - 1, pEvent->GetFloat("avg_sync"));
            stats->SetZoneStrafeSync2Avg(currentZone - 1, pEvent->GetFloat("avg_sync2"));

            stats->SetZoneExitSpeed(currentZone - 1, pEvent->GetFloat("exit_vel"), pEvent->GetFloat("exit_vel_2D"));
            stats->SetZoneVelocityAvg(currentZone - 1, pEvent->GetFloat("avg_vel"), pEvent->GetFloat("avg_vel_2D"));
            stats->SetZoneVelocityMax(currentZone - 1, pEvent->GetFloat("max_vel"), pEvent->GetFloat("max_vel_2D"));

            stats->SetZoneJumps(currentZone - 1, pEvent->GetInt("num_jumps"));
            stats->SetZoneStrafes(currentZone - 1, pEvent->GetInt("num_strafes"));
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
            CMomRunStats *stats = GetRunStats(entIndex);

            stats->SetZoneEnterSpeed(currentZone, enterVel, enterVel2D);

            if (currentZone == 1)
                stats->SetZoneEnterSpeed(currentZone - 1, enterVel, enterVel2D);
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
        CMomRunStats *stats = GetRunStats(entIndex);
        stats->SetZoneJumps(0, pEvent->GetInt("num_jumps"));
        stats->SetZoneStrafes(0, pEvent->GetInt("num_strafes"));
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