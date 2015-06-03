#ifndef TIMERTRIGGERS_H
#define TIMERTRIGGERS_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

// CBaseMomentumTrigger
class CBaseMomentumTrigger: public CTriggerMultiple
{
	DECLARE_CLASS(CBaseMomentumTrigger, CTriggerMultiple);

public:
	virtual void Spawn();
};


// CTriggerTimerStart
class CTriggerTimerStart : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerTimerStart, CBaseMomentumTrigger);

public:
	void EndTouch(CBaseEntity*);
};


// CTriggerTimerStop
class CTriggerTimerStop : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerTimerStop, CBaseMomentumTrigger);

public:
	void StartTouch(CBaseEntity*);
};


// CTriggerCheckpoint
class CTriggerCheckpoint : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerCheckpoint, CTriggerMultiple);
	DECLARE_DATADESC();

public:
	void StartTouch(CBaseEntity*);

	int GetCheckpointNumber();

private:
	int m_iCheckpointNumber;
};


#endif // TIMERTRIGGERS_H
