#ifndef TIMERTRIGGERS_H
#define TIMERTRIGGERS_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"
#include "filters.h"
#include "func_break.h"
#include <momentum/mom_gamerules.h>

// spawnflags 
enum {
    //CTriggerTimerStart
    SF_LIMIT_LEAVE_SPEED = 0x0001,           // Limit speed if player bhopped in start zone?
    SF_USE_LOOKANGLES = 0x0002,             // Use look angles?
    //CTriggerOneHop
    SF_TELEPORT_RESET_ONEHOP = 0x0010,      // Reset hop state if player hops onto another different onehop
    //CTriggerLimitMove
    LIMIT_JUMP = 0x0020,                    //prevent player from jumping
    LIMIT_CROUCH = 0x0040,                  //prevent player from croching
    LIMIT_BHOP = 0x0080,                    //prevent player from bhopping
    //CFuncShootBost and CTriggerMomentumPush
    SF_PUSH_DIRECTION_AS_FINAL_FORCE = 0x0100,  // Use the direction vector as final force instead of calculating it by force amount
    //CTriggerMomentumPush
    SF_PUSH_ONETOUCH = 0x0200,               // Only allow for one touch
    SF_PUSH_ONSTART = 0x0400,                // Modify player velocity on StartTouch
    SF_PUSH_ONEND = 0x0800,                  // Modify player velocity on EndTouch
};

// CBaseMomentumTrigger
class CBaseMomentumTrigger : public CTriggerMultiple
{
    DECLARE_CLASS(CBaseMomentumTrigger, CTriggerMultiple);

public:
    void Spawn() override;
    //Used to calculate if a position is inside of this trigger's bounds
    bool ContainsPosition(const Vector &pos)
    {
        return CollisionProp()->IsPointInBounds(pos);
    }
};

// CTriggerTimerStop
class CTriggerTimerStop : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTimerStop, CBaseMomentumTrigger);

public:
    void StartTouch(CBaseEntity*) override;
    void EndTouch(CBaseEntity*) override;
};

// CTriggerTeleportEnt
class CTriggerTeleportEnt : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTeleportEnt, CBaseMomentumTrigger);
    DECLARE_DATADESC();

public:
    //This void teleports the touching entity!
    void StartTouch(CBaseEntity*) override;
    // Used by children classes to define what ent to teleport to (see CTriggerOneHop)
    void SetDestinationEnt(CBaseEntity *ent) { pDestinationEnt = ent; }
    bool ShouldStopPlayer() { return m_bResetVelocity; }
    bool ShouldResetAngles() { return m_bResetAngles; }
    void SetShouldStopPlayer(bool newB) { m_bResetVelocity = newB; }
    void SetShouldResetAngles(bool newB) { m_bResetAngles = newB; }

    virtual void AfterTeleport() {};//base class does nothing

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
    void StartTouch(CBaseEntity*) override;
    // the following is only used by CFilterCheckpoint
    virtual int GetCheckpointNumber() { return m_iCheckpointNumber; }
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
    void StartTouch(CBaseEntity*) override;
    void EndTouch(CBaseEntity*) override;
    void Spawn() override
    {
        SetCheckpointNumber(-1);
        BaseClass::Spawn();
    }
    //Used by CTimer and CStageFilter
    virtual int GetStageNumber() { return m_iStageNumber; }
    void SetStageNumber(int newInt) { m_iStageNumber = newInt; }
    //Override, use GetStageNumber()
    int GetCheckpointNumber() override
    { return -1; }

private:
    int m_iStageNumber;
};

// CTriggerTimerStart
class CTriggerTimerStart : public CTriggerStage
{
public:
    DECLARE_CLASS(CTriggerTimerStart, CTriggerStage);
    DECLARE_DATADESC();

    CTriggerTimerStart();
public:
    void EndTouch(CBaseEntity*) override;
    void StartTouch(CBaseEntity*) override;
    void Spawn() override;

    // The start is always the first stage/checkpoint
    int GetCheckpointNumber() override
    { return -1; }//Override
    int GetStageNumber() override
    { return 1; }
    float GetMaxLeaveSpeed() { return m_fBhopLeaveSpeed; }
    void SetMaxLeaveSpeed(float maxLeaveSpeed);
    void SetLookAngles(QAngle newang);
    QAngle GetLookAngles() { return m_angLook; }
    // MOM_TODO: Is this even used right now??
    void SetPunishSpeed(float pPunishSpeed);
    float GetPunishSpeed() { return m_fPunishSpeed; }

    //spawnflags
    bool IsLimitingSpeed() { return HasSpawnFlags(SF_LIMIT_LEAVE_SPEED); }
    void SetIsLimitingSpeed(bool pIsLimitingSpeed);
    void SetHasLookAngles(bool bHasLook);
    bool GetHasLookAngles() { return HasSpawnFlags(SF_USE_LOOKANGLES); }

private:
    QAngle m_angLook;

    //How fast can player leave start trigger if they bhopped?
    float m_fBhopLeaveSpeed;
    float m_fPunishSpeed;
};

// CFilterCheckpoint
class CFilterCheckpoint : public CBaseFilter
{
    DECLARE_CLASS(CFilterCheckpoint, CBaseFilter);
    DECLARE_DATADESC();

public:
    bool PassesFilterImpl(CBaseEntity*, CBaseEntity*) override;

private:
    int m_iCheckpointNumber;

};

// CTriggerTeleportCheckpoint
class CTriggerTeleportCheckpoint : public CTriggerTeleportEnt
{
    DECLARE_CLASS(CTriggerTeleportCheckpoint, CTriggerTeleportEnt);

public:
    void StartTouch(CBaseEntity*) override;
};

// CTriggerOnehop
class CTriggerOnehop : public CTriggerTeleportEnt
{
public:
    DECLARE_CLASS(CTriggerOnehop, CTriggerTeleportEnt);
    DECLARE_DATADESC();
    
    CTriggerOnehop();

public:
    void StartTouch(CBaseEntity*) override;
    float GetHoldTeleportTime() { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think() override;
    void AfterTeleport() override
    {
        m_fStartTouchedTime = -1.0f; SetDestinationEnt(nullptr);
    }

private:
    // The time that the player initally touched the trigger
    float m_fStartTouchedTime;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds;

};

// CTriggerResetOnehop
class CTriggerResetOnehop : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerResetOnehop, CBaseMomentumTrigger);

public:
    void StartTouch(CBaseEntity*) override;

};

// CTriggerMultihop
class CTriggerMultihop : public CTriggerTeleportEnt
{
public:
    DECLARE_CLASS(CTriggerMultihop, CTriggerTeleportEnt);
    DECLARE_DATADESC();

    CTriggerMultihop();
    
public:
    void StartTouch(CBaseEntity*) override;
    void EndTouch(CBaseEntity*) override;
    float GetHoldTeleportTime() { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think() override;
    void AfterTeleport() override
    {
        m_fStartTouchedTime = -1.0f; SetDestinationEnt(nullptr);
    }

private:
    // The time that the player initally touched the trigger. -1 if not checking for teleport
    float m_fStartTouchedTime;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds;

};

// CTriggerUserInput
class CTriggerUserInput : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerUserInput, CBaseMomentumTrigger);
    DECLARE_DATADESC();

public:
    enum key { forward, back, moveleft, moveright, jump, duck, attack, attack2, reload };
    key m_eKey;
    void Think() override;
    void Spawn() override;
    COutputEvent m_OnKeyPressed;

private:
    int m_ButtonRep;

};

// CTriggerLimitMovement
class CTriggerLimitMovement : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerLimitMovement, CBaseMomentumTrigger);

public:
    void Think() override;
    void StartTouch(CBaseEntity *pOther) override;
    void EndTouch(CBaseEntity *pOther) override;

private:
    CountdownTimer m_BhopTimer;
    static constexpr float FL_BHOP_TIMER = 0.15;
};


// CFuncShootBoost
class CFuncShootBoost : public CBreakable
{
    DECLARE_CLASS(CFuncShootBoost, CBreakable);
    DECLARE_DATADESC();

public:
    void Spawn() override;
    int OnTakeDamage(const CTakeDamageInfo &info) override;
    // Force in units per seconds applied to the player
    float m_fPushForce;
    // 0: No
    // 1: Yes
    // 2: Only if the player's velocity is lower than the push velocity, set player's velocity to final push velocity
    // 3: Only if the player's velocity is lower than the push velocity, increase player's velocity by final push velocity
    int m_iIncrease;
    // Dictates the direction of push
    Vector m_vPushDir;
    // If not null, dictates which entity the attacker must be touching for the func to work
    CBaseEntity *m_Destination;
};

// CTriggerMomentumPush
class CTriggerMomentumPush : public CTriggerTeleportEnt
{
public:
    DECLARE_CLASS(CTriggerMomentumPush, CTriggerTeleportEnt);
    DECLARE_DATADESC();

    CTriggerMomentumPush();
public:
    void StartTouch(CBaseEntity*) override;
    void EndTouch(CBaseEntity*) override;
    // Called when (and by) either a StartTouch() or EndTouch() event happens and their requisites are met
    void OnSuccessfulTouch(CBaseEntity*);
    float GetHoldTeleportTime() { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void AfterTeleport() override
    {
        m_fStartTouchedTime = -1.0f; SetDestinationEnt(nullptr);
    }

private:
    // The time that the player initally touched the trigger
    float m_fStartTouchedTime;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds;
    // Force in units per seconds applied to the player
    float m_fPushForce;
    // 1: SetPlayerVelocity to final push force
    // 2: Increase player's current velocity by push final foce ammount // This is almost like the default trigger_push behaviour
    // 3: Only set the player's velocity to the final push velocity if player's velocity is lower than final push velocity
    int m_iIncrease;
    // Dictates the direction of push
    Vector m_vPushDir;
    // Pointer to the destination entity if a teleport is needed
    CBaseEntity *m_Destination;
};
#endif // TIMERTRIGGERS_H
