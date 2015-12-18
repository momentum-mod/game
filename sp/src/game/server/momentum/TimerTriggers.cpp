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
	UTIL_GetLocalPlayer()->SetAbsOrigin(g_Timer.GetStartTrigger()->GetAbsOrigin());
	UTIL_GetLocalPlayer()->SetAbsVelocity(Vector(0));
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

void CTriggerTimerStart::StartTouch(CBaseEntity *pOther) {
	if (pOther->IsPlayer())
	{
		if (g_Timer.IsRunning())
		{
			g_Timer.Stop();
			g_Timer.DispatchResetMessage();
		}
		g_Timer.SetCurrentCheckpointTrigger(NULL);
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
		g_Timer.SetCurrentCheckpointTrigger(this);
		g_Timer.ResetLastOnehop();
}

void CTriggerCheckpoint::EndTouch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
		g_Timer.SetCurrentCheckpointTrigger(this);
	BaseClass::EndTouch(pOther);
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
			UTIL_GetLocalPlayer()->SetAbsOrigin(desiredCP->GetAbsOrigin());
			if (m_bResetVelocity)
			{
				UTIL_GetLocalPlayer()->SetAbsVelocity(Vector(0));
			}
		}
	}
}

// Hack? Stops an assertion on BaseThinkFun from appearing
void CTriggerTeleportCheckpoint::Think()
{

}


LINK_ENTITY_TO_CLASS(trigger_timer_onehop, CTriggerOnehop);

BEGIN_DATADESC(CTriggerOnehop)
DEFINE_KEYFIELD(m_iDestinationCheckpointNumber, FIELD_INTEGER, "checkpoint"),
DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop"),
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold"),
END_DATADESC()

// MOM_TODO: Add teleport after delay m_fMaxHoldSeconds has passed
// CTriggerOnehop
void CTriggerOnehop::StartTouch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		if (m_bHoppedIn)
		{ 
			// (-2): Go to TriggerStart
			// (-1): Last activated checkpoint
			// default: Desired checkpoint with this index
			switch (m_iDestinationCheckpointNumber)
			{
			case (-2) :
			{
				CTriggerTimerStart *desiredCP = g_Timer.GetStartTrigger();
				if (desiredCP != NULL)
					pOther->SetAbsOrigin(desiredCP->GetAbsOrigin());
				break;
			}
			case (-1) :
			{
				CTriggerCheckpoint *desiredCP = g_Timer.GetCurrentCheckpoint();
				if (desiredCP != NULL)
					pOther->SetAbsOrigin(desiredCP->GetAbsOrigin());
				break;
			}
			default:
			{
				CTriggerCheckpoint *desiredCP = g_Timer.GetCheckpointAt(m_iDestinationCheckpointNumber);
				if (desiredCP != NULL)
					pOther->SetAbsOrigin(desiredCP->GetAbsOrigin());
			}
			}
			if (m_bResetVelocity)
				pOther->SetAbsVelocity(Vector(0));
		}
		else
		{
			g_Timer.SetLastOnehop(this);
			m_bHoppedIn = true;
		}
	}
}


LINK_ENTITY_TO_CLASS(trigger_timer_resetonehop, CTriggerResetOnehop);

// CTriggerResetOnehop
void CTriggerResetOnehop::StartTouch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
		g_Timer.ResetLastOnehop();
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
