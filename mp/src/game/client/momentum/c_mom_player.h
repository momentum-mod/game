#pragma once

#include "c_mom_triggers.h"
#include "run/mom_run_entity.h"

class C_MomentumOnlineGhostEntity;
class C_MomentumReplayGhostEntity;

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

    bool HasAutoBhop() { return m_bAutoBhop; }
    // void ResetStrafeSync();

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

    CNetworkArray(int, m_iZoneCount, MAX_TRACKS); // The number of zones for a given track
    CNetworkArray(bool, m_iLinearTracks, MAX_TRACKS); // If a given track is linear or not

    float m_fDuckTimer;
    int m_afButtonDisabled;
    CNetworkVar(bool, m_bAutoBhop);

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

    // Last collision
    void SetLastCollision(const trace_t &tr);
    int GetLastCollisionTick() const { return m_iLastCollisionTick; }
    trace_t& GetLastCollisionTrace() { return m_trLastCollisionTrace; }
  private:
    // Ladder stuff
    float m_flGrabbableLadderTime;

    // Last collision
    int m_iLastCollisionTick; // Tick at which the player last collided with a non-vertical surface
    trace_t m_trLastCollisionTrace; // (startpos and endpos raised up to player head for ceilings)

    float m_flStamina;

    CMomRunEntity *m_pSpecTarget;

    friend class CMomentumGameMovement;
};