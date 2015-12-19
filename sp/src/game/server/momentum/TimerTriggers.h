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
	DECLARE_CLASS(CTriggerCheckpoint, CBaseMomentumTrigger);
	DECLARE_DATADESC();

public:
	void StartTouch(CBaseEntity*);
	int GetCheckpointNumber();
	void SetCheckpointNumber(int);

private:
	int m_iCheckpointNumber;
};

// CTriggerTimerStart
class CTriggerTimerStart : public CTriggerCheckpoint
{
    DECLARE_CLASS(CTriggerTimerStart, CTriggerCheckpoint);

public:
    void EndTouch(CBaseEntity*);
    void StartTouch(CBaseEntity*);
    int GetCheckpointNumber() { return 0; }
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
	DECLARE_CLASS(CTriggerTeleportCheckpoint, CBaseMomentumTrigger);
	DECLARE_DATADESC();

public:
	void StartTouch(CBaseEntity*);

private:
	int m_iCheckpointNumber;
	// Stop player after teleporting him?
	bool m_bResetVelocity = false;
	//void Think();

};

class CTriggerOnehop : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerOnehop, CBaseMomentumTrigger);
	DECLARE_DATADESC();

public:
	void StartTouch(CBaseEntity*);

private:
	bool m_bResetVelocity = true;
	float m_fMaxHoldSeconds = 1;
	// Where to go if it becomes active
	int m_iDestinationCheckpointNumber;

};

class CTriggerResetOnehop : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerResetOnehop, CBaseMomentumTrigger);

public:
	void StartTouch(CBaseEntity*);

};

#endif // TIMERTRIGGERS_H
