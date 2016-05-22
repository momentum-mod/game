#include "cbase.h"
#include "Timer.h"
#include "in_buttons.h"
#include "mom_triggers.h"
#include "movevars_shared.h"
#include "mom_replay.h"
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
    if (pOther->IsPlayer())
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
        IGameEvent *stageEvent = gameeventmanager->CreateEvent("stage_enter");
        if (stageEvent && pPlayer)
        {
            //Set the current stage to this
            g_Timer->SetCurrentStage(this);
            //Set player run data
            pPlayer->m_RunData.m_bIsInZone = true;
            pPlayer->m_RunData.m_iCurrentZone = stageNum;
            if (g_Timer->IsRunning())
            {
                //MOM_TODO: Shouldn't this (also?) be called upon stage exit?
                if (stageNum != 1) 
                    g_Timer->GetTickIntervalOffset(pPlayer, 2);

                stageEvent->SetInt("stage_num", stageNum);
                stageEvent->SetFloat("stage_enter_time", g_Timer->CalculateStageTime(stageNum));
                stageEvent->SetInt("num_jumps", pPlayer->m_PlayerRunStats.m_iStageJumps[stageNum - 1]);
                stageEvent->SetFloat("num_strafes", pPlayer->m_PlayerRunStats.m_iStageStrafes[stageNum - 1]);
                stageEvent->SetFloat("avg_sync", pPlayer->m_PlayerRunStats.m_flStageStrafeSyncAvg[stageNum - 1]);
                stageEvent->SetFloat("avg_sync2", pPlayer->m_PlayerRunStats.m_flStageStrafeSync2Avg[stageNum - 1]);

                //3D VELOCITY
                stageEvent->SetFloat("max_vel", pPlayer->m_PlayerRunStats.m_flStageVelocityMax[stageNum - 1][0]);
                stageEvent->SetFloat("avg_vel", pPlayer->m_PlayerRunStats.m_flStageVelocityAvg[stageNum - 1][0]);
                pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum - 1][0] = pPlayer->GetLocalVelocity().Length();
                stageEvent->SetFloat("stage_exit_vel", pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum - 1][0]);

                //2D VELOCITY
                stageEvent->SetFloat("max_vel_2D", pPlayer->m_PlayerRunStats.m_flStageVelocityMax[stageNum - 1][1]);
                stageEvent->SetFloat("avg_vel_2D", pPlayer->m_PlayerRunStats.m_flStageVelocityAvg[stageNum - 1][1]);
                pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum - 1][1] = pPlayer->GetLocalVelocity().Length2D();
                stageEvent->SetFloat("stage_exit_vel_2D", pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum - 1][1]);

                gameeventmanager->FireEvent(stageEvent);
            }
            else
            {
                stageEvent->SetInt("stage_num", stageNum);//It's 1, and this resets the stats
                gameeventmanager->FireEvent(stageEvent);
            }
        }
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_iCurrentZone = stageNum;
            pGhost->m_RunData.m_bIsInZone = true;
        }
    }
}
void CTriggerStage::EndTouch(CBaseEntity *pOther)
{
    BaseClass::EndTouch(pOther);
    int stageNum = this->GetStageNumber();
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        if (stageNum == 1 || g_Timer->IsRunning())//Timer won't be running if it's the start trigger
        {
            IGameEvent *stageEvent = gameeventmanager->CreateEvent("stage_exit");
            if (stageEvent)
            {
                //Status
                pPlayer->m_RunData.m_bIsInZone = false;

                //Stage num
                stageEvent->SetInt("stage_num", stageNum);

                //3D VELOCITY
                pPlayer->m_PlayerRunStats.m_flStageEnterSpeed[stageNum][0] = pPlayer->GetLocalVelocity().Length();
                stageEvent->SetFloat("stage_enter_vel", pPlayer->m_PlayerRunStats.m_flStageEnterSpeed[stageNum][0]);

                //2D VELOCITY
                pPlayer->m_PlayerRunStats.m_flStageEnterSpeed[stageNum][1] = pPlayer->GetLocalVelocity().Length2D();
                stageEvent->SetFloat("stage_enter_vel_2D", pPlayer->m_PlayerRunStats.m_flStageEnterSpeed[stageNum][1]);

                gameeventmanager->FireEvent(stageEvent);
            }
        }
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_bIsInZone = false;
        }
    }
}
//------------------------------------------------------------------------------------------

//---------- CTriggerTimerStart ------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, CTriggerTimerStart);

BEGIN_DATADESC(CTriggerTimerStart)
DEFINE_KEYFIELD(m_fBhopLeaveSpeed, FIELD_FLOAT, "bhopleavespeed"),
DEFINE_KEYFIELD(m_angLook, FIELD_VECTOR, "lookangles") 
END_DATADESC()

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
                Vector velocity = pOther->GetAbsVelocity();
                    // Isn't it nice how Vector2D.h doesn't have Normalize() on it?
                    // It only has a NormalizeInPlace... Not simple enough for me
                Vector2D vel2D = velocity.AsVector2D();

                if (pPlayer->DidPlayerBhop())
                {
                    if (velocity.AsVector2D().IsLengthGreaterThan(m_fBhopLeaveSpeed))
                    {
                        vel2D = ((vel2D / vel2D.Length()) * (m_fBhopLeaveSpeed));
                        pOther->SetAbsVelocity(Vector(vel2D.x, vel2D.y, velocity.z));
                    }
                }
            } 
            g_Timer->Start(gpGlobals->tickcount);
            g_Timer->GetTickIntervalOffset(pPlayer, 1);
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
            //MOM_TODO: Make the spectator's timer start
            //pGhost->SetStartTick(gpGlobals->tickcount)
        }
    }

    IGameEvent *movementCountsResetEvent = gameeventmanager->CreateEvent("keypress");
    if (movementCountsResetEvent)
    {
        gameeventmanager->FireEvent(movementCountsResetEvent);
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
        pPlayer->m_RunData.m_flLastJumpVel = 0; //also reset last jump velocity when we enter the start zone

        if (g_Timer->IsRunning())
        {
            g_Timer->Stop(false);//Handles stopping replay recording as well
            g_Timer->DispatchResetMessage();
            //lower the player's speed if they try to jump back into the start zone
        }
        //begin recording replay
        if (!g_ReplaySystem->IsRecording(pPlayer))
            g_ReplaySystem->BeginRecording(pPlayer);
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
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);

    IGameEvent *timerStopEvent = gameeventmanager->CreateEvent("timer_stopped");
    IGameEvent *stageEvent = gameeventmanager->CreateEvent("stage_enter");

    // If timer is already stopped, there's nothing to stop (No run state effect to play)
    if (pPlayer)
    {
        g_Timer->SetEndTrigger(this);
        if (g_Timer->IsRunning() && !pPlayer->IsWatchingReplay())
        {
            //send run stats via GameEventManager
            if (timerStopEvent)
            {
                timerStopEvent->SetFloat("avg_sync", pPlayer->m_PlayerRunStats.m_flStageStrafeSyncAvg[0]);
                timerStopEvent->SetFloat("avg_sync2", pPlayer->m_PlayerRunStats.m_flStageStrafeSync2Avg[0]);
                timerStopEvent->SetInt("num_strafes", pPlayer->m_PlayerRunStats.m_iStageStrafes[0]);
                timerStopEvent->SetInt("num_jumps", pPlayer->m_PlayerRunStats.m_iStageJumps[0]);

                //3D VELCOCITY STATS - INDEX 0
                timerStopEvent->SetFloat("avg_vel", pPlayer->m_PlayerRunStats.m_flStageVelocityAvg[0][0]);
                timerStopEvent->SetFloat("start_vel", pPlayer->m_PlayerRunStats.m_flStageEnterSpeed[0][0]);
                float endvel = pPlayer->GetLocalVelocity().Length();
                timerStopEvent->SetFloat("end_vel", endvel);
                if (endvel > pPlayer->m_PlayerRunStats.m_flStageVelocityMax[0][0])
                    timerStopEvent->SetFloat("max_vel", endvel);
                else
                    timerStopEvent->SetFloat("max_vel", pPlayer->m_PlayerRunStats.m_flStageVelocityMax[0][0]);
                pPlayer->m_PlayerRunStats.m_flStageExitSpeed[0][0] = endvel; //we have to set end speed here or else it will be saved as 0 

                //2D VELOCITY STATS - INDEX 1
                timerStopEvent->SetFloat("avg_vel_2D", pPlayer->m_PlayerRunStats.m_flStageVelocityAvg[0][1]);
                timerStopEvent->SetFloat("start_vel_2D", pPlayer->m_PlayerRunStats.m_flStageEnterSpeed[0][1]);
                float endvel2D = pPlayer->GetLocalVelocity().Length2D();
                timerStopEvent->SetFloat("end_vel_2D", endvel2D);
                if (endvel2D > pPlayer->m_PlayerRunStats.m_flStageVelocityMax[0][1])
                    timerStopEvent->SetFloat("max_vel_2D", endvel2D);
                else
                    timerStopEvent->SetFloat("max_vel_2D", pPlayer->m_PlayerRunStats.m_flStageVelocityMax[0][1]);
                pPlayer->m_PlayerRunStats.m_flStageExitSpeed[0][1] = endvel2D;
                gameeventmanager->FireEvent(timerStopEvent);
            }

            if (stageEvent)
            {
                //The last stage is a bit of a doozy.
                //We need to store it in totalstages + 1 so that comparisons can 
                //call forward for determining time spent on the last stage.
                //We set the stage_num one higher so the last stage can still compare against it
                int stageNum = g_Timer->GetCurrentStageNumber();
                stageEvent->SetInt("stage_num", stageNum + 1);
                //And then put the time we finished at as the enter time for the end trigger
                stageEvent->SetFloat("stage_enter_time", g_Timer->GetLastRunTime());

                //This is needed so we have an ending velocity.
                pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum][0] = pPlayer->GetLocalVelocity().Length();
                stageEvent->SetFloat("stage_exit_vel", pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum][0]);
                pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum][1] = pPlayer->GetLocalVelocity().Length2D();
                stageEvent->SetFloat("stage_exit_vel_2D", pPlayer->m_PlayerRunStats.m_flStageExitSpeed[stageNum][1]);
                gameeventmanager->FireEvent(stageEvent);
            }
            
            //MOM_TODO: BUG: If we teleport into the stop trigger, this will still try to get the offset! We need some
            //check or something, you can see my idea below:
            //    if (pPlayer->GetAbsOrigin().AsVector2D().DistTo(CollisionProp()->OBBCenter().AsVector2D()) > 5.0f)
            //However if the ending trigger is very small, this may end up returning true!
            //My idea was to check if the player is very close to the outside edges (using model/size bounds)
            //Or to just check to see if the previous origin was even in the end trigger or not
            //So I (or somebody who wants to) will probably implement that eventually
            
            g_Timer->GetTickIntervalOffset(pPlayer, 0);

            g_Timer->Stop(true);
            pPlayer->m_RunData.m_bMapFinished = true;

            //MOM_TODO: SLOW DOWN/STOP THE PLAYER HERE!
        }
        
        pPlayer->m_RunData.m_bIsInZone = true;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_bMapFinished = true;
            pGhost->m_RunData.m_bIsInZone = true;
            //MOM_TODO: pGhost->EndRunHud(); //sends a hud timer state message to each spectator
            //MOM_TODO: Maybe also play effects if the player is racing against us and lost?
        }
    }
    BaseClass::StartTouch(pOther);
}
void CTriggerTimerStop::EndTouch(CBaseEntity* pOther)
{
    CMomentumPlayer *pMomPlayer = ToCMOMPlayer(pOther);
    if (pMomPlayer)
    {
        pMomPlayer->m_RunData.m_bMapFinished = false;//Close the hud_mapfinished panel
        pMomPlayer->m_RunData.m_bIsInZone = false;//Update status
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity*>(pOther);
        if (pGhost)
        {
            pGhost->m_RunData.m_bMapFinished = false;
            pGhost->m_RunData.m_bIsInZone = false;
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

            pOther->Teleport(&tmp, m_bResetAngles ? &pDestinationEnt->GetAbsAngles() : NULL,
                             m_bResetVelocity ? &vec3_origin : NULL);
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