#ifndef TIMERTRIGGERS_H
#define TIMERTRIGGERS_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"
#include "filters.h"

// CBaseMomentumTrigger
class CBaseMomentumTrigger : public CTriggerMultiple
{
	DECLARE_CLASS(CBaseMomentumTrigger, CTriggerMultiple);

public:
	virtual void Spawn();
	virtual void ResetCheckpoints();
};

// CTriggerTimerStart
class CTriggerTimerStart : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerTimerStart, CBaseMomentumTrigger);

public:
	void EndTouch(CBaseEntity*);
	void StartTouch(CBaseEntity*);
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
	void EndTouch(CBaseEntity*);
	int GetCheckpointNumber();
	void SetCheckpointNumber(int);

private:
	int m_iCheckpointNumber;
};

// CFilterCheckpoint
class CFilterCheckpoint : public CBaseFilter
{
    DECLARE_CLASS(CFilterCheckpoint, CBaseFilter);
    DECLARE_DATADESC();

public:
    bool PassesFilterImpl(CBaseEntity*, CBaseEntity*);

private:
    int m_iCheckpointNumber;

};

// CTriggerTeleportCheckpoint
class CTriggerTeleportCheckpoint : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerTeleportCheckpoint, CTriggerMultiple);
	DECLARE_DATADESC();

public:
	void StartTouch(CBaseEntity*);

private:
	int m_iCheckpointNumber;
	// Stop player after teleporting him?
	bool m_bResetVelocity = false;
	void Think();

};

class CTriggerOnehop : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerOnehop, CTriggerMultiple);
	DECLARE_DATADESC();

public:
	void StartTouch(CBaseEntity*);

private:
	// Have we hopped in it?
	bool m_bHoppedIn = false;
	bool m_bResetVelocity = true;
	float m_fMaxHoldSeconds = 0.2f;
	// Where to go if it becomes active
	int m_iDestinationCheckpointNumber;
};

class CTriggerResetOnehop : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerResetOnehop, CTriggerMultiple);

public:
	void StartTouch(CBaseEntity*);

};

#endif // TIMERTRIGGERS_H
