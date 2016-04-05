#include "cbase.h"

#include "mom_event_listener.h"
#include "tier0/memdbgon.h"

C_Momentum_EventListener::C_Momentum_EventListener()
{
    //add listeners for all of our custom events
    gameeventmanager->AddListener(this, "timer_stopped", false);
    gameeventmanager->AddListener(this, "timer_started", false);
    gameeventmanager->AddListener(this, "player_inside_mapzone", false);
    gameeventmanager->AddListener(this, "run_save", false);
}
void C_Momentum_EventListener::FireGameEvent(IGameEvent *pEvent)
{
    if (!strcmp("run_save", pEvent->GetName()))
    {
        m_bTimeDidSave = pEvent->GetBool("run_saved");
        m_bTimeDidUpload = pEvent->GetBool("run_posted");
    }
    if (!strcmp("timer_stopped", pEvent->GetName()))
    {
        m_flStartSpeed = pEvent->GetFloat("start_vel");
        m_flEndSpeed = pEvent->GetFloat("end_vel");
        m_flStrafeSyncAvg = pEvent->GetFloat("avg_sync");
        m_flStrafeSync2Avg = pEvent->GetFloat("avg_sync2");
        m_flVelocityAvg = pEvent->GetFloat("avg_vel");
        m_flVelocityMax = pEvent->GetFloat("max_vel");
    }
    if (!strcmp("timer_started", pEvent->GetName()))
    {
        m_bTimerIsRunning = pEvent->GetBool("timer_isrunning");
    }
    if (!strcmp("player_inside_mapzone", pEvent->GetName()))
    {
        m_bPlayerInsideStartZone = pEvent->GetBool("inside_startzone");
        m_bPlayerInsideEndZone = pEvent->GetBool("inside_endzone");
        m_bMapFinished = pEvent->GetBool("map_finished"); //different from "inside endzone", this is only fired if the player finished when their timer was running
    }
}