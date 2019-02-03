#pragma once

#include "filters.h"
#include "func_break.h"
#include "modelentities.h"
#include "triggers.h"

class CMomentumPlayer;

// spawnflags
enum
{
    // starts on 0x1000 - SF_TRIGGER_DISALLOW_BOTS
    // CTriggerTimerStart
    SF_LIMIT_LEAVE_SPEED = 1 << 13, // Limit speed if player bhopped in start zone?
    SF_USE_LOOKANGLES = 1 << 14,    // Use look angles?

    // CTriggerOneHop
    SF_TELEPORT_RESET_ONEHOP = 1 << 15, // Reset hop state if player hops onto another different onehop

    // CFuncShootBost and CTriggerMomentumPush
    SF_PUSH_DIRECTION_AS_FINAL_FORCE = 1 << 19, // Use the direction vector as final force instead of calculating it by
                                                // force amount CTriggerMomentumPush
    SF_PUSH_ONETOUCH = 1 << 20,                 // Only allow for one touch
    SF_PUSH_ONSTART = 1 << 21,                  // Modify player velocity on OnStartTouch
    SF_PUSH_ONEND = 1 << 22,                    // Modify player velocity on OnEndTouch
                                                // CTriggerTeleport
    SF_TELE_ONEXIT = 1 << 23,                   // Teleport the player on OnEndTouch instead of OnStartTouch
};

enum
{
    SPEED_NORMAL_LIMIT,
    SPEED_LIMIT_INAIR,
    SPEED_LIMIT_GROUND,
    SPEED_LIMIT_ONLAND,
};

// CBaseMomentumTrigger
class CBaseMomentumTrigger : public CTriggerMultiple
{
    DECLARE_CLASS(CBaseMomentumTrigger, CTriggerMultiple);

  public:
    void Spawn() OVERRIDE;
    // Used to calculate if a position is inside of this trigger's bounds
    bool ContainsPosition(const Vector &pos) { return CollisionProp()->IsPointInBounds(pos); }


    // Point-based zones need a custom collision check
    void InitCustomCollision(CPhysCollide *pPhysCollide, const Vector &vecMins, const Vector &vecMaxs);
    virtual bool TestCollision(const Ray_t &ray, unsigned int mask, trace_t &tr) OVERRIDE;

    const CUtlVector<Vector> &GetZonePoints() const { return m_vZonePoints; }
  private:
    // Point-based zone editing
    CUtlVector<Vector> m_vZonePoints;
    float m_flPointZoneHeight;

    friend class CMomPointZoneBuilder;
};

// CTriggerTimerStop
class CTriggerTimerStop : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTimerStop, CBaseMomentumTrigger);
    DECLARE_NETWORKCLASS();
    DECLARE_DATADESC();

  public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
    int UpdateTransmitState() // always send to all clients
    {
        return SetTransmitState(FL_EDICT_ALWAYS);
    }

    int GetZoneNumber() const { return m_iZoneNumber; };
    void SetZoneNumber(int num) { m_iZoneNumber = num; }

  private:
    int m_iZoneNumber;
};

// CTriggerTeleportEnt
class CTriggerTeleportEnt : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerTeleportEnt, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    // This void teleports the touching entity!
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
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
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    // the following is only used by CFilterCheckpoint
    virtual int GetCheckpointNumber() const { return m_iCheckpointNumber; }
    // The following is used by mapzones.cpp
    void SetCheckpointNumber(int newInt) { m_iCheckpointNumber = newInt; }

  private:
    int m_iCheckpointNumber;
    // Fires when it resets all one hops.
    COutputEvent m_ResetOnehops;
};

// CTriggerStage
// used to declare which major part of the map the player has gotten to
class CTriggerStage : public CTriggerCheckpoint
{
    DECLARE_CLASS(CTriggerStage, CTriggerCheckpoint);
    DECLARE_DATADESC();

  public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
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
    DECLARE_NETWORKCLASS();
    DECLARE_DATADESC();

    CTriggerTimerStart();

  public:
    void OnEndTouch(CBaseEntity *) OVERRIDE;
    void OnStartTouch(CBaseEntity *) OVERRIDE;

    int UpdateTransmitState() // always send to all clients
    {
        return SetTransmitState(FL_EDICT_ALWAYS);
    }

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
    bool &StartOnJump() { return m_bTimerStartOnJump; }
    int &LimitSpeedType() { return m_iLimitSpeedType; }
    int GetZoneNumber() const { return m_iZoneNumber; }
    void SetZoneNumber(int num) { m_iZoneNumber = num; }

  private:
    QAngle m_angLook;

    // How fast can player leave start trigger if they bhopped?
    float m_fBhopLeaveSpeed;
    // This might be needed in case for some maps where the start zone is above a descent, when there is a wall in
    // front: ref bhop_w1s1
    bool m_bTimerStartOnJump;
    int m_iLimitSpeedType;
    // 0 is main start zone, others are bonuses.
    int m_iZoneNumber;
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
    void OnStartTouch(CBaseEntity *) OVERRIDE;
};

// CTriggerOnehop
class CTriggerOnehop : public CTriggerTeleportEnt
{
  public:
    DECLARE_CLASS(CTriggerOnehop, CTriggerTeleportEnt);
    DECLARE_DATADESC();

    CTriggerOnehop();

  public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    float GetHoldTeleportTime() const { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(const float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think() OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
    void AfterTeleport() OVERRIDE
    {
        m_fOnStartTouchedTime = -1.0f;
        SetDestinationEnt(nullptr);
    }

    void SethopNoLongerJumpableFired(bool bState) { m_bhopNoLongerJumpableFired = bState; }

  private:
    // The time that the player initally touched the trigger
    float m_fOnStartTouchedTime;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds;
    // Fires the output when the player cannot go back to the trigger
    COutputEvent m_hopNoLongerJumpable;
    // Is m_hopNoLongerJumpable was already fired?
    bool m_bhopNoLongerJumpableFired;
};

// CTriggerResetOnehop
class CTriggerResetOnehop : public CBaseMomentumTrigger
{
  public:
    DECLARE_CLASS(CTriggerResetOnehop, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;

  private:
    // Fires when it resets all one hops.
    COutputEvent m_ResetOnehops;
};

// CTriggerMultihop
class CTriggerMultihop : public CTriggerTeleportEnt
{
  public:
    DECLARE_CLASS(CTriggerMultihop, CTriggerTeleportEnt);
    DECLARE_DATADESC();

    CTriggerMultihop();

  public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
    float GetHoldTeleportTime() const { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(const float pHoldTime) { m_fMaxHoldSeconds = pHoldTime; }
    void Think() OVERRIDE;
    void AfterTeleport() OVERRIDE
    {
        m_fOnStartTouchedTime = -1.0f;
        SetDestinationEnt(nullptr);
    }

  private:
    // The time that the player initally touched the trigger. -1 if not checking for teleport
    float m_fOnStartTouchedTime;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds;
};

// CTriggerUserInput
class CTriggerUserInput : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerUserInput, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    enum Key
    {
        KEY_FORWARD = 0,
        KEY_BACK,
        KEY_MOVELEFT,
        KEY_MOVERIGHT,
        KEY_JUMP,
        KEY_DUCK,
        KEY_ATTACK,
        KEY_ATTACK2,
        KEY_RELOAD
    };
    Key m_eKey;
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
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void OnEndTouch(CBaseEntity *pOther) OVERRIDE;

    // spawnflags
    // starts on 0x1000 (or 1 << 12) - SF_TRIGGER_DISALLOW_BOTS
    enum
    {
        SF_LIMIT_FORWARD = 1 << 13, // prevent moving forward
        SF_LIMIT_LEFT = 1 << 14,    // prevent moving to the left
        SF_LIMIT_RIGHT = 1 << 15,   // prevent moving to the right
        SF_LIMIT_BACK = 1 << 16,    // prevent moving backwards
        SF_LIMIT_JUMP = 1 << 17,    // prevent player from jumping
        SF_LIMIT_CROUCH = 1 << 18,  // prevent player from crouching
        SF_LIMIT_BHOP = 1 << 19     // prevent player from bhopping
    };
private:
    template <class T>
    void ToggleButtons(T *pEnt, bool bEnable);
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
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
    // Called when (and by) either a OnStartTouch() or OnEndTouch() event happens and their requisites are met
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
    DECLARE_NETWORKCLASS();
    DECLARE_DATADESC();

  public:
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void OnEndTouch(CBaseEntity *pOther) OVERRIDE;
    int UpdateTransmitState() // always send to all clients
    {
        return SetTransmitState(FL_EDICT_ALWAYS);
    }

  public:
    CNetworkVar(bool, m_bStuckOnGround);
    CNetworkVar(bool, m_bAllowingJump);
    CNetworkVar(bool, m_bDisableGravity);
    CNetworkVar(bool, m_bFixUpsideSlope);
};

class CTriggerReverseSpeed : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerReverseSpeed, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void Think(void) OVERRIDE;
    void OnEndTouch(CBaseEntity *pOther) OVERRIDE;

  public:
    bool m_bReverseHorizontalSpeed, m_bReverseVerticalSpeed;
    float m_flInterval;
    bool m_bOnThink, m_bShouldThink;
    Vector vecCalculatedVel;
};

class CTriggerSetSpeed : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerSetSpeed, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void Think(void) OVERRIDE;
    void OnEndTouch(CBaseEntity *pOther) OVERRIDE;

  public:
    float m_flHorizontalSpeedAmount, m_flVerticalSpeedAmount;
    QAngle m_angWishDirection;
    bool m_bKeepHorizontalSpeed, m_bKeepVerticalSpeed;
    float m_flInterval;
    bool m_bOnThink, m_bShouldThink;
    Vector vecCalculatedVel;
};

class CTriggerSpeedThreshold : public CBaseMomentumTrigger
{
    enum
    {
        TRIGGERSPEEDTHRESHOLD_ABOVE,
        TRIGGERSPEEDTHRESHOLD_BELOW
    };

    DECLARE_CLASS(CTriggerSpeedThreshold, CBaseMomentumTrigger);
    DECLARE_DATADESC();

  public:
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void CheckSpeed(CMomentumPlayer *pPlayer);
    void Think() OVERRIDE;
    void OnEndTouch(CBaseEntity *pOther) OVERRIDE;

  private:
    int m_iAboveOrBelow;
    bool m_bHorizontal, m_bVertical;
    float m_flHorizontalSpeed;
    float m_flVerticalSpeed;
    bool m_bOnThink;
    float m_flInterval;
    bool m_bShouldThink;
    COutputEvent m_OnThresholdEvent;
};

class CFuncMomentumBrush : public CFuncBrush
{
public:
    DECLARE_CLASS(CFuncMomentumBrush, CFuncBrush);
    DECLARE_DATADESC();

    CFuncMomentumBrush();

    void Spawn() OVERRIDE;

    bool IsOn() const OVERRIDE;
    void TurnOn() OVERRIDE;
    void TurnOff() OVERRIDE;

    void StartTouch(CBaseEntity* pOther) OVERRIDE;
    void EndTouch(CBaseEntity* pOther) OVERRIDE;

    int m_iStage;
    int m_iWorld;
    bool m_bInverted;
    bool m_bDisableUI;
    byte m_iDisabledAlpha;
};

class CFilterMomentumProgress : public CBaseFilter
{
public:
    DECLARE_CLASS(CFilterMomentumProgress, CBaseFilter);
    DECLARE_DATADESC();

    CFilterMomentumProgress();

protected:
    bool PassesFilterImpl(CBaseEntity* pCaller, CBaseEntity* pEntity) OVERRIDE;

private:
    int m_iWorld, m_iStage;
};

class CTriggerCampaignChangelevel : public CBaseMomentumTrigger
{
public:
    DECLARE_CLASS(CTriggerCampaignChangelevel, CBaseMomentumTrigger);
    DECLARE_DATADESC();

    CTriggerCampaignChangelevel();

protected:
    void OnStartTouch(CBaseEntity* pOther) OVERRIDE;

private:
    int m_iWorld, m_iStage, m_iGametype;
    string_t m_MapOverride;
};

class CMomentumMapInfo : public CPointEntity
{
public:
    DECLARE_CLASS(CMomentumMapInfo, CPointEntity);
    DECLARE_DATADESC();

    CMomentumMapInfo();

protected:
    void Spawn() OVERRIDE;

private:
    int m_iWorld, m_iStage, m_iGametype;
    string_t m_MapAuthor;
};