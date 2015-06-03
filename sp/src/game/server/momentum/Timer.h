#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

class CTriggerTimerStart;
class CTriggerCheckpoint;

class CTimer
{
	//DECLARE_CLASS_NOBASE(CTimer);
public:
	void Start(int startTick);
	void Stop();
	void DispatchStateMessage();
	bool IsRunning();
	void SetRunning(bool running);
	CTriggerTimerStart *GetStartTrigger();
	CTriggerCheckpoint *GetCurrentCheckpoint();
	void SetStartTrigger(CTriggerTimerStart *pTrigger);
	void SetCurrentCheckpoint(CTriggerCheckpoint *pTrigger);

private:
	void WriteMapFile();
	// Caller is responsible for delete[]'ing the array.
	char *GetMapFilePath();

	int m_iStartTick;
	bool m_bIsRunning;
	Vector m_vStart;
	Vector m_vGoal;
	bool m_bIsPaused;
	float m_flSecondsRecord;

	CTriggerTimerStart *m_pStartTrigger;
	CTriggerCheckpoint *m_pCurrentCheckpoint;
};

extern CTimer g_Timer;

#endif // TIMER_H
