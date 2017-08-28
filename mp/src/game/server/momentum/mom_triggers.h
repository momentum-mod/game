#ifndef TIMERTRIGGERS_H
#define TIMERTRIGGERS_H
#ifdef _WIN32
#pragma once
#endif

#include "filters.h"
#include "func_break.h"
#include "triggers.h"
#include <momentum/mom_gamerules.h>

// spawnflags
enum
{
    // CTriggerTimerStart
    SF_LIMIT_LEAVE_SPEED = 0x0001, // Limit speed if player bhopped in start zone?
    SF_USE_LOOKANGLES = 0x0002,    // Use look angles?
    // CTriggerOneHop
    SF_TELEPORT_RESET_ONEHOP = 0x0010, // Reset hop state if player hops onto another different onehop
    // CTriggerLimitMove
    LIMIT_JUMP = 0x0020,   // prevent player from jumping
    LIMIT_CROUCH = 0x0040, // prevent player from croching
    LIMIT_BHOP = 0x0080,   // prevent player from bhopping
    // CFuncShootBost and CTriggerMomentumPush
    SF_PUSH_DIRECTION_AS_FINAL_FORCE =
        0x0100, // Use the direction vector as final force instead of calculating it by force amount
    // CTriggerMomentumPush
    SF_PUSH_ONETOUCH = 0x0200, // Only allow for one touch
    SF_PUSH_ONSTART = 0x0400,  // Modify player velocity on StartTouch
    SF_PUSH_ONEND = 0x0800,    // Modify player velocity on EndTouch
    // CTriggerTeleport
    SF_TELE_ONEXIT = 0x1000,  // Teleport the player on EndTouch instead of StartTouch
};

// CBaseMomentumTrigger
class CBaseMomentumTrigger : public CTriggerMultiple
{
    DECLARE_CLASS(CBaseMomentumTrigger, CTriggerMultiple);

  public:
    void Spawn() OVERRIDE;
    // Used to calculate if a position is inside of this trigger's bounds
    bool ContainsPosition(const Vector &pos) { return CollisionProp()->IsPointInBounds(pos); }
};

// CTriggerTimerStop
class CTriggerTimerStop : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTimerStop, CBaseMomentumTrigger);

  public:
    void StartTouch(CBaseEntity *) OVERRIDE;
    void EndTouch(CBaseEntity *) OVERRIDE;
};

// CTriggerTeleportEnt
class CTriggerTeleportEnt : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTeleportEnt, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    // This void teleports the touching entity!
    void StartTouch(CBaseEntity *) OVERRIDE;
    void EndTouch(CBaseEntity *) OVERRIDE;
    // Used by children classes to define what ent to teleport to (see CTriggerOneHop)
    void SetDestinationEnt(CBaseEntity *ent) { pDestinationEnt = ent; }
    bool ShouldStopPlayer() const { return m_bResetVelocity; }
    bool ShouldResetAngles() const { return m_bResetAngles; }
    void SetShouldStopPlayer(const bool newB) { m_bResetVelocity = newB; }
    void SetShouldResetAngles(const bool newB) { m_bResetAngles = newB; }

    virtual void AfterTeleport(){}; // base class does nothing

protected:
    void HandleTeleport(CBaseEntity *);

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
    void StartTouch(CBaseEntity *) OVERRIDE;
    // the following is only used by CFilterCheckpoint
    virtual int GetCheckpointNumber() const { return m_iCheckpointNumber; }
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
    void StartTouch(CBaseEntity *) OVERRIDE;
    void EndTouch(CBaseEntity *) OVERRIDE;
    void Spawn() OVERRIDE
    {
        SetCheckpointNumber(-1);
        BaseClass::Spawn();
    }
    // Used by CTimer and CStageFilter
    virtual int GetStageNumber() const { return m_iStageNumber; }
    void SetStageNumber(const int newInt) { m_iStageNumber = newInt; }
    // Override, use GetStageNumber()
    int GetCheckpointNumber() const OVERRIDE { return -1; }

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
    void EndTouch(CBaseEntity *) OVERRIDE;
    void StartTouch(CBaseEntity *) OVERRIDE;
    void Spawn() OVERRIDE;

    // The start is always the first stage/checkpoint
    int GetCheckpointNumber() const OVERRIDE { return -1; } // Override
    int GetStageNumber() const OVERRIDE { return 1; }
    float GetMaxLeaveSpeed() const { return m_fBhopLeaveSpeed; }
    void SetMaxLeaveSpeed(const float maxLeaveSpeed);
    void SetLookAngles(const QAngle &newang);
    QAngle GetLookAngles() const { return m_angLook; }

    // spawnflags
    bool IsLimitingSpeed() const { return HasSpawnFlags(SF_LIMIT_LEAVE_SPEED); }
    void SetIsLimitingSpeed(const bool pIsLimitingSpeed);
    void SetHasLookAngles(const bool bHasLook);
    bool HasLookAngles() const { return HasSpawnFlags(SF_USE_LOOKANGLES); }

  private:
    QAngle m_angLook;

    // How fast can player leave start trigger if they bhopped?
    float m_fBhopLeaveSpeed;
};

// CFilterCheckpoint
class CFilterCheckpoint : public CBaseFilter
{
    DECLARE_CLASS(CFilterCheckpoint, CBaseFilter);
    DECLARE_DATADESC();

  public:
    bool PassesFilterImpl(CBaseEntity *, CBaseEntity *) OVERRIDE;

  private:
    int m_iCheckpointNumber;
};

// CTriggerTeleportCheckpoint
class CTriggerTeleportCheckpoint : public CTriggerTeleportEnt
{
    DECLARE_CLASS(CTriggerTeleportCheckpoint, CTriggerTeleportEnt);

  public:
    void StartTouch(CBaseEntity *) OVERRIDE;
};

// CTriggerOnehop
class CTriggerOnehop : public CTriggerTeleportEnt
{
  public:
    DECLARE_CLASS(CTriggerOnehop, CTriggerTeleportEnt);
    DECLARE_DATADESC();

    CTriggerOnehop();

  public:
    void StartTouch(CBaseEntity *) OVERRIDE;
    float GetHoldTeleportTime() const { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(const float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think() OVERRIDE;
    void AfterTeleport() OVERRIDE
    {
        m_fStartTouchedTime = -1.0f;
        SetDestinationEnt(nullptr);
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
    void StartTouch(CBaseEntity *) OVERRIDE;
};

// CTriggerMultihop
class CTriggerMultihop : public CTriggerTeleportEnt
{
  public:
    DECLARE_CLASS(CTriggerMultihop, CTriggerTeleportEnt);
    DECLARE_DATADESC();

    CTriggerMultihop();

  public:
    void StartTouch(CBaseEntity *) OVERRIDE;
    void EndTouch(CBaseEntity *) OVERRIDE;
    float GetHoldTeleportTime() const { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(const float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think() OVERRIDE;
    void AfterTeleport() OVERRIDE
    {
        m_fStartTouchedTime = -1.0f;
        SetDestinationEnt(nullptr);
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
    enum key
    {
        forward,
        back,
        moveleft,
        moveright,
        jump,
        duck,
        attack,
        attack2,
        reload
    };
    key m_eKey;
    void Think() OVERRIDE;
    void Spawn() OVERRIDE;
    COutputEvent m_OnKeyPressed;

  private:
    int m_ButtonRep;
};

#define FL_BHOP_TIMER 0.15f

// CTriggerLimitMovement
class CTriggerLimitMovement : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerLimitMovement, CBaseMomentumTrigger);

  public:
    void Think() OVERRIDE;
    void StartTouch(CBaseEntity *pOther) OVERRIDE;
    void EndTouch(CBaseEntity *pOther) OVERRIDE;

  private:
    CountdownTimer m_BhopTimer;
};

// CFuncShootBoost
class CFuncShootBoost : public CBreakable
{
    DECLARE_CLASS(CFuncShootBoost, CBreakable);
    DECLARE_DATADESC();

  public:
    void Spawn() OVERRIDE;
    int OnTakeDamage(const CTakeDamageInfo &info) OVERRIDE;
    // Force in units per seconds applied to the player
    float m_fPushForce;
    // 0: No
    // 1: Yes
    // 2: Only if the player's velocity is lower than the push velocity, set player's velocity to final push velocity
    // 3: Only if the player's velocity is lower than the push velocity, increase player's velocity by final push
    // velocity
    int m_iIncrease;
    // Dictates the direction of push
    Vector m_vPushDir;
    // If not null, dictates which entity the attacker must be touching for the func to work
    CBaseEntity *m_Destination;
};

// CTriggerMomentumPush
class CTriggerMomentumPush : public CBaseMomentumTrigger
{
  public:
    DECLARE_CLASS(CTriggerMomentumPush, CBaseMomentumTrigger);
    DECLARE_DATADESC();

    CTriggerMomentumPush();

  public:
    void StartTouch(CBaseEntity *) OVERRIDE;
    void EndTouch(CBaseEntity *) OVERRIDE;
    // Called when (and by) either a StartTouch() or EndTouch() event happens and their requisites are met
    void OnSuccessfulTouch(CBaseEntity *);

  private:
    // Force in units per seconds applied to the player
    float m_fPushForce;
    // 1: SetPlayerVelocity to final push force
    // 2: Increase player's current velocity by push final foce ammount // This is almost like the default trigger_push
    // behaviour
    // 3: Only set the player's velocity to the final push velocity if player's velocity is lower than final push
    // velocity
    int m_iIncrease;
    // Dictates the direction of push
    Vector m_vPushDir;
    // Pointer to the destination entity if a teleport is needed
    CBaseEntity *m_Destination;
};

class CTriggerSlide : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerSlide, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    void Think() OVERRIDE;
    void StartTouch(CBaseEntity *pOther) OVERRIDE;
    void EndTouch(CBaseEntity *pOther) OVERRIDE;

  public:
    bool m_bSliding, m_bStuck;
    float m_flGravity,m_flSavedGravity;
};

#endif // TIMERTRIGGERS_H
