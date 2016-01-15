#include "cbase.h"
#include "Timer.h"
#include "TimerTriggers.h"
#include "movevars_shared.h"
#include "../shared/in_buttons.h"

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
    }
}

//---------- CTriggerTimerStart ------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, CTriggerTimerStart);

BEGIN_DATADESC(CTriggerTimerStart)
DEFINE_KEYFIELD(m_fMaxLeaveSpeed, FIELD_FLOAT, "leavespeed"),
DEFINE_KEYFIELD(m_angLook, FIELD_VECTOR, "lookangles")
END_DATADESC()

void CTriggerTimerStart::EndTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer())
    {
        g_Timer.Start(gpGlobals->tickcount);
        g_Timer.SetStartTrigger(this);
        if (IsLimitingSpeed())
        {
            Vector velocity = pOther->GetAbsVelocity();
            if (IsLimitingSpeedOnlyXY())
            {
                Vector2D vel2D = velocity.AsVector2D();
                if (velocity.AsVector2D().IsLengthGreaterThan(m_fMaxLeaveSpeed))
                {
                    // Isn't it nice how Vector2D.h doesn't have Normalize() on it?
                    // It only has a NormalizeInPlace... Not simple enough for me
                    vel2D = ((vel2D / vel2D.Length()) * m_fMaxLeaveSpeed);
                    pOther->SetAbsVelocity(Vector(vel2D.x, vel2D.y, velocity.z));
                }
            }
            else
            {
                if (velocity.IsLengthGreaterThan(m_fMaxLeaveSpeed))
                {
                    pOther->SetAbsVelocity(velocity.Normalized() * m_fMaxLeaveSpeed);
                }
            }
        }
    }
    BaseClass::EndTouch(pOther);
}

void CTriggerTimerStart::StartTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer() && g_Timer.IsRunning())
    {
        g_Timer.Stop(false);
        g_Timer.DispatchResetMessage();
    }
    BaseClass::StartTouch(pOther);
}

void CTriggerTimerStart::Spawn()
{
    BaseClass::Spawn();
    // We don't want negative velocities (We're checking against an absolute value)
    if (m_fMaxLeaveSpeed < 0)
        m_fMaxLeaveSpeed *= (-1);

    m_angLook.z = 0.0f; // Reset roll since mappers will never stop ruining everything.
}

void CTriggerTimerStart::SetMaxLeaveSpeed(float pMaxLeaveSpeed)
{
    if (pMaxLeaveSpeed < 0)
        pMaxLeaveSpeed *= (-1.0f);
    m_fMaxLeaveSpeed = pMaxLeaveSpeed;
}

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

void CTriggerTimerStart::SetLookAngles(QAngle newang)
{
    m_angLook = newang;
}
//----------------------------------------------------------------------------------------------

//----------- CTriggerTimerStop ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, CTriggerTimerStop);

void CTriggerTimerStop::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    // If timer is already stopped, there's nothing to stop (No run state effect to play)
    if (pOther->IsPlayer() && g_Timer.IsRunning())
        g_Timer.Stop(true);
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


// ------------- CFilterCheckpoint -------------------------------------------------------------
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

        if (!PassesTriggerFilters(pOther)) return;

        if (pDestinationEnt)//ensuring not null
        {
            Vector tmp = pDestinationEnt->GetAbsOrigin();
            // make origin adjustments. (origin in center, not at feet)
            tmp.z -= pOther->WorldAlignMins().z;

            pOther->Teleport(&tmp, m_bResetAngles ? &pDestinationEnt->GetAbsAngles() : NULL, m_bResetVelocity ? &vec3_origin : NULL);
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
    //The above is needed for the Think() function of this class,
    //it's very HACKHACK but it works

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

void CTriggerMultihop::EndTouch(CBaseEntity* pOther)
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

//////-------------------------------------------------------------------------------------------
// Test Functions
// These are now deprecated
//////
//
//static void TestCreateTriggerStart(const CCommand &args)
//{
//    CTriggerTimerStart *pTrigger = (CTriggerTimerStart *) CreateEntityByName("trigger_momentum_timer_start");
//    if (pTrigger)
//    {
//        pTrigger->Spawn();
//        pTrigger->SetAbsOrigin(UTIL_GetLocalPlayer()->GetAbsOrigin());
//        if (args.ArgC() >= 3) // At least 3 Args?
//            pTrigger->SetSize(Vector(-Q_atoi(args.Arg(1)), -Q_atoi(args.Arg(2)), -Q_atoi(args.Arg(3))), Vector(Q_atoi(args.Arg(1)), Q_atoi(args.Arg(2)), Q_atoi(args.Arg(3))));
//        else
//            pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
//        if (args.ArgC() >= 4)
//        {
//            pTrigger->SetIsLimitingSpeed(true);
//            pTrigger->SetMaxLeaveSpeed(Q_atoi(args.Arg(4)));
//        }
//        else
//            pTrigger->SetIsLimitingSpeed(false);
//        pTrigger->SetSolid(SOLID_BBOX);
//        pTrigger->AddEffects(0x020);
//        pTrigger->SetName(MAKE_STRING("Start Trigger"));
//
//        // now use mom_reset_to_start
//    }
//}
//
//static void TestCreateTriggerStop(void)
//{
//    CTriggerTimerStop *pTrigger = (CTriggerTimerStop *) CreateEntityByName("trigger_momentum_timer_stop");
//    if (pTrigger)
//    {
//        pTrigger->Spawn();
//        pTrigger->SetAbsOrigin(UTIL_GetLocalPlayer()->GetAbsOrigin());
//        pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
//        pTrigger->SetSolid(SOLID_BBOX);
//        pTrigger->SetName(MAKE_STRING("Stop Trigger"));
//        // now use mom_reset_to_start
//    }
//}
//
//static void TestCreateTriggerCheckpoint(const CCommand &args)
//{
//    CTriggerStage *pTrigger = (CTriggerStage *) CreateEntityByName("trigger_momentum_timer_stage");
//    if (pTrigger)
//    {
//        pTrigger->Spawn();
//        pTrigger->SetAbsOrigin(UTIL_GetLocalPlayer()->GetAbsOrigin());
//        if (args.ArgC() >= 3) // At least 3 Args?
//            pTrigger->SetSize(Vector(-Q_atoi(args.Arg(1)), -Q_atoi(args.Arg(2)), -Q_atoi(args.Arg(3))), Vector(Q_atoi(args.Arg(1)), Q_atoi(args.Arg(2)), Q_atoi(args.Arg(3))));
//        else
//            pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
//        pTrigger->SetSolid(SOLID_BBOX);
//        pTrigger->SetName(MAKE_STRING("Stage Trigger"));
//        if (args.ArgC() >= 4) // At last 4 Args?
//            pTrigger->SetStageNumber(Q_atoi(args.Arg(4)));
//        else
//            pTrigger->SetStageNumber(g_Timer.GetStageCount() + 1);
//        g_Timer.RequestStageCount();
//        // now use mom_reset_to_start
//    }
//}
//
//static ConCommand mom_createstart("mom_createstart", TestCreateTriggerStart, "Create StartTrigger test\nUsage: mom_createstart <SizeX> <SizeY> <SizeZ> [<MaxLeaveSpeed>]\n");
//static ConCommand mom_createstop("mom_createstop", TestCreateTriggerStop, "Create StopTrigger test");
//static ConCommand mom_createcheckpoint("mom_createstage", TestCreateTriggerCheckpoint, "Create Stage test\nUsage: mom_createstage [<SizeX> <SizeY> <SizeZ>]\n");