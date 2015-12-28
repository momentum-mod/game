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

// CTriggerTimerStart
void CTriggerTimerStart::EndTouch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		g_Timer.Start(gpGlobals->tickcount);
		g_Timer.SetStartTrigger(this);
		if (GetIsLimitingSpeed() && pOther != NULL)
		{
			Vector velocity = pOther->GetAbsVelocity();
			if (velocity.IsLengthGreaterThan(m_fMaxLeaveSpeed))
			{
				pOther->SetAbsVelocity(velocity.Normalized() * m_fMaxLeaveSpeed);
			}
		}
	}
	BaseClass::EndTouch(pOther);
}

void CTriggerTimerStart::StartTouch(CBaseEntity *pOther) 
{
	if (pOther->IsPlayer())
	{
		if (g_Timer.IsRunning())
		{
			g_Timer.Stop(false);
			g_Timer.DispatchResetMessage();
		}
		//g_Timer.SetCurrentCheckpointTrigger(NULL);
	}
	BaseClass::StartTouch(pOther);
}

void CTriggerTimerStart::Spawn()
{
	BaseClass::Spawn();
	// We don't want negative velocities (We're checking against an absolute value)
	if (m_fMaxLeaveSpeed < 0)
		m_fMaxLeaveSpeed *= (-1);
}

void CTriggerTimerStart::SetMaxLeaveSpeed(float pMaxLeaveSpeed)
{
	if (pMaxLeaveSpeed < 0)
		pMaxLeaveSpeed *= (-1);
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
	else {
		if (HasSpawnFlags(SF_LIMIT_LEAVE_SPEED))
		{
			RemoveSpawnFlags(SF_LIMIT_LEAVE_SPEED);
		}
	}
}

LINK_ENTITY_TO_CLASS(trigger_timer_start, CTriggerTimerStart);

BEGIN_DATADESC(CTriggerTimerStart)
DEFINE_KEYFIELD(m_fMaxLeaveSpeed, FIELD_FLOAT, "leavespeed")
END_DATADESC()


// CTriggerTimerStop
void CTriggerTimerStop::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch(pOther);
	// If timer is already stopped, there's nothing to stop (No run state effect to play)
	if (pOther->IsPlayer() && g_Timer.IsRunning())
		g_Timer.Stop(true);
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
		g_Timer.DispatchCheckpointMessage();
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
DEFINE_KEYFIELD(m_iCheckpointNumber, FIELD_INTEGER, "checkpoint"),
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
		CBaseMomentumTrigger *desiredCP;
		if (!m_bUsingLinked)
		{
			if (m_iCheckpointNumber == (-1))
			{
				// Get the current checkpoint
				desiredCP = g_Timer.GetCurrentCheckpoint();
			}
			else // If we're here it means we couldn't find that checkpoint onSpawn
				// So we try again, see if there is better luck now
			{
				// Search for the checkpoint with that index
				desiredCP = g_Timer.GetCheckpointAt(m_iCheckpointNumber);
				if (desiredCP != NULL)
				{
					// If we've found it this time, save it so we can use it later if needed
					m_eLinkedTrigger = desiredCP;
					// Use linked one instean next time:
					m_bUsingLinked = true;
				}// If it's not found this time, then it doesn't exist
			}
		}
		else
			desiredCP = m_eLinkedTrigger;
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

void CTriggerTeleportCheckpoint::SetDestinationCheckpointName(string_t pNewName)
{
	m_sLinkedTriggerName = pNewName;
}

void CTriggerTeleportCheckpoint::SetShouldStopPlayer(bool pShouldStop)
{
	m_bResetVelocity = pShouldStop;
}

void CTriggerTeleportCheckpoint::Spawn()
{
	BaseClass::Spawn();
	if (m_sLinkedTriggerName != NULL_STRING)
	{
		m_eLinkedTrigger = (CBaseMomentumTrigger *)gEntList.FindEntityByName(NULL, m_sLinkedTriggerName);
		if (m_eLinkedTrigger != NULL)
		{
			m_bUsingLinked = true;
		}
	}
	else if (m_iCheckpointNumber >= 0)
	{
		m_eLinkedTrigger = g_Timer.GetCheckpointAt(m_iCheckpointNumber);
		if (m_eLinkedTrigger != NULL)
		{
			m_bUsingLinked = true;
		}// If null, maybe we haven't spawned it yet. We'll be searching for it again when the player touches us, so it's not a problem
	}
}

LINK_ENTITY_TO_CLASS(trigger_timer_onehop, CTriggerOnehop);

BEGIN_DATADESC(CTriggerOnehop)
DEFINE_KEYFIELD(m_iDestinationCheckpointNumber, FIELD_INTEGER, "checkpoint"),
DEFINE_KEYFIELD(m_bResetVelocity, FIELD_BOOLEAN, "stop"),
DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold"),
DEFINE_KEYFIELD(m_sLinkedTriggerName,FIELD_STRING,"checkpointname"),
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
    // Default: Desired checkpoint with this index
    CBaseMomentumTrigger* desiredCP;
	if (!m_bUsingLinked)
	{
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
		default:// If we're here it means we couldn't find that checkpoint onSpawn
				// So we try again, see if there is better luck now
		{
			desiredCP = g_Timer.GetCheckpointAt(m_iDestinationCheckpointNumber);
			if (desiredCP != NULL)
			{
				// If we've found it this time, save it so we can use it later if needed
				m_eLinkedTrigger = desiredCP;
				// Use linked one instean next time:
				m_bUsingLinked = true;
			}// If it's not found this time, then it doesn't exist
			break;
		}
		}
	}
	else {
		desiredCP = m_eLinkedTrigger;
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

void CTriggerOnehop::SetDestinationName(string_t pNewName)
{
	m_sLinkedTriggerName = pNewName;
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

void CTriggerOnehop::Spawn()
{
	BaseClass::Spawn();
	if (m_sLinkedTriggerName != NULL_STRING)
	{
		m_eLinkedTrigger = (CBaseMomentumTrigger *)gEntList.FindEntityByName(NULL, m_sLinkedTriggerName);
		if (m_eLinkedTrigger != NULL)
		{
			m_bUsingLinked = true;
		}
	}
	else if (m_iDestinationCheckpointNumber >= 0)
	{
		m_eLinkedTrigger = g_Timer.GetCheckpointAt(m_iDestinationCheckpointNumber);
		if (m_eLinkedTrigger != NULL)
		{
			m_bUsingLinked = true;
		}// If null, maybe we haven't spawned it yet. We'll be searching for it again when the player touches us, so it's not a problem
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
DEFINE_KEYFIELD(m_sLinkedTriggerName, FIELD_STRING, "checkpointname"),
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
	if (!m_bUsingLinked)
	{
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
		default:// If we're here it means we couldn't find that checkpoint onSpawn
				// So we try again, see if there is better luck now
		{
			desiredCP = g_Timer.GetCheckpointAt(m_iDestinationCheckpointNumber);
			if (desiredCP != NULL)
			{
				// If we've found it this time, save it so we can use it later if needed
				m_eLinkedTrigger = desiredCP;
				// Use linked one instean next time:
				m_bUsingLinked = true;
			}// If it's not found this time, then it doesn't exist
			break;
		}
		}
	}
	else {
		desiredCP = m_eLinkedTrigger;
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

void CTriggerMultihop::SetDestinationName(string_t pNewName)
{
	m_sLinkedTriggerName = pNewName;
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

void CTriggerMultihop::Spawn()
{
	BaseClass::Spawn();
	if (m_sLinkedTriggerName != NULL_STRING)
	{
		m_eLinkedTrigger = (CBaseMomentumTrigger *)gEntList.FindEntityByName(NULL, m_sLinkedTriggerName);
		if (m_eLinkedTrigger != NULL)
		{
			m_bUsingLinked = true;
		}
	}
	else if (m_iDestinationCheckpointNumber >= 0)
	{
		m_eLinkedTrigger = g_Timer.GetCheckpointAt(m_iDestinationCheckpointNumber);
		if (m_eLinkedTrigger != NULL)
		{
			m_bUsingLinked = true;
		}// If null, maybe we haven't spawned it yet. We'll be searching for it again when the player touches us, so it's not a problem
	}
}


LINK_ENTITY_TO_CLASS(trigger_userinput, CTriggerUserInput);

BEGIN_DATADESC(CTriggerUserInput)
DEFINE_KEYFIELD(m_eKey,FIELD_INTEGER,"key"),
DEFINE_OUTPUT(KeyPressed, "OnKeyPressed"),
END_DATADESC()

// CTriggerUserInput
void CTriggerUserInput::StartTouch(CBaseEntity *pOther)
{
    BaseClass::StartTouch(pOther);
    if (pOther->IsPlayer())
    {
        m_bPlayerInside = true;
    }
}

void CTriggerUserInput::EndTouch(CBaseEntity* pOther)
{
    BaseClass::EndTouch(pOther);
    if (pOther->IsPlayer())
    {
        m_bPlayerInside = false;
    }
}

void CTriggerUserInput::Think()
{
    if (m_bPlayerInside)
    {
        CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
        if (pPlayer != NULL)
        {
            if (IsTouching(pPlayer))
            {
                if (pPlayer->m_nButtons & m_ButtonRep)
                {
                    KeyPressed();
                }
            }
            else
            {
                // If player is not touching, then he is not inside us
                // He might have teleported without firing triggers
                // This is why we nest this if
                m_bPlayerInside = false;
            }
        }
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
        default:
            DevWarning("Passed unhandled key press");
            break;
    }
    BaseClass::Spawn();
}

void CTriggerUserInput::KeyPressed()
{

}


//////
// Test Functions
//////

// MOM_TODO: Limit who can acces these commands.

static void TestCreateTriggerStart(const CCommand &args)
{
	CTriggerTimerStart *pTrigger = (CTriggerTimerStart *)CreateEntityByName("trigger_timer_start");
	if (pTrigger)
	{
		pTrigger->Spawn();
		pTrigger->SetAbsOrigin(UTIL_GetLocalPlayer()->GetAbsOrigin());
        if (args.ArgC() >= 3) // At least 3 Args?
		    pTrigger->SetSize(Vector(-Q_atoi(args.Arg(1)), -Q_atoi(args.Arg(2)), -Q_atoi(args.Arg(3))), Vector(Q_atoi(args.Arg(1)), Q_atoi(args.Arg(2)), Q_atoi(args.Arg(3))));
        else
            pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
        if (args.ArgC() >= 4)
        {
            pTrigger->SetIsLimitingSpeed(true);
            pTrigger->SetMaxLeaveSpeed(Q_atoi(args.Arg(4)));
        }
		else 
			pTrigger->SetIsLimitingSpeed(false);
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
            if (args.ArgC() >= 3) // At least 3 Args?
                pTrigger->SetSize(Vector(-Q_atoi(args.Arg(1)), -Q_atoi(args.Arg(2)), -Q_atoi(args.Arg(3))), Vector(Q_atoi(args.Arg(1)), Q_atoi(args.Arg(2)), Q_atoi(args.Arg(3))));
            else
                pTrigger->SetSize(Vector(-256, -256, -256), Vector(256, 256, 256));
            pTrigger->SetSolid(SOLID_BBOX);
            pTrigger->SetName(MAKE_STRING("Checkpoint Trigger"));
            pTrigger->SetCheckpointNumber(index);

            // now use mom_reset_to_start
        }
        else
        {
            DevWarning("Can't create a checkpoint without an index.\nPlease see mom_createcheckpoint usage");
        }
	}
}

static ConCommand mom_createstart("mom_createstart", TestCreateTriggerStart, "Create StartTrigger test\nUsage: mom_createstart <SizeX> <SizeY> <SizeZ> [<MaxLeaveSpeed>]\n");
static ConCommand mom_createstop("mom_createstop", TestCreateTriggerStop, "Create StopTrigger test");
static ConCommand mom_createcheckpoint("mom_createcheckpoint", TestCreateTriggerCheckpoint, "Create CheckpointTrigger test\nUsage: mom_createcheckpoint <Index> [<SizeX> <SizeY> <SizeZ>]\n");
