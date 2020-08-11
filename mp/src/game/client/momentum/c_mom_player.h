#pragma once

#include "c_mom_triggers.h"
#include "run/mom_run_entity.h"

class C_MomentumOnlineGhostEntity;
class C_MomentumReplayGhostEntity;

struct SurfaceInteraction
{
    enum Type
    {
        TYPE_LEAVE = 0,
        TYPE_FLOOR,
        TYPE_WALL,
        TYPE_CEILING,
        TYPE_LAND,
        TYPE_GROUNDED,
        TYPE_COUNT
    };

    enum Action
    {
        ACTION_LEAVE = 0,
        ACTION_KNOCKBACK,
        ACTION_WALK,
        ACTION_DUCKWALK,
        ACTION_JUMP,
        ACTION_DUCKJUMP,
        ACTION_CTAP,
        ACTION_COLLISION,
        ACTION_SLIDE,
        ACTION_EDGEBUG,
        ACTION_LAND,
        ACTION_GROUNDED,
    };

    int tick;
    trace_t trace;
    Vector velocity;
    Action action;
};
typedef SurfaceInteraction SurfInt;

class C_MomentumPlayer : public C_BasePlayer, public CMomRunEntity
{
public:
    DECLARE_CLASS(C_MomentumPlayer, C_BasePlayer);
    DECLARE_CLIENTCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_INTERPOLATION();

    C_MomentumPlayer();
    ~C_MomentumPlayer();

    static C_MomentumPlayer *GetLocalMomPlayer();

    // Handles determining if we should be showing the entity we're spectating or ourselves, given the situation.
    // Guaranteed to not be null, as at worst case, it'll return this player.
    CMomRunEntity *GetCurrentUIEntity();
    CMomRunEntityData *GetCurrentUIEntData(); // Same as above, but conveniently gets the run ent data pointer
    CMomRunStats *GetCurrentUIEntStats(); // Same as above but for run stats

    void PostDataUpdate(DataUpdateType_t updateType) OVERRIDE;
    void OnDataChanged(DataUpdateType_t type) OVERRIDE;
    bool CreateMove(float flInputSampleTime, CUserCmd *pCmd) OVERRIDE;

    bool HasAutoBhop() const { return m_bAutoBhop; }
    // void ResetStrafeSync();

    // Ramp stuff
    void SetRampBoardVelocity(const Vector &vecVel);
    void SetRampLeaveVelocity(const Vector &vecVel);

    // Returns the replay entity that the player is watching (first person only)
    int GetSpecEntIndex() const;

    // Overridden for ghost spectating
    Vector GetChaseCamViewOffset(CBaseEntity *target) OVERRIDE;

    void OnObserverTargetUpdated() OVERRIDE;

    CNetworkVar(bool, m_bHasPracticeMode); // Does the player have practice mode enabled?
    CNetworkVar(bool, m_bPreventPlayerBhop); // Used by trigger_limitmovement's BHOP flag
    CNetworkVar(int, m_iLandTick); // Tick at which the player landed on the ground
    CNetworkVar(bool, m_bResumeZoom); // Used by various weapon code
    CNetworkVar(int, m_iShotsFired); // Used in various weapon code
    CNetworkVar(int, m_iDirection); // Used in kickback effects for player
    CNetworkVar(int, m_iLastZoomFOV); // Last FOV when zooming
    CNetworkVar(bool, m_bSurfing);
    CNetworkVector(m_vecRampBoardVel);
    CNetworkVector(m_vecRampLeaveVel);

    CNetworkArray(int, m_iZoneCount, MAX_TRACKS); // The number of zones for a given track
    CNetworkArray(bool, m_iLinearTracks, MAX_TRACKS); // If a given track is linear or not

    float m_fDuckTimer;
    int m_afButtonDisabled;
    int m_iJumpTick;
    CNetworkVar(bool, m_bAutoBhop);
    bool m_bIsSprinting, m_bIsWalking;

    // CMomRunEnt stuff
    RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_PLAYER; }
    CNetworkVarEmbedded(CMomRunEntityData, m_Data);
    virtual CMomRunEntityData *GetRunEntData() OVERRIDE { return &m_Data; }
    CNetworkVarEmbedded(CMomRunStats, m_RunStats);
    virtual CMomRunStats *GetRunStats() OVERRIDE { return &m_RunStats; };
    virtual int GetEntIndex() OVERRIDE { return m_index; }
    virtual float GetCurrentRunTime() OVERRIDE;
    uint64 GetSteamID() override;

    CNetworkHandle(C_TriggerSlide, m_CurrentSlideTrigger);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, int iBulletType, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    float m_flStartSpeed;
    float m_flEndSpeed;

    // Ladder stuff
    float GetGrabbableLadderTime() const { return m_flGrabbableLadderTime; }
    void SetGrabbableLadderTime(float new_time) { m_flGrabbableLadderTime = new_time; }

    // Surface interactions
    int GetInteractionIndex(SurfInt::Type type) const;
    const SurfInt& GetInteraction(int index) const;
    bool SetLastInteraction(const trace_t &tr, const Vector &velocity, SurfInt::Type type);
    void UpdateLastAction(SurfInt::Action action);

    // Mobility sound functions
    void PlayStepSound(const Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force) override;
    virtual void PlayAirjumpSound(const Vector &vecOrigin);
    virtual void PlayPowerSlideSound(const Vector &vecOrigin);
    virtual void StopPowerSlideSound();
    virtual void PlayWallRunSound(const Vector &vecOrigin);
    virtual void StopWallRunSound();

    Vector GetEscapeVel() const { return m_vecCornerEscapeVel; }
    void SetEscapeVel(const Vector &vecNewYaw) { m_vecCornerEscapeVel = vecNewYaw; }

private:
    // Mobility mod (parkour)
    bool m_bWasSprinting;
    bool m_bIsPowerSliding;
    WallRunState m_nWallRunState;
    Vector m_vecWallNorm;
    float m_flAutoViewTime; // if wallrunning, when should start adjusting the view 
    bool m_bWallRunBumpAhead; // are we moving out from the wall anticipating a bump?
    Vector m_vecLastWallRunPos; // Position when we ended the last wallrun
    AirJumpState m_nAirJumpState; // Is the airjump ready, in progress, or done?
    // Is the player allowed to jump while in the air
    bool CanAirJump() const
    {
        return m_nAirJumpState != AIRJUMP_DONE &&
            m_nAirJumpState != AIRJUMP_NORM_JUMPING;
    }
    HSOUNDSCRIPTHANDLE m_hssPowerSlideSound;
    HSOUNDSCRIPTHANDLE m_hssWallRunSound;

    // When a wallrun ends or we go over a cliff, allow a window when
    // jumping counts as a normal jump off the ground/wall, even though
    // technically airborn. Compensating for player's perception/reflexes.
    // This is the absolute time until which we allow the special jump
    float m_flCoyoteTime; 

    // Sometimes we want to have a little cooldown for wallrunning - 
    // mostly if a wallrun ended because it was above a doorway
    float m_flNextWallRunTime;
    Vector m_vecCornerEscapeVel;

    // Ladder stuff
    float m_flGrabbableLadderTime;

    // List of airborne surface interations and start and end ground interactions
    SurfInt m_surfIntList[SurfInt::TYPE_COUNT]; // Stores interactions by type
    SurfInt::Type m_surfIntHistory[SurfInt::TYPE_COUNT]; // Keeps track of the history of interactions

    float m_flStamina;

    CMomRunEntity *m_pSpecTarget;

    friend class CMomentumGameMovement;
};