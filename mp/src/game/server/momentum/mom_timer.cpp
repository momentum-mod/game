#include "cbase.h"

#include "mom_timer.h"

#include "mom_player_shared.h"
#include "mom_replay_system.h"
#include "mom_system_saveloc.h"
#include "mom_system_gamemode.h"
#include "mom_triggers.h"
#include "movevars_shared.h"
#include "run/mom_run_safeguards.h"
#include "trigger_trace_enums.h"

#include "tier0/memdbgon.h"


CMomentumTimer::CMomentumTimer() : CAutoGameSystemPerFrame("CMomentumTimer"),
      m_iStartTick(0), m_iEndTick(0), m_bIsRunning(false),
      m_bCanStart(false), m_bWasCheatsMsgShown(false), m_iTrackNumber(0), m_bShouldUseStartZoneOffset(false)
{
}

void CMomentumTimer::LevelInitPostEntity() { m_bWasCheatsMsgShown = false; }

void CMomentumTimer::LevelShutdownPreEntity()
{
    if (m_bIsRunning)
        Stop(nullptr);

    m_bWasCheatsMsgShown = false;

    for (int i = 0; i < MAX_TRACKS; i++)
    {
        m_hStartTriggers[i] = nullptr;
    }
}

void CMomentumTimer::FrameUpdatePreEntityThink()
{
    if (!m_bWasCheatsMsgShown)
    {
        static ConVarRef cheats("sv_cheats");
        if (cheats.GetBool())
        {
            DispatchCheatsMessage(CMomentumPlayer::GetLocalPlayer());
        }
    }
}

void CMomentumTimer::DispatchCheatsMessage(CMomentumPlayer *pPlayer)
{
    UTIL_ShowMessage("CHEATER", pPlayer);
    // MOM_TODO play a special sound here?
    m_bWasCheatsMsgShown = true;
}

void CMomentumTimer::DispatchTickrateMessage(CMomentumPlayer *pPlayer)
{
    UTIL_ShowMessage("NON_DEFAULT_TICKRATE", pPlayer);
}

bool CMomentumTimer::Start(CMomentumPlayer *pPlayer)
{
    if (!pPlayer)
        return false;

    if (!m_bCanStart)
    {
        Warning("Cannot start timer, make sure to properly reset your timer by landing in the start zone!\n");
        return false;
    }

    // Perform all the checks to ensure player can start
    if (g_pSavelocSystem->IsUsingSaveLocMenu())
    {
        // MOM_TODO: Allow it based on gametype
        Warning("Cannot start timer while using save loc menu!\n");
        return false;
    }
    static ConVarRef mom_zone_edit("mom_zone_edit", true);
    if (mom_zone_edit.IsValid() && mom_zone_edit.GetBool())
    {
        Warning("Cannot start timer while editing zones!\n");
        return false;
    }
    if (pPlayer->m_bHasPracticeMode)
    {
        Warning("Cannot start timer while in practice mode!\n");
        return false;
    }
    if (pPlayer->GetMoveType() == MOVETYPE_NOCLIP)
    {
        Warning("Cannot start timer while in noclip!\n");
        return false;
    }
    static ConVarRef cheats("sv_cheats");
    if (cheats.GetBool())
    {
        // We allow cheats to be enabled but we should warn the player that times won't submit
        DispatchCheatsMessage(pPlayer);
    }
    if (!CloseEnough(gpGlobals->interval_per_tick, g_pGameModeSystem->GetGameMode()->GetIntervalPerTick(), FLT_EPSILON))
    {
        // We also allow different tickrates, but warn the player that times won't submit on anything other than the
        // default tickrate for the current game mode
        DispatchTickrateMessage(pPlayer);
    }

    m_iStartTick = gpGlobals->tickcount;
    m_iEndTick = 0;
    m_iTrackNumber = pPlayer->m_Data.m_iCurrentTrack;
    SetRunning(pPlayer, true);

    // Dispatch a start timer message for the local player
    DispatchTimerEventMessage(pPlayer, pPlayer->entindex(), TIMER_EVENT_STARTED);

    return true;
}

void CMomentumTimer::Stop(CMomentumPlayer *pPlayer, bool bFinished /* = false */, bool bStopRecording /* = true*/)
{
    bool bWasRunning = m_bIsRunning;

    SetRunning(pPlayer, false);

    if (!bWasRunning)
        return;

    if (pPlayer)
    {
        // Set our end time and date
        if (bFinished)
        {
            m_iEndTick = gpGlobals->tickcount;
            g_ReplaySystem.SetTimerStopTick(m_iEndTick);
        }

        DispatchTimerEventMessage(pPlayer, pPlayer->entindex(), bFinished ? TIMER_EVENT_FINISHED : TIMER_EVENT_STOPPED);
    }

    if (g_ReplaySystem.IsRecording() && bStopRecording)
    {
        if (bFinished)
        {
            g_ReplaySystem.StopRecording();
        }
        else
        {
            g_ReplaySystem.CancelRecording();
        }
    }
}

void CMomentumTimer::Reset(CMomentumPlayer *pPlayer)
{
    // It'll get set to true if they teleport to a CP out of here
    g_pSavelocSystem->SetUsingSavelocMenu(false);
    pPlayer->ResetRunStats();
    pPlayer->m_Data.m_bMapFinished = false;
    pPlayer->m_Data.m_iTimerState = TIMER_STATE_NOT_RUNNING;

    if (m_bIsRunning)
    {
        Stop(pPlayer, false, false); // Don't stop our replay just yet
        DispatchResetMessage(pPlayer);
    }
    else
    {
        // Reset last jump velocity when we enter the start zone without a timer
        pPlayer->m_Data.m_flLastJumpVel = 0;

        // Handle the replay recordings
        if (g_ReplaySystem.IsRecording())
            g_ReplaySystem.CancelRecording();

        g_ReplaySystem.BeginRecording();
    }

    // Reset our CanStart bool
    m_bCanStart = true;
}

void CMomentumTimer::TryStart(CMomentumPlayer *pPlayer, bool bUseStartZoneOffset)
{
    if (!m_bIsRunning)
    {
        SetShouldUseStartZoneOffset(bUseStartZoneOffset);

        // The Start method could fail if CP menu or prac mode is activated here
        if (Start(pPlayer))
        {
            // Used for trimming later on
            if (g_ReplaySystem.IsRecording())
            {
                g_ReplaySystem.SetTimerStartTick(gpGlobals->tickcount);
            }

            // Used for spectating later on
            pPlayer->m_Data.m_iStartTick = gpGlobals->tickcount;

            // Are we in mid air when we started? If so, our first jump should be 1, not 0
            if (pPlayer->IsInAirDueToJump())
            {
                pPlayer->m_RunStats.SetZoneJumps(0, 1);
                pPlayer->m_RunStats.SetZoneJumps(pPlayer->m_Data.m_iCurrentZone, 1);
            }
        }
        else
        {
            DispatchTimerEventMessage(pPlayer, pPlayer->entindex(), TIMER_EVENT_FAILED);
        }

        // Force canStart to false regardless of starting or not
        m_bCanStart = false;
    }
    else
    {
        SetShouldUseStartZoneOffset(!bUseStartZoneOffset);
    }

    pPlayer->m_Data.m_bMapFinished = false;
}

void CMomentumTimer::DispatchResetMessage(CMomentumPlayer *pPlayer) const
{
    CSingleUserRecipientFilter user(pPlayer);
    user.MakeReliable();
    UserMessageBegin(user, "Timer_Reset");
    MessageEnd();
}

void CMomentumTimer::DispatchTimerEventMessage(CBasePlayer *pPlayer, int iEntIndx, int type) const
{
    IGameEvent *pEvent = gameeventmanager->CreateEvent("timer_event");
    if (pEvent)
    {
        pEvent->SetInt("ent", iEntIndx);
        pEvent->SetInt("type", type);
        gameeventmanager->FireEvent(pEvent);
    }

    CSingleUserRecipientFilter user(pPlayer);
    user.MakeReliable();

    UserMessageBegin(user, "Timer_Event");
    WRITE_BYTE(type);
    MessageEnd();
}

void CMomentumTimer::SetStartTrigger(int track, CTriggerTimerStart *pTrigger)
{
    if (track >= 0 && track < MAX_TRACKS)
    {
        // Make sure trigger and associated track aren't mismatched
        if (pTrigger && pTrigger->GetTrackNumber() == track)
            m_hStartTriggers[track] = pTrigger;
        else
            Warning("Cannot set the start trigger for the given track; the trigger is null or its track doesn't match!\n");
    }
    else
    {
        Warning("Cannot set the start trigger; the track is outside the valid track numbers!\n");
    }
}

int CMomentumTimer::GetLastRunTime() const { return m_iEndTick - m_iStartTick; }

void CMomentumTimer::SetRunning(CMomentumPlayer *pPlayer, bool isRunning)
{
    m_bIsRunning = isRunning;

    if (pPlayer)
        pPlayer->m_Data.m_iTimerState = isRunning ? TIMER_STATE_RUNNING : TIMER_STATE_NOT_RUNNING;
}
void CMomentumTimer::CalculateTickIntervalOffset(CMomentumPlayer *pPlayer, const int zoneType, const int zoneNumber)
{
    if (!pPlayer)
        return;

    Ray_t ray;
    Vector start, end, offset;

    // Since EndTouch is called after PostThink (which is where previous origins are stored) we need to go 1 more tick
    // in the previous data to get the real previous origin.
    if (zoneType == ZONE_TYPE_START) // EndTouch
    {
        start = pPlayer->GetLocalOrigin();
        end = pPlayer->GetPreviousOrigin(1);
    }
    else // StartTouch
    {
        start = pPlayer->GetPreviousOrigin();
        end = pPlayer->GetLocalOrigin();
    }

    ray.Init(start, end, pPlayer->CollisionProp()->OBBMins(), pPlayer->CollisionProp()->OBBMaxs());
    CTimeTriggerTraceEnum endTriggerTraceEnum(&ray, pPlayer->GetAbsVelocity());
    enginetrace->EnumerateEntities(ray, true, &endTriggerTraceEnum);

    DevLog("Time offset was %f seconds (%s)\n", endTriggerTraceEnum.GetOffset(),
           zoneType == ZONE_TYPE_START ? "EndTouch" : "StartTouch");
    SetIntervalOffset(zoneNumber, endTriggerTraceEnum.GetOffset());
}

// Practice mode that stops the timer and allows the player to noclip.
void CMomentumTimer::EnablePractice(CMomentumPlayer *pPlayer)
{
    // MOM_TODO: if (m_bIsRunning && g_ReplaySystem.IsRecording()) g_ReplaySystem.MarkEnterPracticeMode()
}

void CMomentumTimer::DisablePractice(CMomentumPlayer *pPlayer)
{
    // MOM_TODO: if (m_bIsRunning && g_ReplaySystem.IsRecording()) g_ReplaySystem.MarkExitPracticeMode()
}

//--------- Commands --------------------------------

CON_COMMAND(mom_start_mark_create, "Marks a starting point inside the start trigger for a more customized starting location.\n")
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        pPlayer->CreateStartMark();
    }
}

CON_COMMAND(mom_start_mark_clear, "Clears the saved start location for your current track, if there is one.\n"
                                  "You may also specify the track number to clear as the parameter; "
                                  "\"mom_start_mark_clear 2\" clears track 2's start mark.")
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        int track = pPlayer->m_Data.m_iCurrentTrack;
        if (args.ArgC() > 1)
        {
            track = Q_atoi(args[1]);
        }

        pPlayer->ClearStartMark(track);
    }
}

CON_COMMAND_F(mom_timer_stop, "Stops the timer if it is currently running.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        g_pMomentumTimer->Stop(pPlayer);
    }
}

CON_COMMAND_F(mom_restart,
              "Restarts the player to the start trigger. Optionally takes a track number to restart to (default is "
              "main track).\n",
              FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_RESTART))
        return;

    int track = args.ArgC() > 1 ? Q_atoi(args[1]) : pPlayer->m_Data.m_iCurrentTrack;
    pPlayer->TimerCommand_Restart(track);
}

CON_COMMAND_F(mom_restart_stage, 
              "Teleports the player back to the start of the current stage.\n"
              "Optionally takes a stage or stage/track arguments which teleport the player to the desired "
              "stage on the default (current) or desired track, but stops the timer.\n"
              "Usage: mom_restart_stage [stage] [track]\n",
              FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE)
{
    if (g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF))
    {
        Warning("This command deliberately does nothing in this gamemode!\n");
        return;
    }

    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_RESTART_STAGE))
        return;

    int stage = 0, track = 0;
    if (args.ArgC() > 1)
    {
        stage = Q_atoi(args[1]);
        track = args.ArgC() > 2 ? Q_atoi(args[2]) : pPlayer->m_Data.m_iCurrentTrack;
    }
    else
    {
        stage = pPlayer->m_Data.m_iCurrentZone;
        track = pPlayer->m_Data.m_iCurrentTrack;
    }

    if (pPlayer->m_iLinearTracks.Get(track) == true)
    {
        Warning("Track only has one zone! Use mom_restart instead.\n");
        return;
    }


    pPlayer->TimerCommand_RestartStage(stage, track);
}

static CMomentumTimer s_Timer;
CMomentumTimer *g_pMomentumTimer = &s_Timer;
