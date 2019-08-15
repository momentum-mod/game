#include "cbase.h"

#include "mom_timer.h"

#include <ctime>
#include "in_buttons.h"
#include "mom_player_shared.h"
#include "mom_system_saveloc.h"
#include "mom_triggers.h"
#include "movevars_shared.h"

#include "tier0/memdbgon.h"

class CTimeTriggerTraceEnum : public IEntityEnumerator
{
  public:
    CTimeTriggerTraceEnum(Ray_t *pRay, Vector velocity) : m_vecVelocity(velocity), m_pRay(pRay) { m_flOffset = 0.0f; }

    bool EnumEntity(IHandleEntity *pHandleEntity) OVERRIDE;
    float GetOffset() { return m_flOffset; }

  private:
    float m_flOffset;
    Vector m_vecVelocity;
    Ray_t *m_pRay;
};

CMomentumTimer::CMomentumTimer()
    : CAutoGameSystemPerFrame("CMomentumTimer"), m_iStartTick(0), m_iEndTick(0), m_iLastRunDate(0), m_bIsRunning(false),
      m_bCanStart(false), m_bWasCheatsMsgShown(false), m_iTrackNumber(0), m_bShouldUseStartZoneOffset(false)
{
}

void CMomentumTimer::LevelInitPostEntity() { m_bWasCheatsMsgShown = false; }

void CMomentumTimer::LevelShutdownPreEntity()
{
    if (IsRunning())
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

bool CMomentumTimer::TryStart(CMomentumPlayer *pPlayer)
{
    if (!pPlayer)
        return false;

    if (!m_bCanStart)
    {
        Warning("Cannot start timer, make sure to properly reset your timer by landing in the start zone!\n");
        return false;
    }

    // Perform all the checks to ensure player can start
    if (g_pMOMSavelocSystem->IsUsingSaveLocMenu())
    {
        // MOM_TODO: Allow it based on gametype
        Warning("Cannot start timer while using save loc menu!\n");
        return false;
    }
    static ConVarRef mom_zone_edit("mom_zone_edit");
    if (mom_zone_edit.GetBool())
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

    m_iStartTick = gpGlobals->tickcount;
    m_iEndTick = 0;
    m_iLastRunDate = 0;
    m_iTrackNumber = pPlayer->m_Data.m_iCurrentTrack;
    SetRunning(pPlayer, true);

    // Dispatch a start timer message for the local player
    DispatchTimerEvent(pPlayer, pPlayer->entindex(), TIMER_EVENT_START);

    return true;
}

bool CMomentumTimer::TryEnd(CMomentumPlayer *pPlayer)
{
    SetRunning(pPlayer, false);
    if (pPlayer)
    {
        m_iEndTick = gpGlobals->tickcount;
        time(&m_iLastRunDate); // Set the last run date for the replay
    }

    return true;
}

void CMomentumTimer::Stop(CMomentumPlayer *pPlayer)
{
    TryEnd(pPlayer);

    if (pPlayer)
    {
        DispatchTimerEvent(pPlayer, pPlayer->entindex(), TIMER_EVENT_STOP);
    }
}

void CMomentumTimer::Finish(CMomentumPlayer *pPlayer)
{
    TryEnd(pPlayer);

    if (pPlayer)
    {
        DispatchTimerEvent(pPlayer, pPlayer->entindex(), TIMER_EVENT_FINISH);
    }
}

void CMomentumTimer::Reset(CMomentumPlayer *pPlayer)
{
    // It'll get set to true if they teleport to a CP out of here
    g_pMOMSavelocSystem->SetUsingSavelocMenu(false);
    pPlayer->ResetRunStats();
    pPlayer->m_Data.m_bMapFinished = false;
    pPlayer->m_Data.m_bTimerRunning = false;

    if (m_bIsRunning)
    {
        Stop(pPlayer);
        DispatchTimerEvent(pPlayer, pPlayer->entindex(), TIMER_EVENT_RESET);
    }
    else
    {
        // Reset last jump velocity when we enter the start zone without a timer
        pPlayer->m_Data.m_flLastJumpVel = 0;
    }

    // Reset our CanStart bool
    m_bCanStart = true;
}

void CMomentumTimer::Start(CMomentumPlayer *pPlayer, bool bUseStartZoneOffset)
{
    if (!m_bIsRunning)
    {
        SetShouldUseStartZoneOffset(bUseStartZoneOffset);

        // The Start method could fail if CP menu or prac mode is activated here
        if (TryStart(pPlayer))
        {
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
            DispatchTimerEvent(pPlayer, pPlayer->entindex(), TIMER_EVENT_FAILED_TO_START);
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

void CMomentumTimer::DispatchTimerEvent(CBasePlayer *pPlayer, int iEntIndx, int type) const
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
            Warning(
                "Cannot set the start trigger for the given track; the trigger is null or its track doesn't match!\n");
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
        pPlayer->m_Data.m_bTimerRunning = isRunning;
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

// override of IEntityEnumerator's EnumEntity() in order for our trace to hit zone triggers
bool CTimeTriggerTraceEnum::EnumEntity(IHandleEntity *pHandleEntity)
{
    trace_t tr;
    // store entity that we found on the trace
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

    // Stop the trace if this entity is solid.
    if (pEnt->IsSolid())
        return false;

    // if we aren't hitting a momentum trigger
    // the return type of EnumEntity tells the engine whether to continue enumerating future entities
    // or not.
    if (Q_strnicmp(pEnt->GetClassname(), "trigger_momentum_", Q_strlen("trigger_momentum_")) == 1)
        return false;

    // In this case, we want to continue in case we hit another type of trigger.

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);
    if (tr.fraction > 0.0f)
    {
        m_flOffset = tr.startpos.DistTo(tr.endpos) / m_vecVelocity.Length();

        // Account for slowmotion/timescale
        m_flOffset /= gpGlobals->interval_per_tick / gpGlobals->frametime;
        return true; // We hit our target
    }

    return false;
}

// Practice mode that stops the timer and allows the player to noclip.
void CMomentumTimer::EnablePractice(CMomentumPlayer *pPlayer)
{
    DispatchTimerEvent(pPlayer, pPlayer->entindex(), TIMER_EVENT_ENTER_PRACTICE);
}

void CMomentumTimer::DisablePractice(CMomentumPlayer *pPlayer)
{
    DispatchTimerEvent(pPlayer, pPlayer->entindex(), TIMER_EVENT_EXIT_PRACTICE);
}

//--------- Commands --------------------------------

CON_COMMAND(mom_start_mark_create,
            "Marks a starting point inside the start trigger for a more customized starting location.\n")
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

CON_COMMAND_F(mom_timer_stop, "Stops the timer if it is currently running.", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer && g_pMomentumTimer->IsRunning())
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
    if (!pPlayer || !pPlayer->AllowUserTeleports())
        return;

    int track = pPlayer->m_Data.m_iCurrentTrack;
    if (args.ArgC() > 1)
    {
        track = Q_atoi(args[1]);
    }

    const auto pStart = g_pMomentumTimer->GetStartTrigger(track);
    if (pStart)
    {
        const auto pStartMark = pPlayer->GetStartMark(track);
        if (pStartMark)
        {
            pStartMark->Teleport(pPlayer);
        }
        else
        {
            // Don't set angles if still in start zone.
            QAngle ang = pStart->GetLookAngles();
            pPlayer->Teleport(&pStart->WorldSpaceCenter(), (pStart->HasLookAngles() ? &ang : nullptr), &vec3_origin);
        }

        pPlayer->m_Data.m_iCurrentTrack = track;
        pPlayer->ResetRunStats();
    }
    else
    {
        const auto pStartPoint = pPlayer->EntSelectSpawnPoint();
        if (pStartPoint)
        {
            pPlayer->Teleport(&pStartPoint->GetAbsOrigin(), &pStartPoint->GetAbsAngles(), &vec3_origin);
            pPlayer->ResetRunStats();
        }
    }
}

CON_COMMAND_F(mom_reset, "Teleports the player back to the start of the current stage.\n",
              FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer && pPlayer->AllowUserTeleports())
    {
        const auto pCurrentZone = pPlayer->GetCurrentZoneTrigger();
        // MOM_TODO do a trace downwards from the top of the trigger's center to touchable land, teleport the player
        // there
        if (pCurrentZone)
            pPlayer->Teleport(&pCurrentZone->WorldSpaceCenter(), nullptr, &vec3_origin);
        else
            Warning("Cannot reset, you have no current zone!\n");
    }
}

CON_COMMAND_F(mom_stage_tele,
              "Teleports the player to the desired stage. Stops the timer (Useful for mappers)\n"
              "Usage: mom_stage_tele <stage> [track]\nThe default track is the current track the player is on.",
              FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = CMomentumPlayer::GetLocalPlayer();
    const Vector *pVec = nullptr;
    const QAngle *pAng = nullptr;
    if (pPlayer && pPlayer->AllowUserTeleports() && args.ArgC() >= 2)
    {
        int track = pPlayer->m_Data.m_iCurrentTrack;
        if (args.ArgC() > 2)
        {
            track = Q_atoi(args[2]);
        }

        const auto desiredIndex = Q_atoi(args[1]);
        if (desiredIndex == ZONE_NUMBER_START)
        {
            const auto pStartMark = pPlayer->GetStartMark(track);
            if (pStartMark)
            {
                pVec = &pStartMark->pos;
                pAng = &pStartMark->ang;
            }
            else
            {
                // If no mark was found, we teleport to the center of the start trigger
                CBaseEntity *pEnt = g_pMomentumTimer->GetStartTrigger(track);
                if (pEnt)
                {
                    pVec = &pEnt->GetAbsOrigin();

                    if (track != pPlayer->m_Data.m_iCurrentTrack)
                    {
                        pPlayer->ResetRunStats();
                        pPlayer->m_Data.m_iCurrentTrack = track;
                    }
                }
            }
        }
        else if (desiredIndex > ZONE_NUMBER_START)
        {
            CTriggerStage *pStage = nullptr;

            while ((pStage = static_cast<CTriggerStage *>(
                        gEntList.FindEntityByClassname(pStage, "trigger_momentum_timer_stage"))) != nullptr)
            {
                if (pStage && pStage->GetZoneNumber() == desiredIndex && pStage->GetTrackNumber() == track)
                {
                    pVec = &pStage->GetAbsOrigin();
                    pAng = &pStage->GetAbsAngles();
                    break;
                }
            }
        }

        // Teleport if we have a destination
        if (pVec)
        {
            // pAng can be null here, it's okay
            pPlayer->Teleport(pVec, pAng, &vec3_origin);
            // Stop *after* the teleport
            g_pMomentumTimer->Stop(pPlayer);
        }
        else
        {
            Warning("Could not teleport to stage %i! Perhaps it doesn't exist?\n", desiredIndex);
        }
    }
}

static CMomentumTimer s_Timer;
CMomentumTimer *g_pMomentumTimer = &s_Timer;
