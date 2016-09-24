#include "cbase.h"
#include "Timer.h"
#include "in_buttons.h"
#include "mom_triggers.h"
#include "mom_player.h"
#include "movevars_shared.h"
#include "mom_replay_system.h"
#include "tier0/memdbgon.h"
#include "mom_replay_entity.h"


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
        //Set the current stage to this
        g_Timer->SetCurrentZone(this);
        //Set player run data
        pPlayer->m_RunData.m_bIsInZone = true;
        pPlayer->m_RunData.m_iCurrentZone = stageNum;
        stageEvent = gameeventmanager->CreateEvent("zone_enter");
        if (g_Timer->IsRunning())
        { 
			pPlayer->m_RunStats.SetZoneExitSpeed(stageNum - 1, pPlayer->GetLocalVelocity().Length(), pPlayer->GetLocalVelocity().Length2D());
            g_Timer->CalculateTickIntervalOffset(pPlayer, g_Timer->ZONETYPE_END);
            pPlayer->m_RunStats.SetZoneEnterTime(stageNum, g_Timer->CalculateStageTime(stageNum));
            pPlayer->m_RunStats.SetZoneTime(stageNum - 1,
                pPlayer->m_RunStats.GetZoneEnterTime(stageNum) - pPlayer->m_RunStats.GetZoneEnterTime(stageNum - 1));
        }
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            stageEvent = gameeventmanager->CreateEvent("zone_enter");
            pGhost->m_RunData.m_iCurrentZone = stageNum;
            pGhost->m_RunData.m_bIsInZone = true;       
        }
    }

    //Used by speedometer UI
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
        if (stageNum == 1 || g_Timer->IsRunning())//Timer won't be running if it's the start trigger
        {
            //This handles both the start and stage triggers
            g_Timer->CalculateTickIntervalOffset(pPlayer, g_Timer->ZONETYPE_START);

            float enterVel3D = pPlayer->GetLocalVelocity().Length(), enterVel2D = pPlayer->GetLocalVelocity().Length2D();
            pPlayer->m_RunStats.SetZoneEnterSpeed(stageNum, enterVel3D, enterVel2D);
            if (stageNum == 1)
                pPlayer->m_RunStats.SetZoneEnterSpeed(0, enterVel3D, enterVel2D);

            stageEvent = gameeventmanager->CreateEvent("zone_exit");
        }

        //Status
        pPlayer->m_RunData.m_bIsInZone = false;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_bIsInZone = false;

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
DEFINE_KEYFIELD(m_fBhopLeaveSpeed, FIELD_FLOAT, "bhopleavespeed"),
DEFINE_KEYFIELD(m_angLook, FIELD_VECTOR, "lookangles") 
END_DATADESC()

CTriggerTimerStart::CTriggerTimerStart() : m_angLook(QAngle(0, 0, 0)), m_fBhopLeaveSpeed(250), m_fPunishSpeed(200) {};

void CTriggerTimerStart::EndTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer())
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);

        //surf or other gamemodes has timer start on exiting zone, bhop timer starts when the player jumps
        if (!pPlayer->m_bHasPracticeMode && !g_Timer->IsRunning()) // do not start timer if player is in practice mode or it's already running.
        {
            if (IsLimitingSpeed())
            {
                if (pPlayer->DidPlayerBhop())
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
            } 
            g_Timer->Start(gpGlobals->tickcount);
            if (g_Timer->IsRunning())
            {
                //Used for trimming later on
                if (g_ReplaySystem->GetReplayManager()->Recording())
                {
                    g_ReplaySystem->SetTimerStartTick(gpGlobals->tickcount);
                }

                pPlayer->m_RunData.m_bTimerRunning = g_Timer->IsRunning();
                //Used for spectating later on
                pPlayer->m_RunData.m_iStartTick = gpGlobals->tickcount;
            }
        }
        pPlayer->m_RunData.m_bIsInZone = false;
        pPlayer->m_RunData.m_bMapFinished = false;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_bIsInZone = false;
            pGhost->m_RunData.m_bMapFinished = false;
            pGhost->m_RunData.m_bTimerRunning = true;
            pGhost->m_RunData.m_iStartTick = gpGlobals->tickcount;
            pGhost->StartTimer(gpGlobals->tickcount);

            //Needed for hud_comparisons
            IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");
            if (timerStateEvent)
            {
                timerStateEvent->SetInt("ent", pGhost->entindex());
                timerStateEvent->SetBool("is_running", true);

                gameeventmanager->FireEvent(timerStateEvent);
            }
        }
    }
    // stop thinking on end touch
    SetNextThink(-1);
    BaseClass::EndTouch(pOther);
}

void CTriggerTimerStart::StartTouch(CBaseEntity *pOther)
{
    g_Timer->SetStartTrigger(this);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        pPlayer->ResetRunStats();//Reset run stats
        pPlayer->m_RunData.m_bIsInZone = true;
        pPlayer->m_RunData.m_bMapFinished = false;
        pPlayer->m_RunData.m_bTimerRunning = false;
        pPlayer->m_RunData.m_flLastJumpVel = 0; //also reset last jump velocity when we enter the start zone
        pPlayer->m_RunData.m_flRunTime = 0.0f; //MOM_TODO: Do we want to reset this?

        if (g_Timer->IsRunning())
        {
            g_Timer->Stop(false);//Handles stopping replay recording as well
            g_Timer->DispatchResetMessage();
            //lower the player's speed if they try to jump back into the start zone
        }

        //begin recording replay
		// TODO (OrfeasZ): Do we need to pass a player here?
		if (!g_ReplaySystem->GetReplayManager()->Recording())
		{
			g_ReplaySystem->BeginRecording(pPlayer);
		}
		else
        {
            g_ReplaySystem->StopRecording(pPlayer, true, false);
            g_ReplaySystem->BeginRecording(pPlayer);
        }
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_bIsInZone = true;
            pGhost->m_RunData.m_bMapFinished = false;
			pGhost->m_RunData.m_bTimerRunning = false; //Fixed
        }
    }
    // start thinking
    SetNextThink(gpGlobals->curtime);
    BaseClass::StartTouch(pOther);
}

void CTriggerTimerStart::Spawn()
{
    // We don't want negative velocities (We're checking against an absolute value)
    m_fBhopLeaveSpeed = abs(m_fBhopLeaveSpeed);
    m_angLook.z = 0.0f; // Reset roll since mappers will never stop ruining everything.
    BaseClass::Spawn();
}
void CTriggerTimerStart::SetMaxLeaveSpeed(float pBhopLeaveSpeed) { m_fBhopLeaveSpeed = pBhopLeaveSpeed; }
void CTriggerTimerStart::SetPunishSpeed(float pPunishSpeed) { m_fPunishSpeed = abs(pPunishSpeed); }
void CTriggerTimerStart::SetLookAngles(QAngle newang) { m_angLook = newang; }
void CTriggerTimerStart::SetIsLimitingSpeed(bool bIsLimitSpeed)
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
void CTriggerTimerStart::SetHasLookAngles(bool bHasLook)
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

        g_Timer->SetEndTrigger(this);
        if (g_Timer->IsRunning() && !pPlayer->IsWatchingReplay())
        {
            int zoneNum = pPlayer->m_RunData.m_iCurrentZone;

            // This is needed so we have an ending velocity.
			pPlayer->m_RunStats.SetZoneExitSpeed(zoneNum, pPlayer->GetLocalVelocity().Length(), pPlayer->GetLocalVelocity().Length2D());

            //Check to see if we should calculate the timer offset fix
            if (ContainsPosition(pPlayer->GetPrevOrigin()))
                DevLog("PrevOrigin inside of end trigger, not calculating offset!\n");
            else
            {
                DevLog("Previous origin is NOT inside the trigger, calculating offset...\n");
                g_Timer->CalculateTickIntervalOffset(pPlayer, g_Timer->ZONETYPE_END);
            }

            //This is needed for the final stage
            pPlayer->m_RunStats.SetZoneTime(zoneNum,
                g_Timer->GetCurrentTime() - 
                pPlayer->m_RunStats.GetZoneEnterTime(zoneNum));

            //Ending velocity checks
			float endvel = pPlayer->GetLocalVelocity().Length();
			float endvel2D = pPlayer->GetLocalVelocity().Length2D();

			float finalVel = endvel;
			float finalVel2D = endvel2D;

            if (endvel <= pPlayer->m_RunStats.GetZoneVelocityMax(0, false))
                finalVel = pPlayer->m_RunStats.GetZoneVelocityMax(0, false);

            if (endvel2D <= pPlayer->m_RunStats.GetZoneVelocityMax(0, true))
                finalVel2D = pPlayer->m_RunStats.GetZoneVelocityMax(0, true);

            pPlayer->m_RunStats.SetZoneVelocityMax(0, finalVel, finalVel2D);
            pPlayer->m_RunStats.SetZoneExitSpeed(0, endvel, endvel2D);

            //Stop the timer
            g_Timer->Stop(true);
            pPlayer->m_RunData.m_flRunTime = g_Timer->GetLastRunTime();
            //The map is now finished, show the mapfinished panel
            pPlayer->m_RunData.m_bMapFinished = true;
            pPlayer->m_RunData.m_bTimerRunning = false;
        }

        stageEvent = gameeventmanager->CreateEvent("zone_enter");
        
        pPlayer->m_RunData.m_bIsInZone = true;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            stageEvent = gameeventmanager->CreateEvent("zone_enter");
            pGhost->m_RunData.m_bMapFinished = true;
            pGhost->m_RunData.m_bTimerRunning = false;
            pGhost->m_RunData.m_bIsInZone = true;

            //Needed for hud_comparisons
            IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");
            if (timerStateEvent)
            {
                timerStateEvent->SetInt("ent", pGhost->entindex());
                timerStateEvent->SetBool("is_running", false);

                gameeventmanager->FireEvent(timerStateEvent);
            }
            pGhost->StopTimer();
            //MOM_TODO: Maybe play effects if the player is racing against us and lost?
        }
    }

    //Used by speedometer
    if (stageEvent)
    {
        gameeventmanager->FireEvent(stageEvent);
    }

    BaseClass::StartTouch(pOther);
}
void CTriggerTimerStop::EndTouch(CBaseEntity* pOther)
{
    CMomentumPlayer *pMomPlayer = ToCMOMPlayer(pOther);
    int lastZoneNumber = -1;
    if (pMomPlayer)
    {
        pMomPlayer->SetLaggedMovementValue(1.0f);//Reset slow motion
        pMomPlayer->m_RunData.m_bMapFinished = false;//Close the hud_mapfinished panel
        pMomPlayer->m_RunData.m_bIsInZone = false;//Update status
        lastZoneNumber = pMomPlayer->m_RunData.m_iCurrentZone;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_bMapFinished = false;
            pGhost->m_RunData.m_bIsInZone = false;
            lastZoneNumber = pGhost->m_RunData.m_iCurrentZone;
        }
    }
    BaseClass::EndTouch(pOther);
}
//----------------------------------------------------------------------------------------------

//---------- CTriggerCheckpoint ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_checkpoint, CTriggerCheckpoint);

BEGIN_DATADESC(CTriggerCheckpoint)
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint"), 
END_DATADESC()

void CTriggerCheckpoint::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    if (pOther->IsPlayer())
    {
        g_Timer->SetCurrentCheckpointTrigger(this);
        g_Timer->RemoveAllOnehopsFromList();
    }
}
//----------------------------------------------------------------------------------------------

//------------- CFilterCheckpoint --------------------------------------------------------------
LINK_ENTITY_TO_CLASS(filter_activator_checkpoint, CFilterCheckpoint);

BEGIN_DATADESC(CFilterCheckpoint)
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint")
END_DATADESC()

bool CFilterCheckpoint::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
    return (g_Timer->GetCurrentCheckpoint() &&
        g_Timer->GetCurrentCheckpoint()->GetCheckpointNumber() >= m_iCheckpointNumber);
}
//----------------------------------------------------------------------------------------------

//----------- CTriggerTeleport -----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_teleport, CTriggerTeleportEnt);

BEGIN_DATADESC(CTriggerTeleportEnt)
DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop"),
DEFINE_KEYFIELD(m_bResetAngles, FIELD_BOOLEAN, "resetang")
END_DATADESC()

void CTriggerTeleportEnt::StartTouch(CBaseEntity *pOther)
{
    if (pOther)
    {
        BaseClass::StartTouch(pOther);

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
    SetDestinationEnt(g_Timer->GetCurrentCheckpoint());
    BaseClass::StartTouch(pOther);
}
//-----------------------------------------------------------------------------------------------

//------------ CTriggerOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_onehop, CTriggerOnehop);

BEGIN_DATADESC(CTriggerOnehop)
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold")
END_DATADESC()

CTriggerOnehop::CTriggerOnehop() : m_fStartTouchedTime(0.0), m_fMaxHoldSeconds(1) {};

void CTriggerOnehop::StartTouch(CBaseEntity *pOther)
{
    SetDestinationEnt(nullptr);
    BaseClass::StartTouch(pOther);
    // The above is needed for the Think() function of this class,
    // it's very HACKHACK but it works

    if (pOther->IsPlayer())
    {
        m_fStartTouchedTime = gpGlobals->realtime;
        if (g_Timer->FindOnehopOnList(this) != (-1))
        {
            SetDestinationEnt(g_Timer->GetCurrentCheckpoint());
            BaseClass::StartTouch(pOther);
        }
        else
        {
            if (g_Timer->GetOnehopListCount() > 0)
            {
                // I don't know if Count gets updated for each for, so better be safe than sorry
                // This method shouldn't be slow. Isn't it?
                int c_MaxCount = g_Timer->GetOnehopListCount();
                for (int iIndex = 0; iIndex < c_MaxCount; iIndex++)
                {
                    CTriggerOnehop *thisOnehop = g_Timer->FindOnehopOnList(iIndex);
                    if (thisOnehop != nullptr && thisOnehop->HasSpawnFlags(SF_TELEPORT_RESET_ONEHOP))
                        g_Timer->RemoveOnehopFromList(thisOnehop);
                }
            }
            g_Timer->AddOnehopToListTail(this);
        }
    }
}

void CTriggerOnehop::Think()
{
    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer != nullptr && m_fStartTouchedTime > 0)
    {
        if (IsTouching(pPlayer) && (gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds))
        {
            SetDestinationEnt(g_Timer->GetCurrentCheckpoint());
            BaseClass::StartTouch(pPlayer);
        }
    }
}
//-----------------------------------------------------------------------------------------------

//------- CTriggerResetOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_resetonehop, CTriggerResetOnehop);

void CTriggerResetOnehop::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    if (pOther->IsPlayer())
        g_Timer->RemoveAllOnehopsFromList();
}
//-----------------------------------------------------------------------------------------------

//---------- CTriggerMultihop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_multihop, CTriggerMultihop);

BEGIN_DATADESC(CTriggerMultihop)
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold")
END_DATADESC()

CTriggerMultihop::CTriggerMultihop() : m_fStartTouchedTime(0.0), m_fMaxHoldSeconds(1) {}

void CTriggerMultihop::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    if (pOther->IsPlayer())
    {
        m_fStartTouchedTime = gpGlobals->realtime;
    }
}

void CTriggerMultihop::EndTouch(CBaseEntity *pOther)
{
    // We don't want to keep checking for tp
    m_fStartTouchedTime = -1.0f;
    BaseClass::EndTouch(pOther);
}

void CTriggerMultihop::Think()
{
    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer != nullptr && m_fStartTouchedTime > 0)
    {
        if (IsTouching(pPlayer) && (gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds))
        {
            SetDestinationEnt(g_Timer->GetCurrentCheckpoint());
            BaseClass::StartTouch(pPlayer);
        }
    }
}
//-----------------------------------------------------------------------------------------------

//--------- CTriggerUserInput -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_userinput, CTriggerUserInput);

BEGIN_DATADESC(CTriggerUserInput)
DEFINE_KEYFIELD(m_eKey, FIELD_INTEGER, "lookedkey"),
DEFINE_OUTPUT(m_OnKeyPressed, "OnKeyPressed"),
END_DATADESC()

void CTriggerUserInput::Think()
{
    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer != nullptr && IsTouching(pPlayer) && (pPlayer->m_nButtons & m_ButtonRep))
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

void CTriggerLimitMovement::Think()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    if (pPlayer && IsTouching(pPlayer))
    {
        if (HasSpawnFlags(LIMIT_BHOP))
        {
            pPlayer->DisableButtons(IN_JUMP);
            // if player in air
            if (pPlayer->GetGroundEntity() != nullptr)
            {
                // only start timer if we havent already started
                if (!m_BhopTimer.HasStarted())
                    m_BhopTimer.Start(FL_BHOP_TIMER);

                // when finished
                if (m_BhopTimer.IsElapsed())
                {
                    pPlayer->EnableButtons(IN_JUMP);
                    m_BhopTimer.Reset();
                }
            }
        }
    }
    // figure out if timer elapsed or not
    if (m_BhopTimer.GetRemainingTime() <= 0)
        m_BhopTimer.Invalidate();
    // DevLog("Bhop Timer Remaining Time:%f\n", m_BhopTimer.GetRemainingTime());

    // HACKHACK - this prevents think from running too fast, breaking the timer
    // and preventing the player from jumping until the timer runs out
    // Thinking every 0.25 seconds seems to feel good, but we can adjust this later
    SetNextThink(gpGlobals->curtime + 0.25);
    BaseClass::Think();
}

void CTriggerLimitMovement::StartTouch(CBaseEntity *pOther)
{
    if (pOther && pOther->IsPlayer())
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
                pPlayer->DisableButtons(IN_JUMP);
            }
        }
    }
    BaseClass::StartTouch(pOther);
}

void CTriggerLimitMovement::EndTouch(CBaseEntity *pOther)
{
    if (pOther && pOther->IsPlayer())
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
        if (pPlayer)
        {
            pPlayer->EnableButtons(IN_JUMP);
            pPlayer->EnableButtons(IN_DUCK);
        }
    }
    m_BhopTimer.Reset();
    BaseClass::EndTouch(pOther);
}
//-----------------------------------------------------------------------------------------------

//---------- CFuncShootBoost --------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(func_shootboost, CFuncShootBoost);

BEGIN_DATADESC(CFuncShootBoost)
DEFINE_KEYFIELD(m_vPushDir, FIELD_VECTOR, "pushdir"),
DEFINE_KEYFIELD(m_fPushForce, FIELD_FLOAT, "force"),
DEFINE_KEYFIELD(m_iIncrease, FIELD_INTEGER, "increase"),
END_DATADESC()

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
DEFINE_KEYFIELD(m_vPushDir, FIELD_VECTOR, "pushdir"),
DEFINE_KEYFIELD(m_fPushForce, FIELD_FLOAT, "force"),
DEFINE_KEYFIELD(m_iIncrease, FIELD_INTEGER, "increase"),
END_DATADESC()

CTriggerMomentumPush::CTriggerMomentumPush() : m_fStartTouchedTime(0.0), m_fMaxHoldSeconds(1) {};

void CTriggerMomentumPush::StartTouch(CBaseEntity *pOther)
{
    if (pOther && HasSpawnFlags(SF_PUSH_ONSTART) && pOther->IsPlayer())
        OnSuccessfulTouch(pOther);
}

void CTriggerMomentumPush::EndTouch(CBaseEntity *pOther)
{
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
