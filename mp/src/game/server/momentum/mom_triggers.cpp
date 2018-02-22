#include "cbase.h"

#include "in_buttons.h"
#include "mom_player.h"
#include "mom_replay_entity.h"
#include "mom_replay_system.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "movevars_shared.h"
#include "tier0/memdbgon.h"

// CBaseMomentumTrigger
void CBaseMomentumTrigger::Spawn()
{
    BaseClass::Spawn();
    // temporary
    m_debugOverlays |= (OVERLAY_BBOX_BIT | OVERLAY_TEXT_BIT);
}

//---------- CTriggerStage -----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stage, CTriggerStage);

BEGIN_DATADESC(CTriggerStage)
DEFINE_KEYFIELD(m_iStageNumber, FIELD_INTEGER, "stage")
END_DATADESC()

void CTriggerStage::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    int stageNum = GetStageNumber();

    IGameEvent *stageEvent = nullptr;
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        // Set the current stage to this
        g_pMomentumTimer->SetCurrentZone(this);
        // Set player run data
        pPlayer->m_SrvData.m_RunData.m_bIsInZone = true;
        pPlayer->m_SrvData.m_RunData.m_iCurrentZone = stageNum;
        stageEvent = gameeventmanager->CreateEvent("zone_enter");
        if (g_pMomentumTimer->IsRunning())
        {
            pPlayer->m_RunStats.SetZoneExitSpeed(stageNum - 1, pPlayer->GetLocalVelocity().Length(),
                                                 pPlayer->GetLocalVelocity().Length2D());
            g_pMomentumTimer->CalculateTickIntervalOffset(pPlayer, g_pMomentumTimer->ZONETYPE_END);
            pPlayer->m_RunStats.SetZoneEnterTime(stageNum, g_pMomentumTimer->CalculateStageTime(stageNum));
            pPlayer->m_RunStats.SetZoneTime(stageNum - 1, pPlayer->m_RunStats.GetZoneEnterTime(stageNum) -
                                                              pPlayer->m_RunStats.GetZoneEnterTime(stageNum - 1));
        }
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            stageEvent = gameeventmanager->CreateEvent("zone_enter");
            pGhost->m_SrvData.m_RunData.m_iCurrentZone = stageNum;
            pGhost->m_SrvData.m_RunData.m_bIsInZone = true;
        }
    }

    // Used by speedometer UI
    if (stageEvent)
    {
        gameeventmanager->FireEvent(stageEvent);
    }
}
void CTriggerStage::EndTouch(CBaseEntity *pOther)
{
    BaseClass::EndTouch(pOther);
    int stageNum = GetStageNumber();
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    IGameEvent *stageEvent = nullptr;
    if (pPlayer)
    {
        // Timer won't be running if it's the start trigger
        if ((stageNum == 1 || g_pMomentumTimer->IsRunning()) && !pPlayer->m_SrvData.m_bHasPracticeMode)
        {
            // This handles both the start and stage triggers
            g_pMomentumTimer->CalculateTickIntervalOffset(pPlayer, g_pMomentumTimer->ZONETYPE_START);

            float enterVel3D = pPlayer->GetLocalVelocity().Length(),
                  enterVel2D = pPlayer->GetLocalVelocity().Length2D();
            pPlayer->m_RunStats.SetZoneEnterSpeed(stageNum, enterVel3D, enterVel2D);
            if (stageNum == 1)
                pPlayer->m_RunStats.SetZoneEnterSpeed(0, enterVel3D, enterVel2D);

            stageEvent = gameeventmanager->CreateEvent("zone_exit");
        }

        // Status
        pPlayer->m_SrvData.m_RunData.m_bIsInZone = false;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            pGhost->m_SrvData.m_RunData.m_bIsInZone = false;

            stageEvent = gameeventmanager->CreateEvent("zone_exit");
        }
    }

    if (stageEvent)
    {
        gameeventmanager->FireEvent(stageEvent);
    }
}
//------------------------------------------------------------------------------------------

//---------- CTriggerTimerStart ------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, CTriggerTimerStart);

BEGIN_DATADESC(CTriggerTimerStart)
DEFINE_KEYFIELD(m_fBhopLeaveSpeed, FIELD_FLOAT, "bhopleavespeed")
, DEFINE_KEYFIELD(m_angLook, FIELD_VECTOR, "lookangles") END_DATADESC();

CTriggerTimerStart::CTriggerTimerStart() : m_angLook(vec3_angle), m_fBhopLeaveSpeed(250){};

void CTriggerTimerStart::EndTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer())
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);

        bool bCheating = pPlayer->GetMoveType() == MOVETYPE_NOCLIP;

        // surf or other gamemodes has timer start on exiting zone, bhop timer starts when the player jumps
        // do not start timer if player is in practice mode or it's already running.
        if (!g_pMomentumTimer->IsRunning() && !pPlayer->m_SrvData.m_bHasPracticeMode && !bCheating &&
            !pPlayer->IsUsingCPMenu())
        {
            if (IsLimitingSpeed() && pPlayer->DidPlayerBhop())
            {
                Vector velocity = pOther->GetAbsVelocity();
                // Isn't it nice how Vector2D.h doesn't have Normalize() on it?
                // It only has a NormalizeInPlace... Not simple enough for me
                Vector2D vel2D = velocity.AsVector2D();
                if (velocity.AsVector2D().IsLengthGreaterThan(m_fBhopLeaveSpeed))
                {
                    vel2D = ((vel2D / vel2D.Length()) * (m_fBhopLeaveSpeed));
                    pOther->SetAbsVelocity(Vector(vel2D.x, vel2D.y, velocity.z));
                }
            }
            g_pMomentumTimer->Start(gpGlobals->tickcount);
            // The Start method could return if CP menu or prac mode is activated here
            if (g_pMomentumTimer->IsRunning())
            {
                // Used for trimming later on
                if (g_ReplaySystem.m_bRecording)
                {
                    g_ReplaySystem.SetTimerStartTick(gpGlobals->tickcount);
                }

                pPlayer->m_SrvData.m_RunData.m_bTimerRunning = g_pMomentumTimer->IsRunning();
                // Used for spectating later on
                pPlayer->m_SrvData.m_RunData.m_iStartTick = gpGlobals->tickcount;

                // Are we in mid air when we started? If so, our first jump should be 1, not 0
                if (pPlayer->m_bInAirDueToJump)
                {
                    pPlayer->m_RunStats.SetZoneJumps(0, 1);
                    pPlayer->m_RunStats.SetZoneJumps(pPlayer->m_SrvData.m_RunData.m_iCurrentZone, 1);
                }
            }
        }
        else
        {
            // MOM_TODO: Find a better way of doing this
            // If we can't start the run, play a warning sound
            pPlayer->EmitSound("Watermelon.Scrape");
        }
        pPlayer->m_SrvData.m_RunData.m_bIsInZone = false;
        pPlayer->m_SrvData.m_RunData.m_bMapFinished = false;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            pGhost->m_SrvData.m_RunData.m_bIsInZone = false;
            pGhost->m_SrvData.m_RunData.m_bMapFinished = false;
            pGhost->m_SrvData.m_RunData.m_bTimerRunning = true;
            pGhost->m_SrvData.m_RunData.m_iStartTick = gpGlobals->tickcount;
            pGhost->StartTimer(gpGlobals->tickcount);

            // Needed for hud_comparisons
            IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");
            if (timerStateEvent)
            {
                timerStateEvent->SetInt("ent", pGhost->entindex());
                timerStateEvent->SetBool("is_running", true);

                gameeventmanager->FireEvent(timerStateEvent);
            }
        }
    }

    BaseClass::EndTouch(pOther);
}

void CTriggerTimerStart::StartTouch(CBaseEntity *pOther)
{
    g_pMomentumTimer->SetStartTrigger(this);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        pPlayer->ResetRunStats(); // Reset run stats
        pPlayer->m_SrvData.m_RunData.m_bMapFinished = false;
        pPlayer->m_SrvData.m_RunData.m_bTimerRunning = false;
        pPlayer->m_SrvData.m_RunData.m_flRunTime = 0.0f; // MOM_TODO: Do we want to reset this?

        if (g_pMomentumTimer->IsRunning())
        {
            g_pMomentumTimer->Stop(false, false); // Don't stop our replay just yet
            g_pMomentumTimer->DispatchResetMessage();
        }
        else
        {
            // Reset last jump velocity when we enter the start zone without a timer
            pPlayer->m_SrvData.m_RunData.m_flLastJumpVel = 0;

            // Handle the replay recordings
            if (g_ReplaySystem.m_bRecording)
                g_ReplaySystem.StopRecording(true, false);

            g_ReplaySystem.BeginRecording(pPlayer);
        }
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            pGhost->m_SrvData.m_RunData.m_bMapFinished = false;
            pGhost->m_SrvData.m_RunData.m_bTimerRunning = false; // Fixed
        }
    }

    BaseClass::StartTouch(pOther);
}

void CTriggerTimerStart::Spawn()
{
    // We don't want negative velocities (We're checking against an absolute value)
    m_fBhopLeaveSpeed = abs(m_fBhopLeaveSpeed);
    m_angLook.z = 0.0f; // Reset roll since mappers will never stop ruining everything.
    BaseClass::Spawn();
}
void CTriggerTimerStart::SetMaxLeaveSpeed(const float pBhopLeaveSpeed) { m_fBhopLeaveSpeed = pBhopLeaveSpeed; }
void CTriggerTimerStart::SetLookAngles(const QAngle &newang) { m_angLook = newang; }
void CTriggerTimerStart::SetIsLimitingSpeed(const bool bIsLimitSpeed)
{
    if (bIsLimitSpeed)
    {
        if (!HasSpawnFlags(SF_LIMIT_LEAVE_SPEED))
        {
            AddSpawnFlags(SF_LIMIT_LEAVE_SPEED);
        }
    }
    else
    {
        if (HasSpawnFlags(SF_LIMIT_LEAVE_SPEED))
        {
            RemoveSpawnFlags(SF_LIMIT_LEAVE_SPEED);
        }
    }
}
void CTriggerTimerStart::SetHasLookAngles(const bool bHasLook)
{
    if (bHasLook)
    {
        if (!HasSpawnFlags(SF_USE_LOOKANGLES))
        {
            AddSpawnFlags(SF_USE_LOOKANGLES);
        }
    }
    else
    {
        if (HasSpawnFlags(SF_USE_LOOKANGLES))
        {
            RemoveSpawnFlags(SF_USE_LOOKANGLES);
        }
    }
}
//----------------------------------------------------------------------------------------------

//----------- CTriggerTimerStop ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, CTriggerTimerStop);

void CTriggerTimerStop::StartTouch(CBaseEntity *pOther)
{
    IGameEvent *stageEvent = nullptr;
    // If timer is already stopped, there's nothing to stop (No run state effect to play)
    if (pOther->IsPlayer())
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);

        g_pMomentumTimer->SetEndTrigger(this);
        if (g_pMomentumTimer->IsRunning() && !pPlayer->IsSpectatingGhost())
        {
            int zoneNum = pPlayer->m_SrvData.m_RunData.m_iCurrentZone;

            // This is needed so we have an ending velocity.

            const float endvel = pPlayer->GetLocalVelocity().Length();
            const float endvel2D = pPlayer->GetLocalVelocity().Length2D();

            pPlayer->m_RunStats.SetZoneExitSpeed(zoneNum, endvel, endvel2D);

            // Check to see if we should calculate the timer offset fix
            if (ContainsPosition(pPlayer->GetPrevOrigin()))
                DevLog("PrevOrigin inside of end trigger, not calculating offset!\n");
            else
            {
                DevLog("Previous origin is NOT inside the trigger, calculating offset...\n");
                g_pMomentumTimer->CalculateTickIntervalOffset(pPlayer, g_pMomentumTimer->ZONETYPE_END);
            }

            // This is needed for the final stage
            pPlayer->m_RunStats.SetZoneTime(zoneNum, g_pMomentumTimer->GetCurrentTime() -
                                                         pPlayer->m_RunStats.GetZoneEnterTime(zoneNum));

            // Ending velocity checks

            float finalVel = endvel;
            float finalVel2D = endvel2D;

            if (endvel <= pPlayer->m_RunStats.GetZoneVelocityMax(0, false))
                finalVel = pPlayer->m_RunStats.GetZoneVelocityMax(0, false);

            if (endvel2D <= pPlayer->m_RunStats.GetZoneVelocityMax(0, true))
                finalVel2D = pPlayer->m_RunStats.GetZoneVelocityMax(0, true);

            pPlayer->m_RunStats.SetZoneVelocityMax(0, finalVel, finalVel2D);
            pPlayer->m_RunStats.SetZoneExitSpeed(0, endvel, endvel2D);

            // Stop the timer
            g_pMomentumTimer->Stop(true);
            pPlayer->m_SrvData.m_RunData.m_flRunTime = g_pMomentumTimer->GetLastRunTime();
            // The map is now finished, show the mapfinished panel
            pPlayer->m_SrvData.m_RunData.m_bMapFinished = true;
            pPlayer->m_SrvData.m_RunData.m_bTimerRunning = false;
        }

        stageEvent = gameeventmanager->CreateEvent("zone_enter");

        pPlayer->m_SrvData.m_RunData.m_bIsInZone = true;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            stageEvent = gameeventmanager->CreateEvent("zone_enter");
            pGhost->m_SrvData.m_RunData.m_bMapFinished = true;
            pGhost->m_SrvData.m_RunData.m_bTimerRunning = false;
            pGhost->m_SrvData.m_RunData.m_bIsInZone = true;

            // Needed for hud_comparisons
            IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");
            if (timerStateEvent)
            {
                timerStateEvent->SetInt("ent", pGhost->entindex());
                timerStateEvent->SetBool("is_running", false);

                gameeventmanager->FireEvent(timerStateEvent);
            }
            pGhost->StopTimer();
            // MOM_TODO: Maybe play effects if the player is racing against us and lost?
        }
    }

    // Used by speedometer
    if (stageEvent)
    {
        gameeventmanager->FireEvent(stageEvent);
    }

    BaseClass::StartTouch(pOther);
}
void CTriggerTimerStop::EndTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pMomPlayer = ToCMOMPlayer(pOther);
    int lastZoneNumber = -1;
    if (pMomPlayer)
    {
        pMomPlayer->SetLaggedMovementValue(1.0f);            // Reset slow motion
        pMomPlayer->m_SrvData.m_RunData.m_bIsInZone = false; // Update status
        lastZoneNumber = pMomPlayer->m_SrvData.m_RunData.m_iCurrentZone;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            pGhost->m_SrvData.m_RunData.m_bIsInZone = false;
            lastZoneNumber = pGhost->m_SrvData.m_RunData.m_iCurrentZone;
        }
    }
    BaseClass::EndTouch(pOther);
}
//----------------------------------------------------------------------------------------------

//---------- CTriggerCheckpoint ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_checkpoint, CTriggerCheckpoint);

BEGIN_DATADESC(CTriggerCheckpoint)
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint"), END_DATADESC();

void CTriggerCheckpoint::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        pPlayer->SetCurrentCheckpointTrigger(this);
        pPlayer->RemoveAllOnehops();
    }
}
//----------------------------------------------------------------------------------------------

//------------- CFilterCheckpoint --------------------------------------------------------------
LINK_ENTITY_TO_CLASS(filter_activator_checkpoint, CFilterCheckpoint);

BEGIN_DATADESC(CFilterCheckpoint)
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint")
END_DATADESC();

bool CFilterCheckpoint::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pEntity);
    return (pPlayer && pPlayer->GetCurrentCheckpointTrigger() &&
            pPlayer->GetCurrentCheckpointTrigger()->GetCheckpointNumber() >= m_iCheckpointNumber);
}
//----------------------------------------------------------------------------------------------

//----------- CTriggerTeleport -----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_teleport, CTriggerTeleportEnt);

BEGIN_DATADESC(CTriggerTeleportEnt)
DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop")
, DEFINE_KEYFIELD(m_bResetAngles, FIELD_BOOLEAN, "resetang"), END_DATADESC();

void CTriggerTeleportEnt::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);

    // SF_TELE_ONEXIT defaults to 0 so ents that inherit from this class and call this method DO fire the tp logic
    if (pOther && !HasSpawnFlags(SF_TELE_ONEXIT))
    {
        HandleTeleport(pOther);
    }
}

void CTriggerTeleportEnt::EndTouch(CBaseEntity *pOther)
{
    BaseClass::EndTouch(pOther);

    if (pOther && HasSpawnFlags(SF_TELE_ONEXIT))
    {
        HandleTeleport(pOther);
    }
}

void CTriggerTeleportEnt::HandleTeleport(CBaseEntity *pOther)
{
    if (pOther)
    {
        if (!pDestinationEnt)
        {
            if (m_target != NULL_STRING)
                pDestinationEnt = gEntList.FindEntityByName(nullptr, m_target, nullptr, pOther, pOther);
            else
            {
                DevWarning("CTriggerTeleport cannot teleport, pDestinationEnt and m_target are null!\n");
                return;
            }
        }

        if (!PassesTriggerFilters(pOther))
            return;

        if (pDestinationEnt) // ensuring not null
        {
            Vector tmp = pDestinationEnt->GetAbsOrigin();
            // make origin adjustments. (origin in center, not at feet)
            tmp.z -= pOther->WorldAlignMins().z;

            pOther->Teleport(&tmp, m_bResetAngles ? &pDestinationEnt->GetAbsAngles() : nullptr,
                             m_bResetVelocity ? &vec3_origin : nullptr);
            AfterTeleport();
        }
    }
}

//----------------------------------------------------------------------------------------------

//----------- CTriggerTeleportCheckpoint -------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_teleport_checkpoint, CTriggerTeleportCheckpoint);

void CTriggerTeleportCheckpoint::StartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
        SetDestinationEnt(pPlayer->GetCurrentCheckpointTrigger());
    BaseClass::StartTouch(pOther);
}
//-----------------------------------------------------------------------------------------------

//------------ CTriggerOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_onehop, CTriggerOnehop);

BEGIN_DATADESC(CTriggerOnehop)
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold")
END_DATADESC();

CTriggerOnehop::CTriggerOnehop() : m_fStartTouchedTime(-1.0), m_fMaxHoldSeconds(1){};

void CTriggerOnehop::StartTouch(CBaseEntity *pOther)
{
    // Needed for the Think() function of this class
    CBaseMomentumTrigger::StartTouch(pOther);

    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        m_fStartTouchedTime = gpGlobals->realtime;
        if (pPlayer->FindOnehopOnList(this))
        {
            SetDestinationEnt(pPlayer->GetCurrentCheckpointTrigger());
            HandleTeleport(pPlayer); // Does the teleporting
        }
        else
        {
            pPlayer->AddOnehop(this);
        }
    }
}

void CTriggerOnehop::Think()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && m_fStartTouchedTime > 0 && IsTouching(pPlayer) &&
        gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds)
    {
        SetDestinationEnt(pPlayer->GetCurrentCheckpointTrigger());
        HandleTeleport(pPlayer);
    }
}
//-----------------------------------------------------------------------------------------------

//------- CTriggerResetOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_resetonehop, CTriggerResetOnehop);

void CTriggerResetOnehop::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
        pPlayer->RemoveAllOnehops();
}
//-----------------------------------------------------------------------------------------------

//---------- CTriggerMultihop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_multihop, CTriggerMultihop);

BEGIN_DATADESC(CTriggerMultihop)
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold")
END_DATADESC();

CTriggerMultihop::CTriggerMultihop() : m_fStartTouchedTime(0.0), m_fMaxHoldSeconds(1) {}

void CTriggerMultihop::StartTouch(CBaseEntity *pOther)
{
    CBaseMomentumTrigger::StartTouch(pOther);
    if (pOther->IsPlayer())
    {
        m_fStartTouchedTime = gpGlobals->realtime;
    }
}

void CTriggerMultihop::EndTouch(CBaseEntity *pOther)
{
    // We don't want to keep checking for tp
    m_fStartTouchedTime = -1.0f;
    CBaseMomentumTrigger::EndTouch(pOther);
}

void CTriggerMultihop::Think()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && m_fStartTouchedTime > 0 && IsTouching(pPlayer) &&
        gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds)
    {
        SetDestinationEnt(pPlayer->GetCurrentCheckpointTrigger());
        HandleTeleport(pPlayer);
    }
}
//-----------------------------------------------------------------------------------------------

//--------- CTriggerUserInput -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_userinput, CTriggerUserInput);

BEGIN_DATADESC(CTriggerUserInput)
DEFINE_KEYFIELD(m_eKey, FIELD_INTEGER, "lookedkey"), DEFINE_OUTPUT(m_OnKeyPressed, "OnKeyPressed"), END_DATADESC();

void CTriggerUserInput::Think()
{
    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer && IsTouching(pPlayer) && pPlayer->m_nButtons & m_ButtonRep)
    {
        m_OnKeyPressed.FireOutput(pPlayer, this);
    }
    BaseClass::Think();
}

void CTriggerUserInput::Spawn()
{
    switch (m_eKey)
    {
    case forward:
        m_ButtonRep = IN_FORWARD;
        break;
    case back:
        m_ButtonRep = IN_BACK;
        break;
    case moveleft:
        m_ButtonRep = IN_MOVELEFT;
        break;
    case moveright:
        m_ButtonRep = IN_MOVERIGHT;
        break;
    case jump:
        m_ButtonRep = IN_JUMP;
        break;
    case duck:
        m_ButtonRep = IN_DUCK;
        break;
    case attack:
        m_ButtonRep = IN_ATTACK;
        break;
    case attack2:
        m_ButtonRep = IN_ATTACK2;
        break;
    case reload:
        m_ButtonRep = IN_RELOAD;
        break;
    default:
        DevWarning("Passed unhandled key press");
        break;
    }
    BaseClass::Spawn();
}
//-----------------------------------------------------------------------------------------------

//--------- CTriggerLimitMovement -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_limitmovement, CTriggerLimitMovement);

void CTriggerLimitMovement::StartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        if (HasSpawnFlags(LIMIT_JUMP))
        {
            pPlayer->DisableButtons(IN_JUMP);
        }
        if (HasSpawnFlags(LIMIT_CROUCH))
        {
            pPlayer->DisableButtons(IN_DUCK);
        }
        if (HasSpawnFlags(LIMIT_BHOP))
        {
            pPlayer->m_SrvData.m_bPreventPlayerBhop = true;
        }
    }

    BaseClass::StartTouch(pOther);
}

void CTriggerLimitMovement::EndTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        pPlayer->EnableButtons(IN_JUMP);
        pPlayer->EnableButtons(IN_DUCK);

        if (HasSpawnFlags(LIMIT_BHOP))
            pPlayer->m_SrvData.m_bPreventPlayerBhop = false;
    }

    BaseClass::EndTouch(pOther);
}
//-----------------------------------------------------------------------------------------------

//---------- CFuncShootBoost --------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(func_shootboost, CFuncShootBoost);

BEGIN_DATADESC(CFuncShootBoost)
DEFINE_KEYFIELD(m_vPushDir, FIELD_VECTOR, "pushdir")
, DEFINE_KEYFIELD(m_fPushForce, FIELD_FLOAT, "force"), DEFINE_KEYFIELD(m_iIncrease, FIELD_INTEGER, "increase"),
    END_DATADESC();

void CFuncShootBoost::Spawn()
{
    BaseClass::Spawn();
    // temporary
    m_debugOverlays |= (OVERLAY_BBOX_BIT | OVERLAY_TEXT_BIT);
    if (m_target != NULL_STRING)
        m_Destination = gEntList.FindEntityByName(nullptr, m_target);
}

int CFuncShootBoost::OnTakeDamage(const CTakeDamageInfo &info)
{
    CBaseEntity *pInflictor = info.GetAttacker();
    if (pInflictor)
    {
        Vector finalVel = m_vPushDir.Normalized() * m_fPushForce;
        switch (m_iIncrease)
        {
        case 0:
            break;
        case 1:
            finalVel += pInflictor->GetAbsVelocity();
            break;
        case 2:
            if (finalVel.LengthSqr() < pInflictor->GetAbsVelocity().LengthSqr())
                finalVel = pInflictor->GetAbsVelocity();
            break;
        case 3: // The description of this method says the player velocity is increaed by final velocity,
            // but we're just adding one vec to the other, which is not quite the same
            if (finalVel.LengthSqr() < pInflictor->GetAbsVelocity().LengthSqr())
                finalVel += pInflictor->GetAbsVelocity();
            break;
        case 4:
            pInflictor->SetBaseVelocity(finalVel);
            break;
        default:
            DevWarning("CFuncShootBoost:: %i not recognised as valid for m_iIncrease", m_iIncrease);
            break;
        }
        if (m_Destination)
        {
            if (static_cast<CBaseTrigger *>(m_Destination)->IsTouching(pInflictor))
            {
                pInflictor->SetAbsVelocity(finalVel);
            }
        }
        else
        {
            pInflictor->SetAbsVelocity(finalVel);
        }
    }
    // As we don't want to break it, we don't call BaseClass::OnTakeDamage(info);
    // OnTakeDamage returns the damage dealt
    return info.GetDamage();
}
//-----------------------------------------------------------------------------------------------

//---------- CTriggerMomentumPush ---------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_push, CTriggerMomentumPush);

BEGIN_DATADESC(CTriggerMomentumPush)
DEFINE_KEYFIELD(m_vPushDir, FIELD_VECTOR, "pushdir")
, DEFINE_KEYFIELD(m_fPushForce, FIELD_FLOAT, "force"),
    DEFINE_KEYFIELD(m_iIncrease, FIELD_INTEGER, "increase") END_DATADESC();

CTriggerMomentumPush::CTriggerMomentumPush(){};

void CTriggerMomentumPush::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    if (pOther && HasSpawnFlags(SF_PUSH_ONSTART) && pOther->IsPlayer())
        OnSuccessfulTouch(pOther);
}

void CTriggerMomentumPush::EndTouch(CBaseEntity *pOther)
{
    BaseClass::EndTouch(pOther);
    if (pOther && HasSpawnFlags(SF_PUSH_ONEND) && pOther->IsPlayer())
        OnSuccessfulTouch(pOther);
}

void CTriggerMomentumPush::OnSuccessfulTouch(CBaseEntity *pOther)
{
    if (pOther)
    {
        Vector finalVel;
        if (HasSpawnFlags(SF_PUSH_DIRECTION_AS_FINAL_FORCE))
            finalVel = m_vPushDir;
        else
            finalVel = m_vPushDir.Normalized() * m_fPushForce;
        switch (m_iIncrease)
        {
        case 0:
            break;
        case 1:
            finalVel += pOther->GetAbsVelocity();
            break;
        case 2:
            if (finalVel.LengthSqr() < pOther->GetAbsVelocity().LengthSqr())
                finalVel = pOther->GetAbsVelocity();
            break;
        case 3:
            pOther->SetBaseVelocity(finalVel);
            break;
        default:
            DevWarning("CTriggerMomentumPush:: %i not recognised as valid for m_iIncrease", m_iIncrease);
            break;
        }

        pOther->SetAbsVelocity(finalVel);
    }
}
//-----------------------------------------------------------------------------------------------

//--------- CTriggerSlide -------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(trigger_momentum_slide, CTriggerSlide);

BEGIN_DATADESC(CTriggerSlide)
DEFINE_KEYFIELD(m_bStuckOnGround, FIELD_BOOLEAN, "StuckOnGround")
, DEFINE_KEYFIELD(m_bAllowingJump, FIELD_BOOLEAN, "AllowingJump"),
    DEFINE_KEYFIELD(m_bDisableGravity, FIELD_BOOLEAN, "DisableGravity"),
    DEFINE_KEYFIELD(m_bFixUpsideSlope, FIELD_BOOLEAN, "FixUpsideSlope")
    //,DEFINE_KEYFIELD(m_flSlideGravity, FIELD_FLOAT, "GravityValue")
    END_DATADESC();

// Sometimes when a trigger is touching another trigger, it disables the slide when it shouldn't, because endtouch was
// called for one trigger but the player was actually into another trigger, so we must check if we were inside of any of
// thoses.
void CTriggerSlide::StartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(pOther);

    if (pPlayer != nullptr)
    {
        pPlayer->m_SrvData.m_SlideData.SetEnabled();
        pPlayer->m_SrvData.m_SlideData.SetAllowingJump(m_bAllowingJump);
        pPlayer->m_SrvData.m_SlideData.SetStuckToGround(m_bStuckOnGround);
        pPlayer->m_SrvData.m_SlideData.SetEnableGravity(!m_bDisableGravity);
        pPlayer->m_SrvData.m_SlideData.SetFixUpsideSlope(m_bFixUpsideSlope);
        pPlayer->m_SrvData.m_SlideData.IncTouchCounter();
        // engine->Con_NPrintf( 0, "StartTouch: %i\n" , entindex() );
        // pPlayer->m_SrvData.m_SlideData.SetGravity(m_flSlideGravity);
    }

    BaseClass::StartTouch(pOther);
}

void CTriggerSlide::EndTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(pOther);

    if (pPlayer != nullptr)
    {
        pPlayer->m_SrvData.m_SlideData.DecTouchCounter();

        if (pPlayer->m_SrvData.m_SlideData.GetTouchCounter() == 0)
            pPlayer->m_SrvData.m_SlideData.Reset();

        // engine->Con_NPrintf( 1 , "EndTouch: %i\n" , entindex() );
    }

    BaseClass::EndTouch(pOther);
}
//-----------------------------------------------------------------------------------------------
