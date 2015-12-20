#include "cbase.h"
#include "Timer.h"
#include "TimerTriggers.h"
#include "movevars_shared.h"

#include "tier0/memdbgon.h"

// CBaseMomentumTrigger
void CBaseMomentumTrigger::Spawn()
{
	BaseClass::Spawn();
	// temporary
	m_debugOverlays |= (OVERLAY_BBOX_BIT | OVERLAY_TEXT_BIT);
}

// MOM_TODO Base for func_resetcheckpoints. Not implemented yet
void CBaseMomentumTrigger::ResetCheckpoints()
{

	g_Timer.SetCurrentCheckpointTrigger(NULL);
	if (g_Timer.GetStartTrigger() != NULL && UTIL_GetLocalPlayer() != NULL)
		UTIL_GetLocalPlayer()->SetAbsOrigin(g_Timer.GetStartTrigger()->GetAbsOrigin());
		UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);
}

// MOM_TODO Limit speed inside Start Trigger

// CTriggerTimerStart
void CTriggerTimerStart::EndTouch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		g_Timer.Start(gpGlobals->tickcount);
		g_Timer.SetStartTrigger(this);
	}
	BaseClass::EndTouch(pOther);
}

void CTriggerTimerStart::StartTouch(CBaseEntity *pOther) 
{
	if (pOther->IsPlayer())
	{
		if (g_Timer.IsRunning())
		{
			g_Timer.Stop();
			g_Timer.DispatchResetMessage();
		}
		//g_Timer.SetCurrentCheckpointTrigger(NULL);
	}
	BaseClass::StartTouch(pOther);
}

LINK_ENTITY_TO_CLASS(trigger_timer_start, CTriggerTimerStart);


// CTriggerTimerStop
void CTriggerTimerStop::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch(pOther);
	// If timer is already stopped, there's nothing to stop (No run state effect to play)
	if (pOther->IsPlayer() && g_Timer.IsRunning())
		g_Timer.Stop();
}

LINK_ENTITY_TO_CLASS(trigger_timer_stop, CTriggerTimerStop);


// CTriggerCheckpoint
void CTriggerCheckpoint::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch(pOther);

    if (pOther->IsPlayer())
    {
        g_Timer.SetCurrentCheckpointTrigger(this);
		g_Timer.RemoveAllOnehopsFromList();
    }
}

int CTriggerCheckpoint::GetCheckpointNumber()
{
	return m_iCheckpointNumber;
}

void CTriggerCheckpoint::SetCheckpointNumber(int newInt)
{
	m_iCheckpointNumber = newInt;
}

LINK_ENTITY_TO_CLASS(trigger_timer_checkpoint, CTriggerCheckpoint);

BEGIN_DATADESC(CTriggerCheckpoint)
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint")
END_DATADESC()


LINK_ENTITY_TO_CLASS(filter_activator_checkpoint, CFilterCheckpoint);

BEGIN_DATADESC(CFilterCheckpoint)
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint")
END_DATADESC()

// CFilterCheckpoint
bool CFilterCheckpoint::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
    return (g_Timer.GetCurrentCheckpoint() &&
        g_Timer.GetCurrentCheckpoint()->GetCheckpointNumber() >= m_iCheckpointNumber);
}


LINK_ENTITY_TO_CLASS(trigger_timer_checkpoint_teleport, CTriggerTeleportCheckpoint);

BEGIN_DATADESC(CTriggerTeleportCheckpoint)
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint"),
DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop")
END_DATADESC()

// CTriggerTeleportCheckpoint
void CTriggerTeleportCheckpoint::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch(pOther);
	if (pOther->IsPlayer())
	{
		CTriggerCheckpoint *desiredCP;
		if (m_iCheckpointNumber == (-1))
		{
			// Get the current checkpoint
			desiredCP = g_Timer.GetCurrentCheckpoint();
		}
		else
		{
			// Search for the checkpoint with that index
			desiredCP = g_Timer.GetCheckpointAt(m_iCheckpointNumber);
		}
		if (desiredCP != NULL)
		{
			pOther->Teleport(&desiredCP->GetAbsOrigin(), NULL, m_bResetVelocity ? &vec3_origin : NULL);
		}
	}
}

void CTriggerTeleportCheckpoint::SetDestinationCheckpointNumber(int pNewNumber)
{
	m_iCheckpointNumber = pNewNumber;
}

void CTriggerTeleportCheckpoint::SetShouldStopPlayer(bool pShouldStop)
{
	m_bResetVelocity = pShouldStop;
}


LINK_ENTITY_TO_CLASS(trigger_timer_onehop, CTriggerOnehop);

BEGIN_DATADESC(CTriggerOnehop)
DEFINE_KEYFIELD(m_iDestinationCheckpointNumber, FIELD_INTEGER, "checkpoint"),
DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop"),
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold"),
END_DATADESC()

// CTriggerOnehop
void CTriggerOnehop::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
	if (pOther->IsPlayer())
	{
        m_fStartTouchedTime = gpGlobals->realtime;
		if (g_Timer.FindOnehopOnList(this) != (-1))
            HandleTeleport(pOther);
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

void CTriggerOnehop::HandleTeleport(CBaseEntity *pOther)
{
    // (-2): Go to TriggerStart
    // (-1): Last activated checkpoint
    // default: Desired checkpoint with this index
    CBaseMomentumTrigger* desiredCP;
    switch (m_iDestinationCheckpointNumber)
    {
    case (-2) :
    {
        desiredCP = g_Timer.GetStartTrigger();
        break;
    }
    case (-1) :
    {
        desiredCP = g_Timer.GetCurrentCheckpoint();
        break;
    }
    default:
    {
        desiredCP = g_Timer.GetCheckpointAt(m_iDestinationCheckpointNumber);
        break;
    }
    }
    if (desiredCP != NULL)
    {
        pOther->Teleport(&desiredCP->GetAbsOrigin(), NULL, m_bResetVelocity ? &vec3_origin : NULL);
        // And we reset the lastOnehop to NULL so the player has a chance and it doesn't auto teleports again   
		g_Timer.RemoveAllOnehopsFromList();
        m_fStartTouchedTime = -1.0f;
    }
}

void CTriggerOnehop::SetDestinationIndex(int pNewIndex)
{
	m_iDestinationCheckpointNumber = pNewIndex;
}

void CTriggerOnehop::SetShouldStopPlayer(bool pShouldStop)
{
	m_bResetVelocity = pShouldStop;
}

void CTriggerOnehop::SetHoldTeleportTime(float pHoldTime)
{
	m_fMaxHoldSeconds = pHoldTime;
}

void CTriggerOnehop::Think()
{
    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer != NULL && m_fStartTouchedTime > 0)
    {
        if (IsTouching(pPlayer) && (gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds))
        {
            HandleTeleport(pPlayer);
        }
    }
}


LINK_ENTITY_TO_CLASS(trigger_timer_resetonehop, CTriggerResetOnehop);

// CTriggerResetOnehop
void CTriggerResetOnehop::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
	if (pOther->IsPlayer())
		g_Timer.RemoveAllOnehopsFromList();
}


LINK_ENTITY_TO_CLASS(trigger_timer_multihop, CTriggerMultihop);

BEGIN_DATADESC(CTriggerMultihop)
DEFINE_KEYFIELD(m_iDestinationCheckpointNumber, FIELD_INTEGER, "checkpoint"),
DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop"),
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold"),
END_DATADESC()

// CTriggerMultihop
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

void CTriggerMultihop::HandleTeleport(CBaseEntity *pOther)
{
	// (-2): Go to TriggerStart
	// (-1): Last activated checkpoint
	// default: Desired checkpoint with this index
	CBaseMomentumTrigger* desiredCP;
	switch (m_iDestinationCheckpointNumber)
	{
	case (-2) :
	{
		desiredCP = g_Timer.GetStartTrigger();
		break;
	}
	case (-1) :
	{
		desiredCP = g_Timer.GetCurrentCheckpoint();
		break;
	}
	default:
	{
		desiredCP = g_Timer.GetCheckpointAt(m_iDestinationCheckpointNumber);
		break;
	}
	}
	if (desiredCP != NULL)
	{
		pOther->Teleport(&desiredCP->GetAbsOrigin(), NULL, m_bResetVelocity ? &vec3_origin : NULL);
		m_fStartTouchedTime = -1.0f;
	}
}

void CTriggerMultihop::SetDestinationIndex(int pNewIndex)
{
	m_iDestinationCheckpointNumber = pNewIndex;
}

void CTriggerMultihop::SetShouldStopPlayer(bool pShouldStop)
{
	m_bResetVelocity = pShouldStop;
}

void CTriggerMultihop::SetHoldTeleportTime(float pHoldTime)
{
	m_fMaxHoldSeconds = pHoldTime;
}

void CTriggerMultihop::Think()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer != NULL && m_fStartTouchedTime > 0)
	{
		if (IsTouching(pPlayer) && (gpGlobals->realtime - m_fStartTouchedTime >= m_fMaxHoldSeconds))
		{
			HandleTeleport(pPlayer);
		}
	}
}


//////
// Test Functions
//////

static void TestCreateTriggerStart(void)
{
	CTriggerTimerStart *pTrigger = (CTriggerTimerStart *)CreateEntityByName("trigger_timer_start");
	if (pTrigger)
	{
		pTrigger->Spawn();
		pTrigger->SetAbsOrigin(UTIL_GetLocalPlayer()->GetAbsOrigin());
		pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
		pTrigger->SetSolid(SOLID_BBOX);
		pTrigger->AddEffects(0x020);
		pTrigger->SetName(MAKE_STRING("Start Trigger"));
		// now use mom_reset_to_start
	}
}

static void TestCreateTriggerStop(void)
{
	CTriggerTimerStop *pTrigger = (CTriggerTimerStop *)CreateEntityByName("trigger_timer_stop");
	if (pTrigger)
	{
		pTrigger->Spawn();
		pTrigger->SetAbsOrigin(UTIL_GetLocalPlayer()->GetAbsOrigin());
		pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
		pTrigger->SetSolid(SOLID_BBOX);
		pTrigger->SetName(MAKE_STRING("Stop Trigger"));
		// now use mom_reset_to_start
	}
}

static void TestCreateTriggerCheckpoint(const CCommand &args)
{
	CTriggerCheckpoint *pTrigger = (CTriggerCheckpoint *)CreateEntityByName("trigger_timer_checkpoint");
	if (pTrigger)
	{
		if (args.ArgC() > 0)
		{
			int index = Q_atoi(args.Arg(1));
			pTrigger->Spawn();
			pTrigger->SetAbsOrigin(UTIL_GetLocalPlayer()->GetAbsOrigin());
			pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
			pTrigger->SetSolid(SOLID_BBOX);
			pTrigger->SetName(MAKE_STRING("Checkpoint Trigger"));
			pTrigger->SetCheckpointNumber(index);
		}
		// now use mom_reset_to_start
	}
}

static ConCommand mom_createstart("mom_createstart", TestCreateTriggerStart, "Create StartTrigger test");
static ConCommand mom_createstop("mom_createstop", TestCreateTriggerStop, "Create StopTrigger test");
static ConCommand mom_createcheckpoint("mom_createcheckpoint", TestCreateTriggerCheckpoint, "Create CheckpointTrigger test");
