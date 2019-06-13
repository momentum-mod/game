#pragma once

#include "mom_ghostdefs.h"
#include "mom_shareddefs.h"
#include "GameEventListener.h"
#include "run/mom_run_entity.h"

struct SavedLocation_t;
class CBaseMomentumTrigger;
class CTriggerOnehop;
class CTriggerProgress;
class CTriggerSlide;
class CMomentumGhostBaseEntity;

struct SavedState_t
{
    int m_nButtons;           // Saved player buttons being pressed
    Vector m_vecLastPos;      // Saved location before the replay was played or practice mode.
    QAngle m_angLastAng;      // Saved angles before the replay was played or practice mode.
    Vector m_vecLastVelocity; // Saved velocity before the replay was played or practice mode.
    float m_fLastViewOffset;  // Saved viewoffset before the replay was played or practice mode.    
    // Stats-related
    int m_nSavedPerfectSyncTicks;
    int m_nSavedStrafeTicks;
    int m_nSavedAccelTicks;
};

// The player can spend this many ticks in the air inside the start zone before their speed is limited
#define MAX_AIRTIME_TICKS 15
#define NUM_TICKS_TO_BHOP 10 // The number of ticks a player can be on a ground before considered "not bunnyhopping"
#define MAX_PREVIOUS_ORIGINS 3 // The number of previous origins saved

class CMomentumPlayer : public CBasePlayer, public CGameEventListener, public CMomRunEntity
{
  public:
    DECLARE_CLASS(CMomentumPlayer, CBasePlayer);
    DECLARE_SERVERCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_DATADESC();

    CMomentumPlayer();
    ~CMomentumPlayer(void);

    static CMomentumPlayer* CreatePlayer(const char *className, edict_t *ed);
    static CMomentumPlayer* GetLocalPlayer();

    int FlashlightIsOn() OVERRIDE { return IsEffectActive(EF_DIMLIGHT); }
    void FlashlightTurnOn() OVERRIDE;
    void FlashlightTurnOn(bool bEmitSound);
    void FlashlightTurnOff() OVERRIDE;
    void FlashlightTurnOff(bool bEmitSound);

    void SendAppearance();

    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;

    void CreateViewModel(int index = 0) OVERRIDE;
    void PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper) OVERRIDE;
    void SetupVisibility(CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize) OVERRIDE;

    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

    // MOM_TODO: This is called when the player spawns so that HUD elements can be updated
    // void InitHUD() OVERRIDE;

    void CommitSuicide(bool bExplode = false, bool bForce = false) OVERRIDE{};
    void CommitSuicide(const Vector &vecForce, bool bExplode = false, bool bForce = false) OVERRIDE{};
    bool CanBreatheUnderwater() const OVERRIDE { return true; }

    void SetAllowUserTeleports(bool allow) { m_bAllowUserTeleports = allow; }
    bool AllowUserTeleports() const { return m_bAllowUserTeleports; }

    // SPAWNING
    CBaseEntity *EntSelectSpawnPoint() OVERRIDE;

    void EnableAutoBhop();
    void DisableAutoBhop();
    bool HasAutoBhop() const { return m_bAutoBhop; }
    bool DidPlayerBhop() const { return m_bDidPlayerBhop; }
    // think function for detecting if player bhopped
    void UpdateRunStats();
    void UpdateRunSync();
    void UpdateStrafes();
    void UpdateMaxVelocity();
    // slows down the player in a tween-y fashion
    void TweenSlowdownPlayer();
    void ResetRunStats();
    void CalculateAverageStats();

    void LimitSpeed(float flSpeedLimit, bool bSaveZ);

    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_afButtonDisabled);
    CNetworkHandle(CTriggerSlide, m_CurrentSlideTrigger);

    CUtlVector<CTriggerSlide*> m_vecSlideTriggers;

    // Run entity stuff
    virtual RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_PLAYER; }
    virtual void SetButtonsEnabled(int iButtonFlags, bool bEnable) OVERRIDE;
    virtual void SetBhopEnabled(bool bEnable) OVERRIDE;
    virtual bool GetBhopEnabled() const OVERRIDE;
    virtual void OnZoneEnter(CTriggerZone* pTrigger) OVERRIDE;
    virtual void OnZoneExit(CTriggerZone* pTrigger) OVERRIDE;
    CNetworkVarEmbedded(CMomRunEntityData, m_Data);
    virtual CMomRunEntityData *GetRunEntData() OVERRIDE { return &m_Data;}
    CNetworkVarEmbedded(CMomRunStats, m_RunStats);
    virtual CMomRunStats *GetRunStats() OVERRIDE { return &m_RunStats; }
    virtual int GetEntIndex() OVERRIDE { return entindex(); }

    CNetworkVar(bool, m_bHasPracticeMode); // Does the player have practice mode enabled?
    CNetworkVar(bool, m_bPreventPlayerBhop); // Used by trigger_limitmovement's BHOP flag
    CNetworkVar(int, m_iLandTick); // Tick at which the player landed on the ground
    CNetworkVar(bool, m_bResumeZoom); // Used by various weapon code
    CNetworkVar(int, m_iShotsFired); // Used in various weapon code
    CNetworkVar(int, m_iDirection); // Used in kickback effects for player
    CNetworkVar(int, m_iLastZoomFOV); // Last FOV when zooming

    CNetworkArray(int, m_iZoneCount, MAX_TRACKS); // The number of zones for a given track
    CNetworkArray(bool, m_iLinearTracks, MAX_TRACKS); // If a given track is linear or not

    bool m_bDidPlayerBhop; // MOM_TODO needs networking?
    bool m_bShouldLimitPlayerSpeed;
    bool m_bStartTimerOnJump; // Should the timer start when jumping in the start trigger?
    int m_iLimitSpeedType;    // Limit speed only when touching ground?
    int m_iSuccessiveBhops;   // How many successive bhops this player has
    CNetworkVar(bool, m_bAutoBhop); // Is the player using auto bhop?


    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance, bool &bPaint);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, float flDistance, int iPenetration,
                    int iBulletType, int iDamage, float flRangeModifier, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    // Used by g_MOMBlockFix door/button fix code
    void Touch(CBaseEntity *) OVERRIDE;
    int GetLastBlock() const { return m_iLastBlock; }
    void SetLastBlock(int lastBlock) { m_iLastBlock = lastBlock; }
    float GetPunishTime() const { return m_flPunishTime; }
    void SetPunishTime(float newTime) { m_flPunishTime = newTime; }

    // Replay stuff
    
    bool IsSpectatingGhost() const { return m_hObserverTarget.Get() && GetGhostEnt(); }
    CMomentumGhostBaseEntity *GetGhostEnt() const;

    bool StartObserverMode(int mode) OVERRIDE;
    void StopObserverMode() OVERRIDE;
    bool IsValidObserverTarget(CBaseEntity *target) OVERRIDE;
    bool SetObserverTarget(CBaseEntity *target) OVERRIDE;
    CBaseEntity *FindNextObserverTarget(bool bReverse) OVERRIDE;
    int GetNextObserverSearchStartPoint(bool bReverse) OVERRIDE;
    void CheckObserverSettings() OVERRIDE;
    void ValidateCurrentObserverTarget() OVERRIDE;
    void TravelSpectateTargets(bool bReverse); // spec_next and spec_prev pass into here

    void StopSpectating();

    // Practice mode
    void TogglePracticeMode();
    void EnablePracticeMode();
    void DisablePracticeMode();

    // Used when spectating/practicing during a run
    void SaveCurrentRunState(bool bFromPracticeMode); // Entering practice/spectate
    void RestoreRunState(bool bFromPracticeMode); // Exiting practice/spectate

    // Used by momentum triggers
    Vector GetPreviousOrigin(unsigned int previous_count = 0) const;
    void NewPreviousOrigin(Vector origin);

    // for calc avg
    int m_nZoneAvgCount[MAX_ZONES];
    float m_flZoneTotalSync[MAX_ZONES], m_flZoneTotalSync2[MAX_ZONES], m_flZoneTotalVelocity[MAX_ZONES][2];
    
    //Overrode for the spectating GUI and weapon dropping
    bool ClientCommand(const CCommand &args) OVERRIDE;
    void MomentumWeaponDrop(CBaseCombatWeapon *pWeapon);

    void ToggleDuckThisFrame(bool bState);

    int GetPerfectSyncTicks() const { return m_nPerfectSyncTicks; }
    void SetPerfectSyncTicks(int ticks) { m_nPerfectSyncTicks = ticks; }
    int GetStrafeTicks() const { return m_nStrafeTicks; }
    void SetStrafeTicks(int ticks) { m_nStrafeTicks = ticks; }
    int GetAccelTicks() const { return m_nAccelTicks; }
    void SetAccelTicks(int ticks) { m_nAccelTicks = ticks; }

    // Trail Methods
    void Teleport(const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity) OVERRIDE;
    bool KeyValue(const char *szKeyName, const char *szValue) OVERRIDE;
    bool KeyValue(const char *szKeyName, float flValue) OVERRIDE;
    bool KeyValue(const char *szKeyName, const Vector &vecValue) OVERRIDE;
    void CreateTrail();
    void RemoveTrail();
     
    // Player's apperence properties
    GhostAppearance_t m_playerAppearanceProps;
    // Catches any messages the player sends through "say"
    void CheckChatText(char *p, int bufsize) OVERRIDE;

    // Adds the give Onehop to the hopped list.
    // Returns: Its new index.
    void AddOnehop(CTriggerOnehop *pTrigger);
    // Finds a Onehop on the hopped list.
    // Returns: true if found, else false
    bool FindOnehopOnList(CTriggerOnehop *pTrigger) const;
    // Removes all onehops
    void RemoveAllOnehops();

    void SetCurrentProgressTrigger(CBaseMomentumTrigger *pTrigger);
    CBaseMomentumTrigger* GetCurrentProgressTrigger() const;
    int m_iProgressNumber; // Used by actual progress triggers for filtering

    void SetCurrentZoneTrigger(CTriggerZone *pZone) { return m_CurrentZoneTrigger.Set(pZone); }
    CTriggerZone *GetCurrentZoneTrigger() const { return m_CurrentZoneTrigger.Get(); }

    void CreateStartMark();
    SavedLocation_t *GetStartMark(int track) const { return (track >= 0 && track < MAX_TRACKS) ? m_pStartZoneMarks[track] : nullptr; }
    void ClearStartMark(int track);

    void DoMuzzleFlash() OVERRIDE;
    void PreThink() OVERRIDE;
    void PostThink() OVERRIDE;

    // Ladder stuff
    float GetGrabbableLadderTime() const { return m_flGrabbableLadderTime; }
    void SetGrabbableLadderTime(float new_time) { m_flGrabbableLadderTime = new_time; }

    void SetLastEyeAngles(const QAngle &ang) { m_qangLastAngle = ang; }
    const QAngle &LastEyeAngles() const { return m_qangLastAngle; }

    void OnJump();
    void OnLand();

    void SetIsInAirDueToJump(bool val) { m_bInAirDueToJump = val; }
    bool IsInAirDueToJump() const { return m_bInAirDueToJump; }

    SavedState_t *GetSavedRunState() { return &m_SavedRunState; }
  private:
    // Spawn stuff
    bool SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pSpot);
    void SetPracticeModeState();

    CSteamID m_sSpecTargetSteamID;

    bool m_bInAirDueToJump;

    bool m_bWasSpectating; // Was the player spectating and then respawned?

    // Strafe sync.
    int m_nPerfectSyncTicks;
    int m_nStrafeTicks;
    int m_nAccelTicks;
    QAngle m_qangLastAngle;

    // used by CMomentumGameMovement
    bool m_duckUntilOnGround;
    float m_flStamina;

    bool m_bAllowUserTeleports;

    // Ladder stuff
    float m_flGrabbableLadderTime;

    // Trigger stuff
    CUtlVector<CTriggerOnehop*> m_vecOnehops;
    CHandle<CBaseMomentumTrigger> m_CurrentProgress;
    CHandle<CTriggerZone> m_CurrentZoneTrigger;

    SavedLocation_t *m_pStartZoneMarks[MAX_TRACKS];

    // for detecting bhop
    friend class CMomentumGameMovement;
    float m_flPunishTime;
    int m_iLastBlock;

    // for strafe sync
    float m_flLastVelocity;

    int m_nPrevButtons;

    char m_pszDefaultEntName[128];

    // Used by momentum triggers
    Vector m_vecPreviousOrigins[MAX_PREVIOUS_ORIGINS];

    float m_flTweenVelValue;
    // Trail pointer
    CBaseEntity* m_eTrail;
    bool m_bWasInAir;
    bool m_bShouldLimitSpeed;

    // Saved states
    int m_iOldTrack, m_iOldZone; // Previous zones before spectating/practice mode/entering endzone

    SavedState_t m_SavedRunState; // Used when either entering practice mode or spectating while in a run
    SavedState_t m_PracticeModeState; // Only used when the path is (in a run) -> (enters Practice) -> (spectates)
};