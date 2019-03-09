#include "cbase.h"

#include "in_buttons.h"
#include "mom_player_shared.h"
#include "mom_replay_entity.h"
#include "mom_replay_system.h"
#include "mom_system_saveloc.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "mom_system_progress.h"
#include "fmtstr.h"

#include "dt_utlvector_send.h"

#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(CBaseMomentumTrigger, DT_BaseMomentumTrigger)
SendPropUtlVector(SENDINFO_UTLVECTOR(m_vecZonePoints), 32, SendPropVector(NULL, 0, sizeof(Vector))),
SendPropFloat(SENDINFO(m_flZoneHeight)),
END_SEND_TABLE();

// CBaseMomentumTrigger
void CBaseMomentumTrigger::Spawn()
{
    AddSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS);
    BaseClass::Spawn();
    // temporary
    m_debugOverlays |= (OVERLAY_BBOX_BIT | OVERLAY_TEXT_BIT);
}

bool CBaseMomentumTrigger::TestCollision(const Ray_t &ray, unsigned int mask, trace_t &tr)
{
    auto pPhys = VPhysicsGetObject();
    Assert(pPhys);

    physcollision->TraceBox(ray, pPhys->GetCollide(), GetAbsOrigin(), GetAbsAngles(), &tr);
    

    return true;
}

int CBaseMomentumTrigger::GetZoneType()
{
    return MOMZONETYPE_INVALID;
}

void CBaseMomentumTrigger::InitCustomCollision(CPhysCollide *pPhysCollide, const Vector &vecMins, const Vector &vecMaxs)
{
    // We are able to create a vphysics object just fine, but how physics work IN vphysics is no good for us.
    // We'll have to use the same method triggers use,
    // which is using partitions and waiting for engine->SolidMoved to call StartTouch/EndTouch for us
    // from the object (player in our case)
    // For that we need to insert ourselves to the partition system and
    // do a custom collision test.
    // The default collision test only works if the entity is a proper model or brush.
    // In our case, we're neither.
    objectparams_t params = { 0 };
    params.enableCollisions = true;
    params.pGameData = this;
    params.pName = "";
    params.mass = 1.0f;
    params.volume = 1.0f;

    auto pPhys = physenv->CreatePolyObject(pPhysCollide, 0, GetAbsOrigin(), GetAbsAngles(), &params);
    Assert(pPhys);
    
    pPhys->EnableMotion(false);
    pPhys->EnableGravity(false);
    pPhys->SetContents(MASK_SOLID);

    VPhysicsDestroyObject();
    VPhysicsSetObject(pPhys);


    if (CollisionProp()->GetPartitionHandle() == PARTITION_INVALID_HANDLE)
        CollisionProp()->CreatePartitionHandle();
    SetSolid(SOLID_VPHYSICS);

    // We need to set the collision bounds manually
    // The collision bound is used by the partition system.
    SetCollisionBounds(vecMins, vecMaxs);


    // If we ever need ray testing uncomment this.
    AddSolidFlags(/*FSOLID_CUSTOMRAYTEST |*/ FSOLID_CUSTOMBOXTEST);
}

//---------- CTriggerStage -----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stage, CTriggerStage);

BEGIN_DATADESC(CTriggerStage)
    DEFINE_KEYFIELD(m_iStageNumber, FIELD_INTEGER, "stage")
END_DATADESC()

void CTriggerStage::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);
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
        stageEvent->SetInt("ent", pPlayer->entindex());
        stageEvent->SetInt("zone_ent", entindex());
        stageEvent->SetInt("num", stageNum);
        if (g_pMomentumTimer->IsRunning())
        {
            pPlayer->m_RunStats.SetZoneExitSpeed(stageNum - 1, pPlayer->GetLocalVelocity().Length(),
                                                 pPlayer->GetLocalVelocity().Length2D());
            g_pMomentumTimer->CalculateTickIntervalOffset(pPlayer, MOMZONETYPE_STOP);
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
            stageEvent->SetInt("ent", pGhost->entindex());
            stageEvent->SetInt("zone_ent", entindex());
            stageEvent->SetInt("num", stageNum);
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
void CTriggerStage::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);
    int stageNum = GetStageNumber();
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    IGameEvent *stageEvent = nullptr;
    if (pPlayer)
    {
        // Timer won't be running if it's the start trigger
        if ((stageNum == 1 || g_pMomentumTimer->IsRunning()) && !pPlayer->m_SrvData.m_bHasPracticeMode)
        {
            float enterVel3D = pPlayer->GetLocalVelocity().Length(),
                  enterVel2D = pPlayer->GetLocalVelocity().Length2D();
            pPlayer->m_RunStats.SetZoneEnterSpeed(stageNum, enterVel3D, enterVel2D);
            if (stageNum == 1)
                pPlayer->m_RunStats.SetZoneEnterSpeed(0, enterVel3D, enterVel2D);

            stageEvent = gameeventmanager->CreateEvent("zone_exit");
            stageEvent->SetInt("ent", pPlayer->entindex());
            stageEvent->SetInt("num", stageNum);
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
            stageEvent->SetInt("ent", pGhost->entindex());
            stageEvent->SetInt("num", stageNum);
        }
    }

    if (stageEvent)
    {
        gameeventmanager->FireEvent(stageEvent);
    }
}

KeyValues *CTriggerStage::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("stage");
    kv->SetInt("number", GetStageNumber());

    return kv;
}

bool CTriggerStage::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "stage"))
    {
        return false;
    }

    SetName(MAKE_STRING("Stage Trigger"));

    SetStageNumber(kv->GetInt("number"));

    return true;
}

int CTriggerStage::GetZoneType()
{
    return MOMZONETYPE_STAGE;
}
//------------------------------------------------------------------------------------------

//---------- CTriggerTimerStart ------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, CTriggerTimerStart);

BEGIN_DATADESC(CTriggerTimerStart)
    // Since the  m_fBhopLeaveSpeed key is mostly used in a lot of cases, we will use this as reference instead.
    // Don't know if we should use m_flMaxSpeed instead, but yeah.
    DEFINE_KEYFIELD(m_fBhopLeaveSpeed, FIELD_FLOAT, "bhopleavespeed"),
    DEFINE_KEYFIELD(m_angLook, FIELD_VECTOR, "lookangles"),
    DEFINE_KEYFIELD(m_bTimerStartOnJump, FIELD_BOOLEAN, "StartOnJump"),
    DEFINE_KEYFIELD(m_iLimitSpeedType, FIELD_INTEGER, "LimitSpeedType"),
    DEFINE_KEYFIELD(m_iZoneNumber, FIELD_INTEGER, "ZoneNumber")
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CTriggerTimerStart, DT_TriggerTimerStart)
END_SEND_TABLE()

CTriggerTimerStart::CTriggerTimerStart()
    : m_angLook(vec3_angle), m_fBhopLeaveSpeed(250), m_bTimerStartOnJump(false), m_iZoneNumber(0),
      m_iLimitSpeedType(SPEED_NORMAL_LIMIT)
{
}
KeyValues *CTriggerTimerStart::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("start");
    kv->SetFloat("bhopleavespeed", GetMaxLeaveSpeed());
    kv->SetBool("limitingspeed", IsLimitingSpeed());
    kv->SetBool("StartOnJump", StartOnJump());
    kv->SetInt("LimitSpeedType", GetLimitSpeedType());
    kv->SetInt("ZoneNumber", GetZoneNumber());
    if (HasLookAngles())
    {
        kv->SetFloat("yaw", GetLookAngles()[YAW]);
    }

    return kv;
};

bool CTriggerTimerStart::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "start"))
    {
        return false;
    }

    SetName(MAKE_STRING("Start Trigger"));

    SetMaxLeaveSpeed(kv->GetFloat("bhopleavespeed"));
    SetIsLimitingSpeed(kv->GetBool("limitingspeed"));
    SetStartOnJump(kv->GetBool("StartOnJump"));
    SetLimitSpeedType(kv->GetInt("LimitSpeedType"));
    SetZoneNumber(kv->GetInt("ZoneNumber"));

    const float nolook = -190.0f;
    float yaw = kv->GetFloat("yaw", nolook);
    if (!CloseEnough(yaw, nolook))
    {
        SetHasLookAngles(true);
        SetLookAngles(QAngle(0.0f, yaw, 0.0f));
    }
    else
    {
        SetHasLookAngles(false);
    }

    return true;
}

int CTriggerTimerStart::GetZoneType()
{
    return MOMZONETYPE_START;
}

void CTriggerTimerStart::OnEndTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer())
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);

        if (pPlayer)
        {
            pPlayer->m_SrvData.m_bShouldLimitPlayerSpeed = false;
        }

        pPlayer->m_SrvData.m_RunData.m_bIsInZone = false;
    }
    else
    {

        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            pGhost->m_SrvData.m_RunData.m_bIsInZone = false;
            pGhost->m_SrvData.m_RunData.m_bMapFinished = false;
            //pGhost->m_SrvData.m_RunData.m_bTimerRunning = true;
            //pGhost->m_SrvData.m_RunData.m_iStartTick = gpGlobals->tickcount;
            //pGhost->StartTimer(gpGlobals->tickcount);

            // This can't be done anymore here due that the player can start timer in start zone while jumping.
            // Needed for hud_comparisons
            /*IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");
            if (timerStateEvent)
            {
                timerStateEvent->SetInt("ent", pGhost->entindex());
                timerStateEvent->SetBool("is_running", true);

                gameeventmanager->FireEvent(timerStateEvent);
            }*/
        }
    }

    BaseClass::OnEndTouch(pOther);
}

void CTriggerTimerStart::OnStartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        g_pMomentumTimer->SetStartTrigger(this);

        // First of all, hard-cap speed, regardless of re-entry
        if (pPlayer->GetAbsVelocity().IsLengthGreaterThan(300.0f))
            pPlayer->SetAbsVelocity(vec3_origin);

		pPlayer->m_SrvData.m_RunData.m_iBonusZone = m_iZoneNumber;
        pPlayer->m_SrvData.m_RunData.m_bTimerStartOnJump = m_bTimerStartOnJump;
        pPlayer->m_SrvData.m_RunData.m_iLimitSpeedType = m_iLimitSpeedType;
        pPlayer->m_SrvData.m_bShouldLimitPlayerSpeed = IsLimitingSpeed();

        g_pMomentumTimer->Reset();
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            pGhost->m_SrvData.m_RunData.m_iBonusZone = m_iZoneNumber;
            pGhost->m_SrvData.m_RunData.m_bMapFinished = false;
            pGhost->m_SrvData.m_RunData.m_bTimerRunning = false; // Fixed
        }
    }

    BaseClass::OnStartTouch(pOther);
}

void CTriggerTimerStart::Spawn()
{
    // We don't want negative velocities (We're checking against an absolute value)
    m_fBhopLeaveSpeed = fabs(m_fBhopLeaveSpeed);
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

BEGIN_DATADESC(CTriggerTimerStop)
    DEFINE_KEYFIELD(m_iZoneNumber, FIELD_INTEGER, "ZoneNumber")
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CTriggerTimerStop, DT_TriggerTimerStop)
END_SEND_TABLE()

void CTriggerTimerStop::OnStartTouch(CBaseEntity *pOther)
{
    IGameEvent *stageEvent = nullptr;
    // If timer is already stopped, there's nothing to stop (No run state effect to play)
    if (pOther->IsPlayer())
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);

        g_pMomentumTimer->SetEndTrigger(this);

        if (g_pMomentumTimer->IsRunning() && !pPlayer->IsSpectatingGhost() &&
            pPlayer->m_SrvData.m_RunData.m_iBonusZone == m_iZoneNumber && !pPlayer->m_SrvData.m_bHasPracticeMode)
        {
            int zoneNum = pPlayer->m_SrvData.m_RunData.m_iCurrentZone;

            // This is needed so we have an ending velocity.

            const float endvel = pPlayer->GetLocalVelocity().Length();
            const float endvel2D = pPlayer->GetLocalVelocity().Length2D();

            pPlayer->m_RunStats.SetZoneExitSpeed(zoneNum, endvel, endvel2D);

            // Check to see if we should calculate the timer offset fix
            if (ContainsPosition(pPlayer->GetPreviousOrigin()))
                DevLog("PrevOrigin inside of end trigger, not calculating offset!\n");
            else
            {
                DevLog("Previous origin is NOT inside the trigger, calculating offset...\n");
                g_pMomentumTimer->CalculateTickIntervalOffset(pPlayer, MOMZONETYPE_STOP);
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

        // Set only if timer is starting, so the previous stage is saved if the map is not complete even if they go
        // to the end zone?
        pPlayer->m_SrvData.m_RunData.m_iOldZone = pPlayer->m_SrvData.m_RunData.m_iCurrentZone;
        pPlayer->m_SrvData.m_RunData.m_iOldBonusZone = pPlayer->m_SrvData.m_RunData.m_iBonusZone;
        pPlayer->m_SrvData.m_RunData.m_iCurrentZone = 0;
        pPlayer->m_SrvData.m_RunData.m_iBonusZone = m_iZoneNumber;

        stageEvent = gameeventmanager->CreateEvent("zone_enter");
        stageEvent->SetInt("ent", pPlayer->entindex());
        stageEvent->SetInt("zone_ent", entindex());
        stageEvent->SetInt("num", m_iZoneNumber);
        pPlayer->m_SrvData.m_RunData.m_bIsInZone = true;
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            stageEvent = gameeventmanager->CreateEvent("zone_enter");
            stageEvent->SetInt("ent", pGhost->entindex());
            stageEvent->SetInt("zone_ent", pGhost->entindex());
            stageEvent->SetInt("num", m_iZoneNumber);
            pGhost->m_SrvData.m_RunData.m_bIsInZone = true;
            pGhost->m_SrvData.m_RunData.m_iOldZone = pGhost->m_SrvData.m_RunData.m_iCurrentZone;
            pGhost->m_SrvData.m_RunData.m_iOldBonusZone = pGhost->m_SrvData.m_RunData.m_iBonusZone;
            pGhost->m_SrvData.m_RunData.m_iCurrentZone = 0;
            pGhost->m_SrvData.m_RunData.m_iBonusZone = m_iZoneNumber;

            // Needed for hud_comparisons
            IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");
            if (timerStateEvent)
            {
                timerStateEvent->SetInt("ent", pGhost->entindex());
                timerStateEvent->SetBool("is_running", false);

                gameeventmanager->FireEvent(timerStateEvent);
            }

            pGhost->m_SrvData.m_RunData.m_bMapFinished = true;
            pGhost->m_SrvData.m_RunData.m_bTimerRunning = false;

            pGhost->StopTimer();

            // MOM_TODO: Maybe play effects if the player is racing against us and lost?
        }
    }

    // Used by speedometer
    if (stageEvent)
    {
        gameeventmanager->FireEvent(stageEvent);
    }

    BaseClass::OnStartTouch(pOther);
}
void CTriggerTimerStop::OnEndTouch(CBaseEntity *pOther)
{
    IGameEvent *pStageEvent = nullptr;
    CMomentumPlayer *pMomPlayer = ToCMOMPlayer(pOther);
    if (pMomPlayer)
    {
        pMomPlayer->SetLaggedMovementValue(1.0f);            // Reset slow motion
        pMomPlayer->m_SrvData.m_RunData.m_bIsInZone = false; // Update status
        pMomPlayer->m_SrvData.m_RunData.m_iCurrentZone = pMomPlayer->m_SrvData.m_RunData.m_iOldZone;
        pMomPlayer->m_SrvData.m_RunData.m_iBonusZone = pMomPlayer->m_SrvData.m_RunData.m_iOldBonusZone;
        pMomPlayer->m_SrvData.m_RunData.m_bMapFinished = false;

        pStageEvent = gameeventmanager->CreateEvent("zone_enter");
        pStageEvent->SetInt("ent", pMomPlayer->entindex());
        pStageEvent->SetInt("num", m_iZoneNumber);
    }
    else
    {
        CMomentumReplayGhostEntity *pGhost = dynamic_cast<CMomentumReplayGhostEntity *>(pOther);
        if (pGhost)
        {
            pGhost->m_SrvData.m_RunData.m_bIsInZone = false;
            pGhost->m_SrvData.m_RunData.m_iCurrentZone = pGhost->m_SrvData.m_RunData.m_iOldZone;
            pGhost->m_SrvData.m_RunData.m_iBonusZone = pGhost->m_SrvData.m_RunData.m_iOldBonusZone;

            pStageEvent = gameeventmanager->CreateEvent("zone_enter");
            pStageEvent->SetInt("ent", pGhost->entindex());
            pStageEvent->SetInt("num", m_iZoneNumber);
        }
    }
    if (pStageEvent)
    {
        gameeventmanager->FireEvent(pStageEvent);
    }

    BaseClass::OnEndTouch(pOther);
}

KeyValues *CTriggerTimerStop::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("end");
    kv->SetInt("ZoneNumber", GetZoneNumber());

    return kv;
}

bool CTriggerTimerStop::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "end"))
    {
        return false;
    }

    SetName(MAKE_STRING("End Trigger"));

    SetZoneNumber(kv->GetInt("ZoneNumber"));

    return true;
}

int CTriggerTimerStop::GetZoneType()
{
    return MOMZONETYPE_STOP;
}

//----------------------------------------------------------------------------------------------

//---------- CTriggerCheckpoint ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_checkpoint, CTriggerCheckpoint);

BEGIN_DATADESC(CTriggerCheckpoint)
    DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint"),
    DEFINE_OUTPUT(m_ResetOnehops, "OnResetOnehops")
END_DATADESC()

void CTriggerCheckpoint::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        m_ResetOnehops.FireOutput(pPlayer, this);
        pPlayer->SetCurrentCheckpointTrigger(this);
        pPlayer->RemoveAllOnehops();
    }
}

KeyValues *CTriggerCheckpoint::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("checkpoint");
    kv->SetInt("number", GetCheckpointNumber());

    return kv;
}

bool CTriggerCheckpoint::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "checkpoint"))
    {
        return false;
    }

    SetName(MAKE_STRING("Checkpoint Trigger"));

    SetCheckpointNumber(kv->GetInt("number"));

    return true;
}

int CTriggerCheckpoint::GetZoneType()
{
    return MOMZONETYPE_CP;
}

//----------------------------------------------------------------------------------------------

//------------- CFilterCheckpoint --------------------------------------------------------------
LINK_ENTITY_TO_CLASS(filter_activator_checkpoint, CFilterCheckpoint);

BEGIN_DATADESC(CFilterCheckpoint)
    DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint")
END_DATADESC()

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
    DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop"),
    DEFINE_KEYFIELD(m_bResetAngles, FIELD_BOOLEAN, "resetang"),
END_DATADESC()

void CTriggerTeleportEnt::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);

    // SF_TELE_ONEXIT defaults to 0 so ents that inherit from this class and call this method DO fire the tp logic
    if (pOther && !HasSpawnFlags(SF_TELE_ONEXIT))
    {
        HandleTeleport(pOther);
    }
}

void CTriggerTeleportEnt::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);

    if (pOther && HasSpawnFlags(SF_TELE_ONEXIT))
    {
        HandleTeleport(pOther);
    }
}

void CTriggerTeleportEnt::HandleTeleport(CBaseEntity *pOther)
{
    if (pOther)
    {
        if (!m_pDestinationEnt)
        {
            if (m_target != NULL_STRING)
                m_pDestinationEnt = gEntList.FindEntityByName(nullptr, m_target, nullptr, pOther, pOther);
            else
            {
                DevWarning("CTriggerTeleport cannot teleport, pDestinationEnt and m_target are null!\n");
                return;
            }
        }

        if (!PassesTriggerFilters(pOther))
            return;

        if (m_pDestinationEnt) // ensuring not null
        {
            Vector tmp = m_pDestinationEnt->GetAbsOrigin();
            // make origin adjustments. (origin in center, not at feet)
            tmp.z -= pOther->WorldAlignMins().z;

            pOther->Teleport(&tmp, m_bResetAngles ? &m_pDestinationEnt->GetAbsAngles() : nullptr,
                             m_bResetVelocity ? &vec3_origin : nullptr);
            AfterTeleport();
        }
    }
}

//----------------------------------------------------------------------------------------------

//----------- CTriggerTeleportCheckpoint -------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_teleport_checkpoint, CTriggerTeleportCheckpoint);

void CTriggerTeleportCheckpoint::OnStartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
        SetDestinationEnt(pPlayer->GetCurrentCheckpointTrigger());
    BaseClass::OnStartTouch(pOther);
}

KeyValues *CTriggerTeleportCheckpoint::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("checkpoint_teleport");
    // kv->SetInt("destination", GetDestinationCheckpointNumber());
    kv->SetBool("stop", ShouldStopPlayer());
    kv->SetBool("resetang", ShouldResetAngles());
    kv->SetString("destinationname", m_target.ToCStr());

    return kv;
}

bool CTriggerTeleportCheckpoint::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "checkpoint_teleport"))
    {
        return false;
    }

    SetName(MAKE_STRING("TeleportToCheckpoint Trigger"));

    SetShouldResetAngles(kv->GetBool("stop"));
    SetShouldResetAngles(kv->GetBool("resetang"));
    m_target = MAKE_STRING(kv->GetString("destinationname"));

    return true;
}

int CTriggerTeleportCheckpoint::GetZoneType()
{
    return MOMZONETYPE_CPTELE;
}
//-----------------------------------------------------------------------------------------------

//------------ CTriggerOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_onehop, CTriggerOnehop);

BEGIN_DATADESC(CTriggerOnehop)
    DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold"),
    DEFINE_OUTPUT(m_hopNoLongerJumpable, "OnHopNoLongerJumpable")
END_DATADESC()

CTriggerOnehop::CTriggerOnehop() : m_fOnStartTouchedTime(-1.0), m_fMaxHoldSeconds(1){};

void CTriggerOnehop::OnStartTouch(CBaseEntity *pOther)
{
    // Needed for the Think() function of this class
    CBaseMomentumTrigger::OnStartTouch(pOther);

    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        m_fOnStartTouchedTime = gpGlobals->realtime;
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
    if (pPlayer && m_fOnStartTouchedTime > 0 && IsTouching(pPlayer) &&
        gpGlobals->realtime - m_fOnStartTouchedTime >= m_fMaxHoldSeconds)
    {
        SetDestinationEnt(pPlayer->GetCurrentCheckpointTrigger());
        HandleTeleport(pPlayer);

        if (!m_bhopNoLongerJumpableFired)
        {
            m_hopNoLongerJumpable.FireOutput(pPlayer, this);
            m_bhopNoLongerJumpableFired = true;
        }
    }
}

void CTriggerOnehop::OnEndTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer != nullptr)
    {
        m_hopNoLongerJumpable.FireOutput(pPlayer, this);
        m_bhopNoLongerJumpableFired = true;
    }
}

KeyValues *CTriggerOnehop::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("onehop");
    // kv->SetInt("destination", GetDestinationIndex());
    kv->SetBool("stop", ShouldStopPlayer());
    kv->SetBool("resetang", ShouldResetAngles());
    kv->SetFloat("hold", GetHoldTeleportTime());
    kv->SetString("destinationname", m_target.ToCStr());

    return kv;
}

bool CTriggerOnehop::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "onehop"))
    {
        return false;
    }

    SetName(MAKE_STRING("Onehop Trigger"));

    SetShouldStopPlayer(kv->GetBool("stop"));
    SetShouldResetAngles(kv->GetBool("resetang"));
    SetHoldTeleportTime(kv->GetFloat("hold"));
    m_target = MAKE_STRING(kv->GetString("destinationname"));

    return true;
}

int CTriggerOnehop::GetZoneType()
{
    return MOMZONETYPE_ONEHOP;
}
//-----------------------------------------------------------------------------------------------

//------- CTriggerResetOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_resetonehop, CTriggerResetOnehop);

BEGIN_DATADESC(CTriggerResetOnehop)
    DEFINE_OUTPUT(m_ResetOnehops, "OnResetOnehops")
END_DATADESC()

void CTriggerResetOnehop::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        m_ResetOnehops.FireOutput(pPlayer, this);
        pPlayer->RemoveAllOnehops();
    }
}

KeyValues *CTriggerResetOnehop::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("resetonehop");

    return kv;
}

bool CTriggerResetOnehop::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "resetonehop"))
    {
        return false;
    }

    SetName(MAKE_STRING("ResetOnehop Trigger"));

    return true;
}

int CTriggerResetOnehop::GetZoneType()
{
    return MOMZONETYPE_RESETONEHOP;
}
//-----------------------------------------------------------------------------------------------

//---------- CTriggerMultihop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_multihop, CTriggerMultihop);

BEGIN_DATADESC(CTriggerMultihop)
    DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold")
END_DATADESC()

CTriggerMultihop::CTriggerMultihop() : m_fOnStartTouchedTime(0.0), m_fMaxHoldSeconds(1) {}

void CTriggerMultihop::OnStartTouch(CBaseEntity *pOther)
{
    CBaseMomentumTrigger::OnStartTouch(pOther);
    if (pOther->IsPlayer())
    {
        m_fOnStartTouchedTime = gpGlobals->realtime;
    }
}

void CTriggerMultihop::OnEndTouch(CBaseEntity *pOther)
{
    // We don't want to keep checking for tp
    m_fOnStartTouchedTime = -1.0f;
    CBaseMomentumTrigger::OnEndTouch(pOther);
}

void CTriggerMultihop::Think()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && m_fOnStartTouchedTime > 0 && IsTouching(pPlayer) &&
        gpGlobals->realtime - m_fOnStartTouchedTime >= m_fMaxHoldSeconds)
    {
        SetDestinationEnt(pPlayer->GetCurrentCheckpointTrigger());
        HandleTeleport(pPlayer);
    }
}

KeyValues *CTriggerMultihop::ToKeyValues() const
{
    KeyValues *kv = new KeyValues("multihop");
    // kv->SetInt("destination", GetDestinationIndex());
    kv->SetBool("stop", ShouldStopPlayer());
    kv->SetFloat("hold", GetHoldTeleportTime());
    kv->SetBool("resetang", ShouldResetAngles());
    kv->SetString("destinationname", m_target.ToCStr());

    return kv;
}

bool CTriggerMultihop::LoadFromKeyValues(KeyValues *kv)
{
    if (!FStrEq(kv->GetName(), "multihop"))
    {
        return false;
    }

    SetName(MAKE_STRING("Multihop Trigger"));

    SetShouldStopPlayer(kv->GetBool("stop"));
    SetShouldResetAngles(kv->GetBool("resetang"));
    SetHoldTeleportTime(kv->GetFloat("hold"));
    m_target = MAKE_STRING(kv->GetString("destinationname"));

    return true;
}

int CTriggerMultihop::GetZoneType()
{
    return MOMZONETYPE_MULTIHOP;
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
    case KEY_FORWARD:
        m_ButtonRep = IN_FORWARD;
        break;
    case KEY_BACK:
        m_ButtonRep = IN_BACK;
        break;
    case KEY_MOVELEFT:
        m_ButtonRep = IN_MOVELEFT;
        break;
    case KEY_MOVERIGHT:
        m_ButtonRep = IN_MOVERIGHT;
        break;
    case KEY_JUMP:
        m_ButtonRep = IN_JUMP;
        break;
    case KEY_DUCK:
        m_ButtonRep = IN_DUCK;
        break;
    case KEY_ATTACK:
        m_ButtonRep = IN_ATTACK;
        break;
    case KEY_ATTACK2:
        m_ButtonRep = IN_ATTACK2;
        break;
    case KEY_RELOAD:
        m_ButtonRep = IN_RELOAD;
        break;
    default:
        DevWarning("Passed unhandled key press");
        m_ButtonRep = 0;
        break;
    }

    BaseClass::Spawn();
}
//-----------------------------------------------------------------------------------------------

//--------- CTriggerLimitMovement -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_limitmovement, CTriggerLimitMovement);

void CTriggerLimitMovement::OnStartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        ToggleButtons(pPlayer, false);
    }
    else
    {
        CMomentumGhostBaseEntity *pGhostEnt = dynamic_cast<CMomentumGhostBaseEntity*>(pOther);
        if (pGhostEnt)
            ToggleButtons(pGhostEnt, false);
    }

    BaseClass::OnStartTouch(pOther);
}

void CTriggerLimitMovement::OnEndTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        ToggleButtons(pPlayer, true);
    }
    else
    {
        CMomentumGhostBaseEntity *pGhostEnt = dynamic_cast<CMomentumGhostBaseEntity*>(pOther);
        if (pGhostEnt)
            ToggleButtons(pGhostEnt, true);
    }

    BaseClass::OnEndTouch(pOther);
}

template <class T>
void CTriggerLimitMovement::ToggleButtons(T* pEnt, bool bEnable)
{
    if (m_spawnflags & SF_LIMIT_FORWARD)
        bEnable ? pEnt->EnableButtons(IN_FORWARD) : pEnt->DisableButtons(IN_FORWARD);
    if (m_spawnflags & SF_LIMIT_LEFT)
        bEnable ? pEnt->EnableButtons(IN_MOVELEFT) : pEnt->DisableButtons(IN_MOVELEFT);
    if (m_spawnflags & SF_LIMIT_RIGHT)
        bEnable ? pEnt->EnableButtons(IN_MOVERIGHT) : pEnt->DisableButtons(IN_MOVERIGHT);
    if (m_spawnflags & SF_LIMIT_BACK)
        bEnable ? pEnt->EnableButtons(IN_BACK) : pEnt->DisableButtons(IN_BACK);
    if (m_spawnflags & SF_LIMIT_JUMP)
        bEnable ? pEnt->EnableButtons(IN_JUMP) : pEnt->DisableButtons(IN_JUMP);
    if (m_spawnflags & SF_LIMIT_CROUCH)
        bEnable ? pEnt->EnableButtons(IN_DUCK) : pEnt->DisableButtons(IN_DUCK);
    if (m_spawnflags & SF_LIMIT_BHOP)
        pEnt->SetDisableBhop(!bEnable);
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
    DEFINE_KEYFIELD(m_iIncrease, FIELD_INTEGER, "increase")
END_DATADESC()

CTriggerMomentumPush::CTriggerMomentumPush(){};

void CTriggerMomentumPush::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);
    if (pOther && HasSpawnFlags(SF_PUSH_ONSTART) && pOther->IsPlayer())
        OnSuccessfulTouch(pOther);
}

void CTriggerMomentumPush::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);
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
    DEFINE_KEYFIELD(m_bStuckOnGround, FIELD_BOOLEAN, "StuckOnGround"),
    DEFINE_KEYFIELD(m_bAllowingJump, FIELD_BOOLEAN, "AllowingJump"),
    DEFINE_KEYFIELD(m_bDisableGravity, FIELD_BOOLEAN, "DisableGravity"),
    DEFINE_KEYFIELD(m_bFixUpsideSlope, FIELD_BOOLEAN, "FixUpsideSlope"),
    //DEFINE_KEYFIELD(m_flSlideGravity, FIELD_FLOAT, "GravityValue")
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CTriggerSlide, DT_TriggerSlide)
SendPropBool(SENDINFO(m_bStuckOnGround)), SendPropBool(SENDINFO(m_bAllowingJump)),
    SendPropBool(SENDINFO(m_bDisableGravity)), SendPropBool(SENDINFO(m_bFixUpsideSlope)), END_SEND_TABLE();

void CTriggerSlide::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);

    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(pOther);

    if (pPlayer != nullptr)
    {
        pPlayer->m_vecSlideTriggers.AddToHead(this);
        pPlayer->m_CurrentSlideTrigger = this;
    }
}

void CTriggerSlide::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);

    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(pOther);

    if (pPlayer != nullptr)
    {
        pPlayer->m_vecSlideTriggers.FindAndRemove(this);

        if (pPlayer->m_CurrentSlideTrigger.Get() == this)
        {
            if (!pPlayer->m_vecSlideTriggers.IsEmpty())
                pPlayer->m_CurrentSlideTrigger = pPlayer->m_vecSlideTriggers[0];
            else
                pPlayer->m_CurrentSlideTrigger = nullptr;
        }
    }
}

//-----------------------------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(trigger_momentum_reversespeed, CTriggerReverseSpeed);

BEGIN_DATADESC(CTriggerReverseSpeed)
    DEFINE_KEYFIELD(m_bReverseHorizontalSpeed, FIELD_BOOLEAN, "ReverseHorizontal"),
    DEFINE_KEYFIELD(m_bReverseVerticalSpeed, FIELD_BOOLEAN, "ReverseVertical"),
    DEFINE_KEYFIELD(m_flInterval, FIELD_FLOAT, "Interval"),
    DEFINE_KEYFIELD(m_bOnThink, FIELD_BOOLEAN, "OnThink")
END_DATADESC()

void CTriggerReverseSpeed::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);

    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(pOther);

    if (pPlayer != nullptr)
    {
        // Reverse x/y velocity.
        if (m_bReverseHorizontalSpeed)
        {
            Vector vecVelocity = pPlayer->GetAbsVelocity();
            float zVelBackup = vecVelocity.z;
            vecVelocity.z = 0.0f;

            float flSpeedAmount = vecVelocity.Length2D();

            // We need to compute its direction now to reverse the speed properly.
            QAngle qDirVelocity;
            VectorNormalizeFast(vecVelocity);
            VectorAngles(vecVelocity, qDirVelocity);

            // Revert the direction
            qDirVelocity.y = AngleNormalize(qDirVelocity.y - 180.0f);

            // Apply the speed.
            Vector vecNewVelocity;
            AngleVectors(qDirVelocity, &vecNewVelocity);
            vecNewVelocity.x *= flSpeedAmount;
            vecNewVelocity.y *= flSpeedAmount;
            vecNewVelocity.z = zVelBackup;

            pPlayer->SetAbsVelocity(vecNewVelocity);
        }

        // Reverse z velocity.
        if (m_bReverseVerticalSpeed)
        {
            Vector vecVelocity = pPlayer->GetAbsVelocity();
            vecVelocity.z = -vecVelocity.z;
            pPlayer->SetAbsVelocity(vecVelocity);
        }

        vecCalculatedVel = pPlayer->GetAbsVelocity();
    }

    if (m_bOnThink)
    {
        SetNextThink(gpGlobals->curtime + m_flInterval);
    }
    else
    {
        SetNextThink(TICK_NEVER_THINK);
    }

    m_bShouldThink = true;
}

void CTriggerReverseSpeed::Think()
{
    BaseClass::Think();

    if (m_bOnThink)
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

        if (pPlayer != nullptr && m_bShouldThink)
        {
            // Shall we will use the already calculated vel here, if we recalculate we could be stuck into a trigger
            // since it will take the new velocity already Reversed? If the interval is high enough it shouldn't matter.
            // pPlayer->SetAbsVelocity(vecCalculatedVel);

            // Reverse x/y velocity.
            if (m_bReverseHorizontalSpeed)
            {
                Vector vecVelocity = pPlayer->GetAbsVelocity();
                float zVelBackup = vecVelocity.z;
                vecVelocity.z = 0.0f;

                float flSpeedAmount = vecVelocity.Length2D();

                // We need to compute its direction now to reverse the speed properly.
                QAngle qDirVelocity;
                VectorNormalizeFast(vecVelocity);
                VectorAngles(vecVelocity, qDirVelocity);

                // Revert the direction
                qDirVelocity.y = AngleNormalize(qDirVelocity.y - 180.0f);

                // Apply the speed.
                Vector vecNewVelocity;
                AngleVectors(qDirVelocity, &vecNewVelocity);
                vecNewVelocity.x *= flSpeedAmount;
                vecNewVelocity.y *= flSpeedAmount;
                vecNewVelocity.z = zVelBackup;

                pPlayer->SetAbsVelocity(vecNewVelocity);
            }

            // Reverse z velocity.
            if (m_bReverseVerticalSpeed)
            {
                Vector vecVelocity = pPlayer->GetAbsVelocity();
                vecVelocity.z = -vecVelocity.z;
                pPlayer->SetAbsVelocity(vecVelocity);
            }
        }

        SetNextThink(gpGlobals->curtime + m_flInterval);
    }
    else
    {
        SetNextThink(TICK_NEVER_THINK);
    }
}

void CTriggerReverseSpeed::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);

    m_bShouldThink = false;
}

//-----------------------------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(trigger_momentum_setspeed, CTriggerSetSpeed);

BEGIN_DATADESC(CTriggerSetSpeed)
    DEFINE_KEYFIELD(m_bKeepHorizontalSpeed, FIELD_BOOLEAN, "KeepHorizontalSpeed"),
    DEFINE_KEYFIELD(m_bKeepVerticalSpeed, FIELD_BOOLEAN, "KeepVerticalSpeed"),
    DEFINE_KEYFIELD(m_flHorizontalSpeedAmount, FIELD_FLOAT, "HorizontalSpeedAmount"),
    DEFINE_KEYFIELD(m_flVerticalSpeedAmount, FIELD_FLOAT, "VerticalSpeedAmount"),
    DEFINE_KEYFIELD(m_angWishDirection, FIELD_VECTOR, "Direction"),
    DEFINE_KEYFIELD(m_flInterval, FIELD_FLOAT, "Interval"),
    DEFINE_KEYFIELD(m_bOnThink, FIELD_BOOLEAN, "OnThink")
END_DATADESC()

void CTriggerSetSpeed::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);

    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(pOther);

    if (pPlayer != nullptr)
    {
        // As far I know, you can get the same direction by just playing with x/y ,
        // for getting the same direction as the z angle, except if there is a gimbal lock on the given angle.
        // I didn't look much about it, but it's pretty interesting. Gotta investigate.

        // Compute velocity direction only from y angle. We ignore these because if the mapper set -90 and 180
        // , the results on x/y axis velocity direction will be close to 0
        // and result that the horizontal speed amount won't be set correctly.
        // Since vertical speed can be set manually anyway, we can ignore and zero the x and z axis on the angle.
        m_angWishDirection.x = m_angWishDirection.z = 0.0f;

        Vector vecNewVelocity;
        AngleVectors(m_angWishDirection, &vecNewVelocity);

        Vector vecNewFinalVelocity = pPlayer->GetAbsVelocity();

        // Apply the speed.
        vecNewVelocity.x *= m_flHorizontalSpeedAmount;
        vecNewVelocity.y *= m_flHorizontalSpeedAmount;
        vecNewVelocity.z = m_flVerticalSpeedAmount;

        if (!m_bKeepVerticalSpeed)
            vecNewFinalVelocity.z = vecNewVelocity.z;

        if (!m_bKeepHorizontalSpeed)
        {
            vecNewFinalVelocity.x = vecNewVelocity.x;
            vecNewFinalVelocity.y = vecNewVelocity.y;
        }

        pPlayer->SetAbsVelocity(vecNewFinalVelocity);
        vecCalculatedVel = vecNewFinalVelocity;
    }

    if (m_bOnThink)
    {
        SetNextThink(gpGlobals->curtime + m_flInterval);
    }
    else
    {
        SetNextThink(TICK_NEVER_THINK);
    }

    m_bShouldThink = true;
}

void CTriggerSetSpeed::Think()
{
    BaseClass::Think();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    if (pPlayer != nullptr && m_bShouldThink)
    {
        pPlayer->SetAbsVelocity(vecCalculatedVel);
    }

    if (m_bOnThink)
    {
        SetNextThink(gpGlobals->curtime + m_flInterval);
    }
    else
    {
        SetNextThink(TICK_NEVER_THINK);
    }
}

void CTriggerSetSpeed::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);
    m_bShouldThink = false;
}

//-----------------------------------------------------------------------------------------------

void CTriggerSpeedThreshold::OnStartTouch(CBaseEntity *pOther) { CheckSpeed(ToCMOMPlayer(pOther)); }

void CTriggerSpeedThreshold::CheckSpeed(CMomentumPlayer *pPlayer)
{
    if (pPlayer != nullptr)
    {
        Vector Velocity = pPlayer->GetAbsVelocity();

        if (m_bHorizontal)
        {
            float zAbs = abs(Velocity.z);

            if (m_iAboveOrBelow == TRIGGERSPEEDTHRESHOLD_ABOVE)
            {
                if (zAbs > m_flHorizontalSpeed)
                {
                    m_OnThresholdEvent.FireOutput(pPlayer, this);
                }
            }
            else
            {
                if (zAbs < m_flHorizontalSpeed)
                {
                    m_OnThresholdEvent.FireOutput(pPlayer, this);
                }
            }
        }

        if (m_bVertical)
        {
            float Vel2D = Velocity.Length2D();

            if (m_iAboveOrBelow == TRIGGERSPEEDTHRESHOLD_ABOVE)
            {
                if (Vel2D > m_flVerticalSpeed)
                {
                    m_OnThresholdEvent.FireOutput(pPlayer, this);
                }
            }
            else
            {
                if (Vel2D < m_flVerticalSpeed)
                {
                    m_OnThresholdEvent.FireOutput(pPlayer, this);
                }
            }
        }
    }

    if (m_bOnThink)
    {
        SetNextThink(gpGlobals->curtime + m_flInterval);
    }
    else
    {
        SetNextThink(TICK_NEVER_THINK);
    }

    m_bShouldThink = true;
}

void CTriggerSpeedThreshold::Think()
{
    BaseClass::Think();

    if (m_bShouldThink)
        CheckSpeed(ToCMOMPlayer(UTIL_GetLocalPlayer()));
}

void CTriggerSpeedThreshold::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);
    m_bShouldThink = false;
}

LINK_ENTITY_TO_CLASS(trigger_momentum_speedthreshold, CTriggerSpeedThreshold);

BEGIN_DATADESC(CTriggerSpeedThreshold)
    DEFINE_KEYFIELD(m_iAboveOrBelow, FIELD_INTEGER, "AboveOrBelow"),
    DEFINE_KEYFIELD(m_bVertical, FIELD_BOOLEAN, "Vertical"),
    DEFINE_KEYFIELD(m_bHorizontal, FIELD_BOOLEAN, "Horizontal"),
    DEFINE_KEYFIELD(m_flVerticalSpeed, FIELD_FLOAT, "VerticalSpeed"),
    DEFINE_KEYFIELD(m_flHorizontalSpeed, FIELD_FLOAT, "HorizontalSpeed"),
    DEFINE_KEYFIELD(m_flInterval, FIELD_FLOAT, "Interval"), 
    DEFINE_KEYFIELD(m_flInterval, FIELD_FLOAT, "Interval"),
    DEFINE_KEYFIELD(m_bOnThink, FIELD_BOOLEAN, "OnThink"),
    DEFINE_OUTPUT(m_OnThresholdEvent, "OnThreshold") 
END_DATADESC()

LINK_ENTITY_TO_CLASS(func_momentum_brush, CFuncMomentumBrush);

BEGIN_DATADESC(CFuncMomentumBrush)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage"), 
    DEFINE_KEYFIELD(m_iDisabledAlpha, FIELD_CHARACTER, "DisabledAlpha"),
    DEFINE_KEYFIELD(m_bInverted, FIELD_BOOLEAN, "Invert"),
    DEFINE_KEYFIELD(m_bDisableUI, FIELD_BOOLEAN, "DisableUI"),
END_DATADESC()

CFuncMomentumBrush::CFuncMomentumBrush()
{
    m_iWorld = -1;
    m_iStage = 0;
    m_iDisabledAlpha = 102;
    m_bInverted = false;
    m_bDisableUI = false;
}

void CFuncMomentumBrush::Spawn()
{
    // On spawn, we need to check if this brush should be enabled

    SetMoveType(MOVETYPE_PUSH); // so it doesn't get pushed by anything
    SetRenderMode(kRenderTransAlpha); // Allows alpha override

    SetSolid(SOLID_BSP); // Seems to have the best collision for standing/jumping (see the bhop block fix system)
    AddEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);
    AddSolidFlags(FSOLID_TRIGGER); // Allow us to touch the player

    SetModel(STRING(GetModelName()));

    if (g_pMomentumProgress->IsStageBeat(m_iStage, m_iWorld))
        TurnOn();
    else
        TurnOff();

    // If it can't move/go away, it's really part of the world
    if (!GetEntityName() || !m_iParent)
        AddFlag(FL_WORLDBRUSH);
    
    CreateVPhysics();
}

bool CFuncMomentumBrush::IsOn() const
{
    return (m_bInverted ? m_iDisabled != 0 : m_iDisabled == 0);
}

void CFuncMomentumBrush::TurnOn()
{
    // Turning this brush "on" means making it 100% solid and visible
    if (IsOn())
        return;

    // MOM_TODO: The below is probably needed later on, after mappers make the map
    /*if (!g_pMomentumProgress->ShouldEnableBrush(m_iStageEnable))
        return;*/

    if (m_bInverted)
       AddSolidFlags(FSOLID_NOT_SOLID);
    else
       RemoveSolidFlags(FSOLID_NOT_SOLID);

    SetRenderColorA(m_bInverted ? m_iDisabledAlpha : 255);

    m_iDisabled = m_bInverted;
}

void CFuncMomentumBrush::TurnOff()
{
    // Turning this brush "off" means making it non-solid, and only translucent
    if (!IsOn())
        return;

    if (m_bInverted)
        RemoveSolidFlags(FSOLID_NOT_SOLID);    
    else
        AddSolidFlags(FSOLID_NOT_SOLID);

    SetRenderColorA(m_bInverted ? 255 : m_iDisabledAlpha);

    m_iDisabled = !m_bInverted;
}

void CFuncMomentumBrush::StartTouch(CBaseEntity* pOther)
{
    BaseClass::StartTouch(pOther);
    // MOM_TODO: Show a UI that says which stage needs unlocking
    if (!m_bDisableUI)
    {
        if (pOther->IsPlayer())
        {
            if (m_iDisabled)
                ClientPrint((CBasePlayer*)pOther, HUD_PRINTCENTER, 
                            CFmtStr("Beat Stage %i To Make This Solid!", m_iStage).Get());
        }
    }
}

void CFuncMomentumBrush::EndTouch(CBaseEntity* pOther)
{
    BaseClass::EndTouch(pOther);
    // MOM_TODO: Hide the UI
    // if (m_iDisabled && pOther->IsPlayer()) or something
}


LINK_ENTITY_TO_CLASS(filter_momentum_progress, CFilterMomentumProgress);
BEGIN_DATADESC(CFilterMomentumProgress)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage")
END_DATADESC()

CFilterMomentumProgress::CFilterMomentumProgress()
{
    m_iWorld = -1;
    m_iStage = 0;
}

bool CFilterMomentumProgress::PassesFilterImpl(CBaseEntity* pCaller, CBaseEntity* pEntity)
{
    // So far the only entity that is a player is the local player
    if (pEntity->IsPlayer())
    {
        return g_pMomentumProgress->IsStageBeat(m_iStage, m_iWorld);
    }
    return false;
}


LINK_ENTITY_TO_CLASS(trigger_momentum_campaign_changelevel, CTriggerCampaignChangelevel);

BEGIN_DATADESC(CTriggerCampaignChangelevel)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage"),
    DEFINE_KEYFIELD(m_iGametype, FIELD_INTEGER, "gametype"),
    DEFINE_KEYFIELD(m_MapOverride, FIELD_STRING, "map_name_override")
END_DATADESC()

CTriggerCampaignChangelevel::CTriggerCampaignChangelevel()
{
    m_iWorld = -1;
    m_iStage = 0;
    m_iGametype = 0;
}

void CTriggerCampaignChangelevel::OnStartTouch(CBaseEntity* pOther)
{
    BaseClass::OnStartTouch(pOther);

    if (pOther->IsPlayer())
    {
        if (!m_MapOverride)
        {
            if (m_iWorld == -1)
            {
                // Go back to the Hub
                engine->ClientCommand(pOther->edict(), "map mom_hub\n");
            }
            else
            {
                // Otherwise go to a specific world stage

                // Build the string
                const char *pMapPrefix;
                switch (m_iGametype)
                {
                case GAMEMODE_SURF:
                case GAMEMODE_TRICKSURF:
                    pMapPrefix = "surf_";
                    break;
                case GAMEMODE_BHOP:
                    pMapPrefix = "bhop_";
                    break;
                case GAMEMODE_KZ:
                    pMapPrefix = "kz_";
                    break;
                    // MOM_TODO: Add the rest of the gametypes here
                default:
                    pMapPrefix = "";
                    break;
                }

                engine->ClientCommand(pOther->edict(), CFmtStr("map %sw%i_s%i\n", pMapPrefix, m_iWorld, m_iStage).Get());
            }
        }
        else
        {
            // Go to the specific map
            engine->ClientCommand(pOther->edict(), CFmtStr("map %s\n", m_MapOverride.ToCStr()).Get());
        }
    }
}


LINK_ENTITY_TO_CLASS(info_momentum_map, CMomentumMapInfo);

BEGIN_DATADESC(CMomentumMapInfo)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage"),
    DEFINE_KEYFIELD(m_iGametype, FIELD_INTEGER, "gametype"),
    DEFINE_KEYFIELD(m_MapAuthor, FIELD_STRING, "author")
END_DATADESC()

CMomentumMapInfo::CMomentumMapInfo(): m_iWorld(-1), m_iStage(0), m_iGametype(0)
{
}

void CMomentumMapInfo::Spawn()
{
    BaseClass::Spawn();

    KeyValues *pKv = new KeyValues("map_info");
    pKv->SetInt("world", m_iWorld);
    pKv->SetInt("stage", m_iStage);
    pKv->SetInt("gametype", m_iGametype);
    pKv->SetString("author", m_MapAuthor.ToCStr());
    g_pModuleComms->FireEvent(pKv);
    // MOM_TODO: Handle this event in Client (UI) and Timer?
}
