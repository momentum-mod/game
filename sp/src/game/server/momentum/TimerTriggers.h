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
	// The start is always the first checkpoint: 0
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
	int GetDestinationCheckpointNumber() { return m_iCheckpointNumber; }
	bool GetShouldStopPlayer() { return m_bResetVelocity; };
	// -1: Current checkpoint
	// default: Checkpoint with pNewNumber index
	void SetDestinationCheckpointNumber(int);
	void SetShouldStopPlayer(bool);

private:
	// Where to teleport the player.
	// -1: Current checkpoint
	// Default: Checkpoint with that index
	int m_iCheckpointNumber;
	// Should the player be stopped after teleport?
	bool m_bResetVelocity = false;

};

// CTriggerOnehop
class CTriggerOnehop : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerOnehop, CBaseMomentumTrigger);
	DECLARE_DATADESC();

public:
	void StartTouch(CBaseEntity*);
	int GetDestinationIndex() { return m_iDestinationCheckpointNumber; }
	bool GetShouldStopPlayer() { return m_bResetVelocity; }
	float GetHoldTeleportTime() { return m_fMaxHoldSeconds; }
	void SetDestinationIndex(int pNewIndex);
	void SetShouldStopPlayer(bool pShouldStop);
	void SetHoldTeleportTime(float pHoldTime);

private:
	// Should the player be stopped after teleport?
	bool m_bResetVelocity = true;
	// Seconds to hold before activating the teleport
	float m_fMaxHoldSeconds = 1;
	// Where to teleport the player if it becomes active
	int m_iDestinationCheckpointNumber = -1;
};

// CTriggerResetOnehop
class CTriggerResetOnehop : public CBaseMomentumTrigger
{
	DECLARE_CLASS(CTriggerResetOnehop, CBaseMomentumTrigger);

public:
	void StartTouch(CBaseEntity*);

};

#endif // TIMERTRIGGERS_H
