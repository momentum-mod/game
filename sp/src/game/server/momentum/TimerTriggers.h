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
};

// CTriggerTimerStop
class CTriggerTimerStop : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTimerStop, CBaseMomentumTrigger);

public:
    void StartTouch(CBaseEntity*);
};

// CTriggerTeleportEnt
class CTriggerTeleportEnt : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTeleportEnt, CBaseMomentumTrigger);
    DECLARE_DATADESC();

public:
    //This void teleports the touching entity!
    void StartTouch(CBaseEntity*);
    // Used by children classes to define what ent to teleport to (see CTriggerOneHop)
    void SetDestinationEnt(CBaseEntity *ent) { pDestinationEnt = ent; }
    bool ShouldStopPlayer() { return m_bResetVelocity; }
    bool ShouldResetAngles() { return m_bResetAngles; }
    void SetShouldStopPlayer(bool newB) { m_bResetVelocity = newB; }
    void SetShouldResetAngles(bool newB) { m_bResetAngles = newB; }

private:
    bool m_bResetVelocity;
    bool m_bResetAngles;
    CBaseEntity *pDestinationEnt;
};

// CTriggerCheckpoint, used by mappers for teleporting 
class CTriggerCheckpoint : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerCheckpoint, CBaseMomentumTrigger);
    DECLARE_DATADESC();

public:
    void StartTouch(CBaseEntity*);
    // the following is only used by CFilterCheckpoint
    int GetCheckpointNumber() { return m_iCheckpointNumber; }
    // The following is used by mapzones.cpp
    void SetCheckpointNumber(int newInt) { m_iCheckpointNumber = newInt; }

private:
    int m_iCheckpointNumber;
};

// CTriggerStage
// used to declare which major part of the map the player has gotten to
class CTriggerStage : public CTriggerCheckpoint
{
    DECLARE_CLASS(CTriggerStage, CTriggerCheckpoint);
    DECLARE_DATADESC();

public:
    void StartTouch(CBaseEntity*);
    //Used by CTimer and CStageFilter
    int GetStageNumber() { return m_iStageNumber; }
    void SetStageNumber(int newInt) { m_iStageNumber = newInt; }
    int GetCheckpointNumber() { return -1; }//Override, use GetStageNumber()

private:
    int m_iStageNumber;
};

// CTriggerTimerStart
class CTriggerTimerStart : public CTriggerStage
{
    DECLARE_CLASS(CTriggerTimerStart, CTriggerStage);
    DECLARE_DATADESC();

public:
    void EndTouch(CBaseEntity*);
    void StartTouch(CBaseEntity*);
    void Spawn();
    // The start is always the first stage/checkpoint
    int GetCheckpointNumber() { return -1; }//Override
    int GetStageNumber() { return 1; }
    float GetMaxLeaveSpeed() { return m_fMaxLeaveSpeed; }
    void SetMaxLeaveSpeed(float pMaxSpeed);
    bool IsLimitingSpeed() { return HasSpawnFlags(SF_LIMIT_LEAVE_SPEED); }
    void SetIsLimitingSpeed(bool pIsLimitingSpeed);

private:
    // How fast can the player leave the start trigger?
    float m_fMaxLeaveSpeed = 280;
    // Limit max leave speed to m_fMaxLeaveSpeed?
    const int SF_LIMIT_LEAVE_SPEED = 0x2;

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
class CTriggerTeleportCheckpoint : public CTriggerTeleportEnt
{
    DECLARE_CLASS(CTriggerTeleportCheckpoint, CTriggerTeleportEnt);

public:
    void StartTouch(CBaseEntity*);
};

// CTriggerOnehop
class CTriggerOnehop : public CTriggerTeleportEnt
{
    DECLARE_CLASS(CTriggerOnehop, CTriggerTeleportEnt);
    DECLARE_DATADESC();

public:
    void StartTouch(CBaseEntity*);
    float GetHoldTeleportTime() { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think();

private:
    // The time that the player initally touched the trigger
    float m_fStartTouchedTime = 0.0f;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds = 1;
    // Reset hop state if player hops onto another different onehop
    const int SF_TELEPORT_RESET_ONEHOP = 0x2;

};

// CTriggerResetOnehop
class CTriggerResetOnehop : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerResetOnehop, CBaseMomentumTrigger);

public:
    void StartTouch(CBaseEntity*);

};

// CTriggerMultihop
class CTriggerMultihop : public CTriggerTeleportEnt
{
    DECLARE_CLASS(CTriggerMultihop, CTriggerTeleportEnt);
    DECLARE_DATADESC();

public:
    void StartTouch(CBaseEntity*);
    void EndTouch(CBaseEntity*);
    float GetHoldTeleportTime() { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think();

private:
    // The time that the player initally touched the trigger. -1 if not checking for teleport
    float m_fStartTouchedTime = 0.0f;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds = 1;

};

// CTriggerUserInput
class CTriggerUserInput : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerUserInput, CBaseMomentumTrigger);
    DECLARE_DATADESC();
public:
    enum key { forward, back, moveleft, moveright };
    key m_eKey;

    void Think();
    void Spawn();

    COutputEvent m_OnKeyPressed;

private:
    int m_ButtonRep;

};

#endif // TIMERTRIGGERS_H
