#include "cbase.h"
#include "Timer.h"
#include "in_buttons.h"
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
    if (pOther->IsPlayer())
    {
        g_Timer.SetCurrentStage(this);

        int stageNum = this->GetStageNumber();
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
        if (stageNum > 1) //Only for stages other than the first one; first stage stats NEED to be OnEndTouch
        {
            IGameEvent *stageEvent = gameeventmanager->CreateEvent("new_stage");
            if (stageEvent)
            {
                stageEvent->SetInt("stage_num", stageNum);
                stageEvent->SetInt("stage_ticks", g_Timer.GetStageTicks(stageNum));
                stageEvent->SetFloat("avg_sync", pPlayer->m_flStageStrafeSyncAvg[stageNum]);
                stageEvent->SetFloat("avg_sync2", pPlayer->m_flStageStrafeSync2Avg[stageNum]);
                stageEvent->SetFloat("avg_vel", pPlayer->m_flStageVelocityAvg[stageNum]);
                stageEvent->SetFloat("max_vel", pPlayer->m_flStageVelocityMax[stageNum]);
                stageEvent->SetInt("num_strafes", pPlayer->m_nStageStrafes[stageNum]);
                stageEvent->SetInt("num_jumps", pPlayer->m_nStageJumps[stageNum]);

                ConVarRef hvel("mom_speedometer_hvel");
                pPlayer->m_flStageEnterVelocity[stageNum] = hvel.GetBool() ? pPlayer->GetLocalVelocity().Length2D() : pPlayer->GetLocalVelocity().Length();
                stageEvent->SetFloat("stage_enter_vel", pPlayer->m_flStageEnterVelocity[stageNum]);
                gameeventmanager->FireEvent(stageEvent);
            }
        }
    }
}
void CTriggerStage::EndTouch(CBaseEntity *pOther)
{
    BaseClass::EndTouch(pOther);
    if (pOther->IsPlayer())
    {
        int stageNum = this->GetStageNumber();
        if (stageNum == 1) //redundant check for this being the first stage so we have to save stats on on EXIT rather than enter
        {
            CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
            IGameEvent *stageEvent = gameeventmanager->CreateEvent("new_stage");
            if (stageEvent)
            {
                stageEvent->SetInt("stage_num", stageNum);
                stageEvent->SetInt("stage_ticks", g_Timer.GetStageTicks(stageNum));
                stageEvent->SetFloat("avg_sync", pPlayer->m_flStageStrafeSyncAvg[stageNum]);
                stageEvent->SetFloat("avg_sync2", pPlayer->m_flStageStrafeSync2Avg[stageNum]);
                stageEvent->SetFloat("avg_vel", pPlayer->m_flStageVelocityAvg[stageNum]);
                stageEvent->SetFloat("max_vel", pPlayer->m_flStageVelocityMax[stageNum]);
                stageEvent->SetInt("num_strafes", pPlayer->m_nStageStrafes[stageNum]);
                stageEvent->SetInt("num_jumps", pPlayer->m_nStageJumps[stageNum]);

                ConVarRef hvel("mom_speedometer_hvel");
                pPlayer->m_flStageEnterVelocity[stageNum] = hvel.GetBool() ? pPlayer->GetLocalVelocity().Length2D() : pPlayer->GetLocalVelocity().Length();
                stageEvent->SetFloat("stage_enter_vel", pPlayer->m_flStageEnterVelocity[stageNum]);
                gameeventmanager->FireEvent(stageEvent);
            }
        }
    }
}
//------------------------------------------------------------------------------------------

//---------- CTriggerTimerStart ------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, CTriggerTimerStart);

BEGIN_DATADESC(CTriggerTimerStart)
DEFINE_KEYFIELD(m_fMaxLeaveSpeed, FIELD_FLOAT, "leavespeed"),
DEFINE_KEYFIELD(m_fBhopLeaveSpeed, FIELD_FLOAT, "bhopleavespeed"),
DEFINE_KEYFIELD(m_angLook, FIELD_VECTOR, "lookangles") 
END_DATADESC()

void CTriggerTimerStart::EndTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer() && !g_Timer.IsPracticeMode(pOther)) // do not start timer if player is in practice mode.
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
        g_Timer.Start(gpGlobals->tickcount);

        if (IsLimitingSpeed())
        {
            Vector velocity = pOther->GetAbsVelocity();
            if (IsLimitingSpeedOnlyXY())
            {
                // Isn't it nice how Vector2D.h doesn't have Normalize() on it?
                // It only has a NormalizeInPlace... Not simple enough for me
                Vector2D vel2D = velocity.AsVector2D();

                if (velocity.AsVector2D().IsLengthGreaterThan(pPlayer->DidPlayerBhop() ? m_fBhopLeaveSpeed : m_fMaxLeaveSpeed))
                {
                    vel2D = ((vel2D / vel2D.Length()) * (pPlayer->DidPlayerBhop() ? m_fBhopLeaveSpeed : m_fMaxLeaveSpeed));
                    pOther->SetAbsVelocity(Vector(vel2D.x, vel2D.y, velocity.z));
                }
            }
            // XYZ limit (this is likely never going to be used, or at least, it shouldn't be)
            else
            {
                if (velocity.IsLengthGreaterThan((pPlayer->DidPlayerBhop() ? m_fBhopLeaveSpeed : m_fMaxLeaveSpeed)))
                    pOther->SetAbsVelocity(velocity.Normalized() *
                    (pPlayer->DidPlayerBhop() ? m_fBhopLeaveSpeed : m_fMaxLeaveSpeed));
            }
        }
    }
    // stop thinking on end touch
    IGameEvent *mapZoneEvent = gameeventmanager->CreateEvent("player_inside_mapzone");
    if (mapZoneEvent)
    {
        mapZoneEvent->SetBool("inside_startzone", false);
        gameeventmanager->FireEvent(mapZoneEvent);
    }
    //reset strafe sync ratio
    SetNextThink(-1);
    BaseClass::EndTouch(pOther);
}

void CTriggerTimerStart::StartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    g_Timer.SetStartTrigger(this);
    if (pOther->IsPlayer() && g_Timer.IsRunning())
    {
        g_Timer.Stop(false);
        g_Timer.DispatchResetMessage();
    }
    pPlayer->m_flLastJumpVel = 0; //also reset last jump velocity when we enter the start zone
    IGameEvent *mapZoneEvent = gameeventmanager->CreateEvent("player_inside_mapzone");
    if (mapZoneEvent)
    {
        mapZoneEvent->SetBool("inside_startzone", true);
        mapZoneEvent->SetBool("map_finished", false);
        gameeventmanager->FireEvent(mapZoneEvent);
    }
    // start thinking
    SetNextThink(gpGlobals->curtime);
    BaseClass::StartTouch(pOther);
}

void CTriggerTimerStart::Spawn()
{
    // We don't want negative velocities (We're checking against an absolute value)
    m_fMaxLeaveSpeed = abs(m_fMaxLeaveSpeed);

    m_fBhopLeaveSpeed = abs(m_fBhopLeaveSpeed);
    m_angLook.z = 0.0f; // Reset roll since mappers will never stop ruining everything.
    BaseClass::Spawn();
}

void CTriggerTimerStart::SetMaxLeaveSpeed(float pMaxLeaveSpeed) { m_fMaxLeaveSpeed = abs(pMaxLeaveSpeed);  }
void CTriggerTimerStart::SetBhopLeaveSpeed(float pBhopMaxLeaveSpeed) { m_fBhopLeaveSpeed = abs(pBhopMaxLeaveSpeed); }

void CTriggerTimerStart::SetIsLimitingSpeed(bool pIsLimitingSpeed)
{
    if (pIsLimitingSpeed)
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

void CTriggerTimerStart::SetIsLimitingSpeedOnlyXY(bool pIsLimitingSpeedOnlyXY)
{
    if (pIsLimitingSpeedOnlyXY)
    {
        if (!HasSpawnFlags(SF_LIMIT_LEAVE_SPEED_ONLYXY))
        {
            AddSpawnFlags(SF_LIMIT_LEAVE_SPEED_ONLYXY);
        }
    }
    else
    {
        if (HasSpawnFlags(SF_LIMIT_LEAVE_SPEED_ONLYXY))
        {
            RemoveSpawnFlags(SF_LIMIT_LEAVE_SPEED_ONLYXY);
        }
    }
}

void CTriggerTimerStart::SetIsLimitingBhop(bool bIsLimitBhop)
{
    if (bIsLimitBhop)
    {
        if (!HasSpawnFlags(SF_LIMIT_LEAVE_SPEED_BHOP))
        {
            AddSpawnFlags(SF_LIMIT_LEAVE_SPEED_BHOP);
        }
    }
    else
    {
        if (HasSpawnFlags(SF_LIMIT_LEAVE_SPEED_BHOP))
        {
            RemoveSpawnFlags(SF_LIMIT_LEAVE_SPEED_BHOP);
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
void CTriggerTimerStart::SetLookAngles(QAngle newang) { m_angLook = newang; }
//----------------------------------------------------------------------------------------------

//----------- CTriggerTimerStop ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, CTriggerTimerStop);

void CTriggerTimerStop::StartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    IGameEvent *timerStopEvent = gameeventmanager->CreateEvent("timer_stopped");
    IGameEvent *mapZoneEvent = gameeventmanager->CreateEvent("player_inside_mapzone");
    ConVarRef hvel("mom_speedometer_hvel");

    BaseClass::StartTouch(pOther);
    // If timer is already stopped, there's nothing to stop (No run state effect to play)
    if (pOther->IsPlayer() && g_Timer.IsRunning())
    {
        //send run stats via GameEventManager
        if (timerStopEvent)
        {
            timerStopEvent->SetFloat("avg_sync", pPlayer->m_flStageStrafeSyncAvg[0]);
            timerStopEvent->SetFloat("avg_sync2", pPlayer->m_flStageStrafeSync2Avg[0]);
            timerStopEvent->SetFloat("avg_vel", pPlayer->m_flStageVelocityAvg[0]);
            timerStopEvent->SetFloat("max_vel", pPlayer->m_flStageVelocityMax[0]);
            timerStopEvent->SetFloat("start_vel", pPlayer->m_flStartSpeed);
            float endvel = hvel.GetBool() ? pPlayer->GetLocalVelocity().Length2D() : pPlayer->GetLocalVelocity().Length();
            timerStopEvent->SetFloat("end_vel", endvel);
            pPlayer->m_flEndSpeed = endvel; //we have to set end speed here or else it will be saved as 0 
            timerStopEvent->SetInt("num_strafes", pPlayer->m_nStageStrafes[0]);
            timerStopEvent->SetInt("num_jumps", pPlayer->m_nStageJumps[0]);
            gameeventmanager->FireEvent(timerStopEvent);
        }
        if (mapZoneEvent) mapZoneEvent->SetBool("map_finished", true); //broadcast that we finished the map with a timer running
        g_Timer.Stop(true);
    }
    if (mapZoneEvent)
    {
        mapZoneEvent->SetBool("inside_endzone", true);
        gameeventmanager->FireEvent(mapZoneEvent);
    }       
}
void CTriggerTimerStop::EndTouch(CBaseEntity* pOther)
{
    IGameEvent *mapZoneEvent = gameeventmanager->CreateEvent("player_inside_mapzone");
    if (mapZoneEvent)
    {
        mapZoneEvent->SetBool("map_finished", false); //once we leave endzone, we no longer want to display end stats again
        mapZoneEvent->SetBool("inside_endzone", false);
        gameeventmanager->FireEvent(mapZoneEvent);
    }    
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
        g_Timer.SetCurrentCheckpointTrigger(this);
        g_Timer.RemoveAllOnehopsFromList();
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
    return (g_Timer.GetCurrentCheckpoint() &&
            g_Timer.GetCurrentCheckpoint()->GetCheckpointNumber() >= m_iCheckpointNumber);
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
                pDestinationEnt = gEntList.FindEntityByName(NULL, m_target, NULL, pOther, pOther);
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
    SetDestinationEnt(g_Timer.GetCurrentCheckpoint());
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
    SetDestinationEnt(NULL);
    BaseClass::StartTouch(pOther);
    // The above is needed for the Think() function of this class,
    // it's very HACKHACK but it works

    if (pOther->IsPlayer())
    {
        m_fStartTouchedTime = gpGlobals->realtime;
        if (g_Timer.FindOnehopOnList(this) != (-1))
        {
            SetDestinationEnt(g_Timer.GetCurrentCheckpoint());
            BaseClass::StartTouch(pOther);
        }
        else
        {
            if (g_Timer.GetOnehopListCount() > 0)
            {
                // I don't know if Count gets updated for each for, so better be safe than sorry
                // This method shouldn't be slow. Isn't it?
                int c_MaxCount = g_Timer.GetOnehopListCount();
                for (int iIndex = 0; iIndex < c_MaxCount; iIndex++)
                {
                    CTriggerOnehop *thisOnehop = g_Timer.FindOnehopOnList(iIndex);
                    if (thisOnehop != NULL && thisOnehop->HasSpawnFlags(SF_TELEPORT_RESET_ONEHOP))
                        g_Timer.RemoveOnehopFromList(thisOnehop);
                }
            }
            g_Timer.AddOnehopToListTail(this);
        }
    }
}

void CTriggerOnehop::Think()
{
    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer != NULL && m_fStartTouchedTime > 0)
    {
        if (IsTouching(pPlayer) && (gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds))
        {
            SetDestinationEnt(g_Timer.GetCurrentCheckpoint());
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
        g_Timer.RemoveAllOnehopsFromList();
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
    if (pPlayer != NULL && m_fStartTouchedTime > 0)
    {
        if (IsTouching(pPlayer) && (gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds))
        {
            SetDestinationEnt(g_Timer.GetCurrentCheckpoint());
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
    if (pPlayer != NULL && IsTouching(pPlayer) && (pPlayer->m_nButtons & m_ButtonRep))
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
            if (pPlayer->GetGroundEntity() != NULL)
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
        m_Destination = gEntList.FindEntityByName(NULL, m_target);
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
            if (((CBaseTrigger *)m_Destination)->IsTouching(pInflictor))
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