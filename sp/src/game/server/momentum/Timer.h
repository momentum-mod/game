#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"

class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerOnehop;

class CTimer
{
	//DECLARE_CLASS_NOBASE(CTimer);
public:
	void Start(int startTick);
	void Stop();
	void DispatchStateMessage();
	void DispatchResetMessage();
	void DispatchCheckpointMessage();
	bool IsRunning();
	void SetRunning(bool running);
	CTriggerTimerStart *GetStartTrigger();
	CTriggerCheckpoint *GetCurrentCheckpoint();
	CTriggerCheckpoint *GetCheckpointAt(int checkpointNumber);
	void SetStartTrigger(CTriggerTimerStart *pTrigger);
	void SetCurrentCheckpointTrigger(CTriggerCheckpoint *pTrigger);
	int GetCurrentCPMenuStep()
	{
		return m_iCurrentStepCP;
	}
	
	//For leaderboard use later on
	bool IsUsingCPMenu() 
	{
		return m_bUsingCPMenu;
	}
	// CheckpointMenu stuff

	void CreateCheckpoint(CBasePlayer*);
	void RemoveLastCheckpoint();
	void RemoveAllCheckpoints() 
	{
		checkpoints.RemoveAll();
		m_iCurrentStepCP = -1;
		m_bUsingCPMenu = false;
	}
	void TeleportToCP(CBasePlayer*, int);
	void SetCurrentCPMenuStep(int newNum) 
	{
		m_iCurrentStepCP = newNum;
	}
	int GetCPCount() 
	{
		return checkpoints.Size();
	}
	// Trigger_Onehop stuff

	// Removes the given Onehop form the hopped list.
	// Returns: True if deleted, False if not found.
	bool RemoveOnehopFromList(CTriggerOnehop* pTrigger);
	// Adds the give Onehop to the hopped list.
	// Returns: Its new index.
	int AddOnehopToListTail(CTriggerOnehop* pTrigger);
	// Finds a Onehop on the hopped list.
	// Returns: Its index. -1 if not found
	int FindOnehopOnList(CTriggerOnehop* pTrigger);
	void RemoveAllOnehopsFromList() { onehops.RemoveAll(); }
	int GetOnehopListCount() { return onehops.Count(); }
	CTriggerOnehop* FindOnehopOnList(int pIndexOnList);

private:

	int m_iStartTick;
	bool m_bIsRunning;
	bool m_bIsPaused;

	CTriggerTimerStart *m_pStartTrigger;
	CTriggerCheckpoint *m_pCurrentCheckpoint;

	struct Checkpoint {
		Vector pos;
		Vector vel;
		QAngle ang;
	};
	CUtlVector<Checkpoint> checkpoints;
	CUtlVector<CTriggerOnehop*> onehops;
	int m_iCurrentStepCP = 0;
	bool m_bUsingCPMenu = false;
};

extern CTimer g_Timer;

#endif // TIMER_H
