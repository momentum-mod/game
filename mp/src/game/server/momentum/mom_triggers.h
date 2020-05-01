#pragma once

#include "filters.h"
#include "func_break.h"
#include "modelentities.h"
#include "triggers.h"

class CMomRunEntity;
class CMomentumPlayer;

// spawnflags
enum
{
    // starts on 0x1000 (1 << 12) - SF_TRIGGER_DISALLOW_BOTS
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

enum SPEED_LIMIT_TYPE
{
    SPEED_NORMAL_LIMIT,
    SPEED_LIMIT_INAIR,
    SPEED_LIMIT_GROUND,
    SPEED_LIMIT_ONLAND,
};

// Convenience enum for quick checks of track type
enum TRACK_TYPE
{
    TRACK_ALL = -1, // Applies to all tracks, useful for re-using a trigger in multiple bonuses
    TRACK_MAIN      // Applies to the main map
};

// CBaseMomentumTrigger
class CBaseMomentumTrigger : public CBaseTrigger
{
public:
    DECLARE_CLASS(CBaseMomentumTrigger, CBaseTrigger);
    DECLARE_DATADESC();

    CBaseMomentumTrigger();

    void Spawn() OVERRIDE;
    // Used to calculate if a position is inside of this trigger's bounds
    bool ContainsPosition(const Vector &pos) { return CollisionProp()->IsPointInBounds(pos); }

    // By default we want to filter out momentum entities that do not pass an inherit track number check.
    virtual bool PassesTriggerFilters(CBaseEntity* pOther) OVERRIDE;

    // Returns this trigger's track number.
    int GetTrackNumber() const { return m_iTrackNumber; }
    void SetTrackNumber(int track) { m_iTrackNumber = track; }

    // Track number signifies what part of the map this trigger is for. 
    // 0 = main map, > 0 is the bonus number (example: 2 = "Bonus 2"), and 
    // -1 (default) means it applies to all possible overall zones in the map (good for re-using a trigger)
    CNetworkVarForDerived(int, m_iTrackNumber);
};

// A filter to retrofit older, non-momentum triggers, to be able to filter by track number.
// This filter checks that the entity (player/ghost/etc) has their current track number as the same as the one set here.
class CFilterTrackNumber : public CBaseFilter
{
public:
    DECLARE_CLASS(CFilterTrackNumber, CBaseFilter);
    DECLARE_DATADESC();

    CFilterTrackNumber();
    bool KeyValue(const char *szKeyName, const char *szValue) OVERRIDE;

    bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity) OVERRIDE;

private:
    int m_iTrackNumber;
};

// Base class for all Zone trigger entities (can be created by zone tools)
class CBaseMomZoneTrigger : public CBaseMomentumTrigger
{
public:
    DECLARE_CLASS(CBaseMomZoneTrigger, CBaseMomentumTrigger);
    DECLARE_NETWORKCLASS();

    CBaseMomZoneTrigger();

    // Point-based zones need a custom collision check
    void InitCustomCollision(CPhysCollide *pPhysCollide, const Vector &vecMins, const Vector &vecMaxs);
    virtual bool TestCollision(const Ray_t &ray, unsigned int mask, trace_t &tr) OVERRIDE;

    // Override this function to have the game save this zone type to the .zon file
    // If you override this make sure to also override LoadFromKeyValues to load values from .zon file
    // Returns false by default to signify it was not saved (kvInto can be deleted)
    virtual bool ToKeyValues(KeyValues *pKvInto);
    // Override this function to load zone specific values from .zon file
    // Return true to signify success
    virtual bool LoadFromKeyValues(KeyValues *pKvFrom);

    virtual int GetZoneType();

    const Vector& GetRestartPosition();

    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_iTrackNumber);

    CNetworkVar(float, m_flZoneHeight);
    // Point-based zone editing
    CUtlVector<Vector> m_vecZonePoints;


private:
    friend class CMomPointZoneBuilder;

    bool FindStandableGroundBelow(const Vector& traceStartPos, Vector& dropPos);

    Vector m_vecRestartPos;
};

// A zone trigger has a signifying "zone number" used to give the player
// a sense of progress. Stages (and the Start Trigger) and Checkpoints both extend from this. 
// The "zone number" is paired with the track number, allowing different tracks
// to have different zone triggers with either similar or different zone numbers.
class CTriggerZone : public CBaseMomZoneTrigger
{
public:
    DECLARE_CLASS(CTriggerZone, CBaseMomZoneTrigger);
    DECLARE_DATADESC();

    CTriggerZone();

    void SetZoneNumber(int newZone) { m_iZoneNumber = newZone; }
    int GetZoneNumber() const { return m_iZoneNumber; };

    virtual void OnStartTouch(CBaseEntity* pOther) OVERRIDE;
    virtual void OnEndTouch(CBaseEntity* pOther) OVERRIDE;

    virtual bool ToKeyValues(KeyValues* pKvInto) OVERRIDE;
    virtual bool LoadFromKeyValues(KeyValues* kv) OVERRIDE;

protected:
    // The zone number of this zone. Keep in mind start timer triggers are always zone number 1,
    // while end triggers are typically zone 0. Everything else is meant to be a checkpoint/stage number.
    // See the start trigger for more info. Zone numbers are otherwise 0 by default.
    int m_iZoneNumber;
};

// Checkpoint triggers are the triggers to use for linear maps -- ones that can denote progress on a linear map,
// but if the player fails at any point, they are teleported to the start trigger, not inside of this trigger.
// These triggers are usually used for marking progress inside of linear maps, to allow comparisons
// against runs at certain parts of a linear map.
class CTriggerCheckpoint : public CTriggerZone
{
public:
    DECLARE_CLASS(CTriggerCheckpoint, CTriggerZone);
    DECLARE_NETWORKCLASS();

    // always send to all clients
    int UpdateTransmitState() OVERRIDE { return SetTransmitState(FL_EDICT_ALWAYS); }

    virtual int GetZoneType() OVERRIDE;
};

// Stage triggers are used to denote large, strung-together "chunks" of the map, usually 
// separated by teleports, each with an increasing number. Failing a stage is usually less punishing 
// than failing in a linear map, as it typically teleports the player back to the stage start (this trigger), 
// rather than the start of the entire map.
// An important NOTE: the start trigger reserves the stage (zone) number of "1", but does not make a map "Staged".
// Adding extra stages (starting with zone number 2 and going up) will then make a map's layout "Staged".
// Another important NOTE: do not mix Checkpoint and Stage triggers for the same Track!
class CTriggerStage : public CTriggerZone
{
public:
    DECLARE_CLASS(CTriggerStage, CTriggerZone);
    DECLARE_NETWORKCLASS();

    // always send to all clients
    int UpdateTransmitState() OVERRIDE { return SetTransmitState(FL_EDICT_ALWAYS); }

    virtual int GetZoneType() OVERRIDE;
};

// The start trigger is the first zone trigger for a track.
// You may pair a start trigger with checkpoint or stage triggers -- either way, the zone triggers after the start
// MUST start at zone number 2. The logic behind this is, the start trigger -> next zone trigger is considered
// "Stage/Checkpoint 1" for comparisons.
class CTriggerTimerStart : public CTriggerZone
{
  public:
    DECLARE_CLASS(CTriggerTimerStart, CTriggerZone);
    DECLARE_NETWORKCLASS();
    DECLARE_DATADESC();

    CTriggerTimerStart();

    // always send to all clients
    int UpdateTransmitState() OVERRIDE { return SetTransmitState(FL_EDICT_ALWAYS); }

    void Spawn() OVERRIDE;

    float GetSpeedLimit() const { return m_fSpeedLimit; }
    void SetSpeedLimit(const float maxLeaveSpeed);
    void SetLookAngles(const QAngle &newang);
    const QAngle &GetLookAngles() const { return m_angLook; }

    // spawnflags
    bool IsLimitingSpeed() const { return HasSpawnFlags(SF_LIMIT_LEAVE_SPEED); }
    void SetIsLimitingSpeed(const bool pIsLimitingSpeed);
    void SetHasLookAngles(const bool bHasLook);
    bool HasLookAngles() const { return HasSpawnFlags(SF_USE_LOOKANGLES); }
    bool StartOnJump() const { return m_bTimerStartOnJump; }
    void SetStartOnJump(const bool bStartOnJump) { m_bTimerStartOnJump = bStartOnJump; }
    int GetLimitSpeedType() const { return m_iLimitSpeedType; }
    void SetLimitSpeedType(const int type) { m_iLimitSpeedType = type; }

    virtual bool ToKeyValues(KeyValues *pKvInto) OVERRIDE;
    virtual bool LoadFromKeyValues(KeyValues *zoneKV) OVERRIDE;

    int GetZoneType() OVERRIDE;
  private:
    QAngle m_angLook;

    // How fast can player leave start trigger?
    float m_fSpeedLimit;
    // This might be needed in case for some maps where the start zone is above a descent, when there is a wall in
    // front: ref bhop_w1s1
    bool m_bTimerStartOnJump;
    int m_iLimitSpeedType;
};

// The stop trigger is not considered a traditional CTriggerZone, due to it not really segmenting the map or showing progress,
// but rather, ending the run. It is technically an extension of the last Stage/Checkpoint of the Track.
// For handling entering this "zone" in events (UI), we therefore just hard-code the zone number to 0.
class CTriggerTimerStop : public CTriggerZone
{
public:
    DECLARE_CLASS(CTriggerTimerStop, CTriggerZone);
    DECLARE_NETWORKCLASS();

    virtual void Spawn() OVERRIDE;

    // always send to all clients
    virtual int UpdateTransmitState() OVERRIDE { return SetTransmitState(FL_EDICT_ALWAYS); }

    int GetZoneType() OVERRIDE;
};

// Our teleport trigger override with extra convenience features
class CTriggerMomentumTeleport : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerMomentumTeleport, CBaseMomentumTrigger);
    DECLARE_DATADESC();

public:
    // This void teleports the touching entity!
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
    // Used by children classes to define what ent to teleport to (see CTriggerOneHop)
    void SetDestinationEnt(CBaseEntity *ent) { m_hDestinationEnt.Set(ent); }
    bool ShouldStopPlayer() const { return m_bResetVelocity; }
    bool ShouldResetAngles() const { return m_bResetAngles; }
    void SetShouldStopPlayer(const bool newB) { m_bResetVelocity = newB; }
    void SetShouldResetAngles(const bool newB) { m_bResetAngles = newB; }

    // Default teleport method, uses the set destination target, if there is one.
    void HandleTeleport(CBaseEntity *pOther);
    // Actual teleport method where the pEntToTeleport is teleported to pTeleportTo
    // True if the entity was teleported, else false
    virtual bool DoTeleport(CBaseEntity *pTeleportTo, CBaseEntity *pEntToTeleport);
    // After teleporting, do this code. Base class does nothing.
    virtual void AfterTeleport(CBaseEntity *pEntTeleported) {};

private:
    bool m_bResetVelocity;
    bool m_bResetAngles;
    EHANDLE m_hDestinationEnt;
};

// CTriggerProgress, used by mappers for teleporting
class CTriggerProgress : public CBaseMomentumTrigger
{
    DECLARE_CLASS(CTriggerProgress, CBaseMomentumTrigger);
    DECLARE_DATADESC();

public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    // the following is only used by CFilterCheckpoint
    virtual int GetProgressNumber() const { return m_iProgressNumber; }
    // The following is used by mapzones.cpp
    void SetProgressNumber(int newInt) { m_iProgressNumber = newInt; }

private:
    int m_iProgressNumber;
    // Fires when it resets all one hops.
    COutputEvent m_ResetOnehops;
};

// A filter that can be applied to check if a player has hit a certain progress
// trigger. The check is >= m_iProgressNumber.
class CFilterProgress : public CBaseFilter
{
    DECLARE_CLASS(CFilterProgress, CBaseFilter);
    DECLARE_DATADESC();

  public:
    bool PassesFilterImpl(CBaseEntity *, CBaseEntity *) OVERRIDE;

  private:
    int m_iProgressNum;
};

// A teleport trigger to teleport the player to their last known touched progress
// trigger.
class CTriggerTeleportProgress : public CTriggerMomentumTeleport
{
    DECLARE_CLASS(CTriggerTeleportProgress, CTriggerMomentumTeleport);

  public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;
};

// A trigger volume that allows multiple repeated entries (hops), but teleports the player
// if they stay inside the trigger for a given period ("hold" time).
class CTriggerMultihop : public CTriggerMomentumTeleport
{
public:
    DECLARE_CLASS(CTriggerMultihop, CTriggerMomentumTeleport);
    DECLARE_DATADESC();

    CTriggerMultihop();

    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void OnEndTouch(CBaseEntity *) OVERRIDE;
    void Think() OVERRIDE;

    float GetHoldTeleportTime() const { return m_fMaxHoldSeconds; }
    void SetHoldTeleportTime(const float fHoldTime) { m_fMaxHoldSeconds = fHoldTime; }

protected:
    // The time that the player initially touched the trigger
    CUtlMap<short, float> m_mapOnStartTouchedTimes;
    // Seconds to hold before activating the teleport
    float m_fMaxHoldSeconds;
};

// Onehop triggers are based on multihop but limit the player to only entering it
// once, until the player reaches a stage/start/progress/resetonehop trigger.
// Upon re-entry or staying inside the trigger past the hold time, the player is teleported 
// to a valid progress trigger.
class CTriggerOnehop : public CTriggerMultihop
{
  public:
    DECLARE_CLASS(CTriggerOnehop, CTriggerMultihop);
    DECLARE_DATADESC();

    CTriggerOnehop();

    void OnStartTouch(CBaseEntity *pEntity) OVERRIDE;
    void SetNoLongerJumpableFired(bool bState) { m_bhopNoLongerJumpableFired = bState; }

  private:
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

    void OnStartTouch(CBaseEntity *) OVERRIDE;

  private:
    // Fires when it resets all one hops.
    COutputEvent m_ResetOnehops;
};

// CTriggerUserInput
class CTriggerUserInput : public CBaseMomentumTrigger
{
public:
    DECLARE_CLASS(CTriggerUserInput, CBaseMomentumTrigger);
    DECLARE_DATADESC();

    CTriggerUserInput();
    void Spawn() OVERRIDE;
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void Think() OVERRIDE;

  private:
    void CheckEnt(CBaseEntity *pOther);

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
    int m_ButtonRep;
    Key m_eKey;
    COutputEvent m_OnKeyPressed;
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
        SF_LIMIT_BHOP = 1 << 19,     // prevent player from bhopping
        SF_LIMIT_WALK = 1 << 20,    // prevent player from +walking
        SF_LIMIT_SPRINT = 1 << 21,  // prevent player from +sprinting
    };
private:
    void ToggleButtons(CMomRunEntity *pEnt, bool bEnable);
};

// CFuncShootBoost
class CFuncShootBoost : public CBreakable
{
  public:
    DECLARE_CLASS(CFuncShootBoost, CBreakable);
    DECLARE_DATADESC();

    CFuncShootBoost();

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
    EHANDLE m_hEntityCheck;
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
    // 2: Increase player's current velocity by push final force amount // This is almost like the default trigger_push
    // behaviour
    // 3: Only set the player's velocity to the final push velocity if player's velocity is lower than final push
    // velocity
    int m_iIncrease;
    // Dictates the direction of push
    Vector m_vPushDir;
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
};

class CTriggerReverseSpeed : public CBaseMomentumTrigger
{
public:
    DECLARE_CLASS(CTriggerReverseSpeed, CBaseMomentumTrigger);
    DECLARE_DATADESC();

    CTriggerReverseSpeed();
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void Think() OVERRIDE;

    void ReverseSpeed(CBaseEntity *pEntity, bool bIsHorizontal);

private:
    bool m_bReverseHorizontalSpeed, m_bReverseVerticalSpeed;
    float m_flInterval;
    bool m_bOnThink;
    Vector vecCalculatedVel;
};

class CTriggerSetSpeed : public CBaseMomentumTrigger
{
public:
    DECLARE_CLASS(CTriggerSetSpeed, CBaseMomentumTrigger);
    DECLARE_DATADESC();

    CTriggerSetSpeed();

    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void OnEndTouch(CBaseEntity *pOther) OVERRIDE;
    void Think(void) OVERRIDE;
    void Touch(CBaseEntity *pOther) OVERRIDE;

private:
    void CalculateSpeed(CBaseEntity *pOther);

    CUtlMap<short, Vector> m_mapCalculatedVelocities;
    float m_flHorizontalSpeedAmount, m_flVerticalSpeedAmount;
    QAngle m_angWishDirection;
    bool m_bKeepHorizontalSpeed, m_bKeepVerticalSpeed;
    float m_flInterval;
    bool m_bOnThink, m_bEveryTick;
};

class CTriggerSpeedThreshold : public CBaseMomentumTrigger
{
    enum
    {
        THRESHOLD_ABOVE = 0,
        THRESHOLD_BELOW
    };

  public:
    DECLARE_CLASS(CTriggerSpeedThreshold, CBaseMomentumTrigger);
    DECLARE_DATADESC();

    CTriggerSpeedThreshold();

    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;
    void CheckSpeed(CBaseEntity *pOther);
    void Think() OVERRIDE;

  private:
    bool CheckSpeedInternal(const float flToCheck, bool bIsHorizontal);

    int m_iAboveOrBelow;
    bool m_bHorizontal, m_bVertical;
    float m_flHorizontalSpeed;
    float m_flVerticalSpeed;
    bool m_bOnThink;
    float m_flInterval;
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

    void StartTouch(CBaseEntity *pOther) OVERRIDE;
    void EndTouch(CBaseEntity *pOther) OVERRIDE;

    int m_iStage;
    int m_iWorld;
    bool m_bInverted;
    bool m_bDisableUI;
    byte m_iDisabledAlpha;
};

class CFilterCampaignProgress : public CBaseFilter
{
  public:
    DECLARE_CLASS(CFilterCampaignProgress, CBaseFilter);
    DECLARE_DATADESC();

    CFilterCampaignProgress();

  protected:
    bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity) OVERRIDE;

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
    void OnStartTouch(CBaseEntity *pOther) OVERRIDE;

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

#define NOGRENADE_SPRITE "sprites/light_glow02_noz.vmt"

class CNoGrenadesZone : public CBaseTrigger
{
public:
    DECLARE_CLASS(CNoGrenadesZone, CBaseTrigger);
    DECLARE_DATADESC();

    ~CNoGrenadesZone();

    void Spawn() override;
    void Precache() override;

    void InputDisable(inputdata_t &inputdata) override { m_bDisabled = true; }
    void InputEnable(inputdata_t &inputdata) override { m_bDisabled = false; }
    void InputToggle(inputdata_t &inputdata) override { m_bDisabled = !m_bDisabled; }

    static bool IsInsideNoGrenadesZone(CBaseEntity *pOther);

    bool m_bAirborneOnly;
};


// CTriggerMomentumCatapult
class CTriggerMomentumCatapult : public CBaseMomentumTrigger
{
  public:
    DECLARE_CLASS(CTriggerMomentumCatapult, CBaseMomentumTrigger);
    DECLARE_DATADESC();

    CTriggerMomentumCatapult();

  public:
    void OnStartTouch(CBaseEntity *) OVERRIDE;
    void Spawn() OVERRIDE;

  private:
    Vector CalculateLaunchVelocity(CBaseEntity *);
    Vector CalculateLaunchVelocityExact(CBaseEntity *);
    void LaunchAtDirection(CBaseEntity *);
    void LaunchAtTarget(CBaseEntity *);

  private:

    enum
    {
        BEST=0,
        SOLUTION_ONE,
        SOLUTION_TWO,
    };

    float m_flPlayerSpeed;
    int m_iUseExactVelocity;
    int m_iExactVelocityChoiceType;
    QAngle m_vLaunchDirection;
    EHANDLE m_hLaunchTarget;
    bool m_bUseLaunchTarget;
    bool m_bUseThresholdCheck;
    float m_flLowerThreshold;
    float m_flUpperThreshold;
    bool m_bOnlyCheckVelocity;
    float m_flEntryAngleTolerance;
    COutputEvent m_OnCatapulted;
};