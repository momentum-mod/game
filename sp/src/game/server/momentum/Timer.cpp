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

CTriggerCheckpoint *CTimer::GetCurrentCheckpoint()
{
	return m_pCurrentCheckpoint;
}

void CTimer::SetStartTrigger(CTriggerTimerStart *pTrigger)
{
	m_pStartTrigger = pTrigger;
}

void CTimer::SetCurrentCheckpoint(CTriggerCheckpoint *pTrigger)
{
	m_pCurrentCheckpoint = pTrigger;
}

void CTimer::WriteMapFile()
{
	char *pszPath = GetMapFilePath();
	FileHandle_t fh = filesystem->Open(pszPath, "w", "MOD");
	if (fh)
	{
		filesystem->FPrintf(fh, "%f %f %f\n",
			m_vStart.x, m_vStart.y, m_vStart.z);
		filesystem->FPrintf(fh, "%f %f %f\n",
			m_vGoal.x, m_vGoal.y, m_vGoal.z);
		filesystem->FPrintf(fh, "%f", m_flSecondsRecord);
		filesystem->Close(fh);
	}
	delete[] pszPath;
}

// Caller is responsible for delete[]'ing the array.
char *CTimer::GetMapFilePath()
{
	const char *pszMapname = (gpGlobals->mapname).ToCStr();
	size_t sz = Q_strlen("maps/") + strlen(pszMapname) + Q_strlen(".bla") + 1;
	char *pszPath = new char[sz];
	Q_strncpy(pszPath, "maps/", sz);
	Q_strncat(pszPath, pszMapname, sz);
	Q_strncat(pszPath, ".bla", sz);
	Q_FixSlashes(pszPath);
	return pszPath;
}


class CTimerCommands
{
public:
	static void ResetToStart()
	{
		// TODO(fatalis):
		// if the ent no longer exists this will crash
		// should probably use a handle or something
		CTriggerTimerStart *start;
		if ((start = g_Timer.GetStartTrigger()) != NULL)
		{
			UTIL_SetOrigin(UTIL_GetLocalPlayer(), start->WorldSpaceCenter(), true);
		}
	}

	static void ResetToCheckpoint()
	{
		CTriggerCheckpoint *checkpoint;
		if ((checkpoint = g_Timer.GetCurrentCheckpoint()) != NULL)
		{
			UTIL_SetOrigin(UTIL_GetLocalPlayer(), checkpoint->WorldSpaceCenter(), true);
		}
	}
};

static ConCommand mom_reset_to_start("mom_restart", CTimerCommands::ResetToStart);
static ConCommand mom_reset_to_checkpoint("mom_reset", CTimerCommands::ResetToCheckpoint);

CTimer g_Timer;
