#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"

class CTriggerTimerStart;
class CTriggerCheckpoint;

class CTimer
{
	//DECLARE_CLASS_NOBASE(CTimer);
public:
	void Start(int startTick);
	void Stop();
	void DispatchStateMessage();
	void DispatchResetMessage();
	bool IsRunning();
	void SetRunning(bool running);
	CTriggerTimerStart *GetStartTrigger();
	CTriggerCheckpoint *GetCurrentCheckpoint();
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
	int m_iCurrentStepCP = 0;
	bool m_bUsingCPMenu = false;
};

extern CTimer g_Timer;

#endif // TIMER_H
