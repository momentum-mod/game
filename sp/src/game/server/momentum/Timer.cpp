#include "cbase.h"
#include "Timer.h"
#include "filesystem.h"
#include "TimerTriggers.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

void CTimer::Start(int start)
{
	m_iStartTick = start;
	SetRunning(true);
	DispatchStateMessage();
}

void CTimer::Stop()
{
	SetRunning(false);
	DispatchStateMessage();
}

void CTimer::DispatchResetMessage()
{
	CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
	user.MakeReliable();
	UserMessageBegin(user, "Timer_Reset");
	MessageEnd();
}

void CTimer::DispatchStateMessage()
{
	CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
	user.MakeReliable();
	UserMessageBegin(user, "Timer_State");
	WRITE_BOOL(m_bIsRunning);
	WRITE_LONG(m_iStartTick);
	MessageEnd();
}

bool CTimer::IsRunning()
{
	return m_bIsRunning;
}

void CTimer::SetRunning(bool running)
{
	m_bIsRunning = running;
}

CTriggerTimerStart *CTimer::GetStartTrigger()
{
	return m_pStartTrigger;
}

// Im not sure if this leaks memory or not
CTriggerCheckpoint *CTimer::GetCheckpointAt(int checkpointNumber)
{
	CBaseEntity* pEnt = gEntList.FindEntityByClassname(NULL, "trigger_timer_checkpoint");
	while (pEnt)
	{
		// Just making sure...
		if (pEnt->ClassMatches("trigger_timer_checkpoint"))
		{
			CTriggerCheckpoint* pTrigger = dynamic_cast<CTriggerCheckpoint*>(pEnt);
			if (pTrigger->GetCheckpointNumber() == checkpointNumber)
			{
				return pTrigger;
			}
		}
		pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_timer_checkpoint");
	}
	return NULL;
}

CTriggerCheckpoint *CTimer::GetCurrentCheckpoint()
{
	return m_pCurrentCheckpoint;
}

void CTimer::SetStartTrigger(CTriggerTimerStart *pTrigger)
{
	m_pStartTrigger = pTrigger;
}

void CTimer::SetCurrentCheckpointTrigger(CTriggerCheckpoint *pTrigger)
{
	// Maybe find a better logic for this one?
	// It works, but it's not pretty
	if (m_pCurrentCheckpoint == NULL)
	{
		m_pCurrentCheckpoint = pTrigger;
	}
	else //pointer is not null
	{
		if (pTrigger != NULL)
		{
			// Is this the starting trigger?
			if (pTrigger->GetCheckpointNumber() == 0)
			{
				// Then set it, even if its index is lower than current's one
				m_pCurrentCheckpoint = pTrigger;
			}
			else if (pTrigger->GetCheckpointNumber() > m_pCurrentCheckpoint->GetCheckpointNumber())
			{
				m_pCurrentCheckpoint = pTrigger;
			}
		}
		else {
			m_pCurrentCheckpoint = pTrigger;
		}
	}
}

void CTimer::CreateCheckpoint(CBasePlayer *pPlayer)
{
	if (!pPlayer) return;
	Checkpoint c;
	c.ang = pPlayer->GetAbsAngles();
	c.pos = pPlayer->GetAbsOrigin();
	c.vel = pPlayer->GetAbsVelocity();
	checkpoints.AddToTail(c);
	// MOM_TODO: Create a decal to show where the checkpoint is?
	m_iCurrentStepCP++;
}

void CTimer::RemoveLastCheckpoint()
{
	if (checkpoints.IsEmpty()) return;
	checkpoints.Remove(m_iCurrentStepCP);
	m_iCurrentStepCP--;//If there's one element left, we still need to decrease currentStep to -1
}

void CTimer::TeleportToCP(CBasePlayer* cPlayer, int cpNum)
{
	if (checkpoints.IsEmpty() || !cPlayer) return;
	Checkpoint c = checkpoints[cpNum];
	cPlayer->Teleport(&c.pos, &c.ang, &c.vel);
}


// CTriggerOnehop
int CTimer::AddOnehopToListTail(CTriggerOnehop *pTrigger)
{
	return onehops.AddToTail(pTrigger);
}

// CTriggerOnehop
bool CTimer::RemoveOnehopFromList(CTriggerOnehop *pTrigger)
{
	return onehops.FindAndRemove(pTrigger);
}

// CTriggerOnehop
int CTimer::FindOnehopOnList(CTriggerOnehop *pTrigger)
{
	return onehops.Find(pTrigger);
}

// CTriggerOnehop
CTriggerOnehop *CTimer::FindOnehopOnList(int pIndexOnList)
{
	return onehops.Element(pIndexOnList);
}


class CTimerCommands
{
public:
	static void ResetToStart()
	{
		// MOM_TODO(fatalis):
		// if the ent no longer exists this will crash
		// should probably use a handle or something
		CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
		if (cPlayer != NULL)
		{
			CTriggerTimerStart *start;
			if ((start = g_Timer.GetStartTrigger()) != NULL)
			{
				UTIL_SetOrigin(cPlayer, start->WorldSpaceCenter(), true);
				cPlayer->SetAbsVelocity(vec3_origin);
			}
		}
	}
	
	static void ResetToCheckpoint()
	{
		CTriggerCheckpoint *checkpoint;
		if ((checkpoint = g_Timer.GetCurrentCheckpoint()) != NULL)
		{
			UTIL_SetOrigin(UTIL_GetLocalPlayer(), checkpoint->WorldSpaceCenter(), true);
			UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);
		}
	}

	static void CPMenu(const CCommand &args)
	{
		if (g_Timer.IsRunning())
		{
			// MOM_TODO consider having a local timer running,
			//as people may want to time their routes they're using CP menu for
			// MOM_TODO consider KZ (lol)
			//g_Timer.SetRunning(false);
		}
		if (args.ArgC() > 0)
		{
			int sel = Q_atoi(args[1]);
			CBasePlayer* cPlayer = UTIL_GetLocalPlayer();
			switch (sel)
			{
			case 1://create a checkpoint
				g_Timer.CreateCheckpoint(cPlayer);
				break;

			case 2://load previous checkpoint
				g_Timer.TeleportToCP(cPlayer, g_Timer.GetCurrentCPMenuStep());
				break;

			case 3://cycle through checkpoints forwards (+1 % length)
				if (g_Timer.GetCPCount() > 0) 
				{
					g_Timer.SetCurrentCPMenuStep((g_Timer.GetCurrentCPMenuStep() + 1) % g_Timer.GetCPCount());
					g_Timer.TeleportToCP(cPlayer, g_Timer.GetCurrentCPMenuStep());
				}
				break;

			case 4://cycle backwards through checkpoints
				if (g_Timer.GetCPCount() > 0)
				{
					g_Timer.SetCurrentCPMenuStep(g_Timer.GetCurrentCPMenuStep() == 0 ? g_Timer.GetCPCount() - 1 : g_Timer.GetCurrentCPMenuStep() - 1);
					g_Timer.TeleportToCP(cPlayer, g_Timer.GetCurrentCPMenuStep());
				}
				break;

			case 5://remove current checkpoint
				g_Timer.RemoveLastCheckpoint();
				break;

			case 6://remove every checkpoint
				g_Timer.RemoveAllCheckpoints();
				break;

			default:
				if (cPlayer != NULL)
				{
					cPlayer->EmitSound("Momentum.UIMissingMenuSelection");
				}
				break;
			}
		}
	}
};

static ConCommand mom_reset_to_start("mom_restart", CTimerCommands::ResetToStart,"Restarts the run");
static ConCommand mom_reset_to_checkpoint("mom_reset", CTimerCommands::ResetToCheckpoint,"Teleports the player back to the last checkpoint");
static ConCommand mom_cpmenu("cpmenu", CTimerCommands::CPMenu, "", FCVAR_HIDDEN | FCVAR_SERVER_CAN_EXECUTE);

CTimer g_Timer;
