#pragma once

#include "mom_ghostdefs.h"
#include "mom_shareddefs.h"
#include "GameEventListener.h"
#include "run/mom_run_entity.h"

class CTriggerOnehop;
class CTriggerProgress;
class CTriggerSlide;
class CMomentumGhostBaseEntity;

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

    static CMomentumPlayer *CreatePlayer(const char *className, edict_t *ed)
    {
        s_PlayerEdict = ed;
        return static_cast<CMomentumPlayer *>(CreateEntityByName(className));
    }

    int FlashlightIsOn() OVERRIDE { return IsEffectActive(EF_DIMLIGHT); }
    void FlashlightTurnOn() OVERRIDE;
    void FlashlightTurnOff() OVERRIDE;

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

    void SetDisableBhop(bool bState);
    void EnableAutoBhop();
    void DisableAutoBhop();
    bool HasAutoBhop() const { return m_bAutoBhop; }
    bool DidPlayerBhop() const { return m_bDidPlayerBhop; }
    // think function for detecting if player bhopped
    void UpdateRunStats();
    void UpdateRunSync();
    void UpdateJumpStrafes();
    void UpdateMaxVelocity();
    // slows down the player in a tween-y fashion
    void TweenSlowdownPlayer();
    void ResetRunStats();
    void CalculateAverageStats();

    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_afButtonDisabled);
    CNetworkHandle(CTriggerSlide, m_CurrentSlideTrigger);

    CUtlVector<CTriggerSlide*> m_vecSlideTriggers;

    // Run entity stuff
    virtual RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_PLAYER; }
    virtual void ToggleButtons(int iButtonFlags, bool bEnable) OVERRIDE;
    virtual void ToggleBhop(bool bEnable) OVERRIDE;
    virtual void OnZoneEnter(CTriggerZone* pTrigger, CBaseEntity *pEnt) OVERRIDE;
    virtual void OnZoneExit(CTriggerZone* pTrigger, CBaseEntity *pEnt) OVERRIDE;
    CNetworkVarEmbedded(CMomRunEntityData, m_Data);
    virtual CMomRunEntityData *GetRunEntData() OVERRIDE { return &m_Data;}
    CNetworkVarEmbedded(CMomRunStats, m_RunStats);
    virtual CMomRunStats *GetRunStats() OVERRIDE { return &m_RunStats; }

    CNetworkVar(bool, m_bHasPracticeMode); // Does the player have practice mode enabled?
    CNetworkVar(bool, m_bPreventPlayerBhop); // Used by trigger_limitmovement's BHOP flag
    CNetworkVar(int, m_iLandTick); // Tick at which the player landed on the ground
    CNetworkVar(bool, m_bResumeZoom); // Used by various weapon code
    CNetworkVar(int, m_iShotsFired); // Used in various weapon code
    CNetworkVar(int, m_iDirection); // Used in kickback effects for player
    CNetworkVar(int, m_iLastZoomFOV); // Last FOV when zooming

    bool m_bDidPlayerBhop; // MOM_TODO needs networking?
    bool m_bShouldLimitPlayerSpeed;
    bool m_bTimerStartOnJump; // The timer should start or not while jumping?
    int m_iLimitSpeedType;    // Limit speed only when touching ground?
    int m_iSuccessiveBhops;   // How many successive bhops this player has
    CNetworkVar(bool, m_bAutoBhop); // Is the player using auto bhop?

    Vector m_vecLastPos;      // Saved location before the replay was played or practice mode.
    QAngle m_angLastAng;      // Saved angles before the replay was played or practice mode.
    Vector m_vecLastVelocity; // Saved velocity before the replay was played or practice mode.
    float m_fLastViewOffset;  // Saved viewoffset before the replay was played or practice mode.

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

    bool IsValidObserverTarget(CBaseEntity *target) OVERRIDE;
    bool SetObserverTarget(CBaseEntity *target) OVERRIDE;
    CBaseEntity *FindNextObserverTarget(bool bReverse) OVERRIDE;
    int GetNextObserverSearchStartPoint(bool bReverse) OVERRIDE;
    void CheckObserverSettings() OVERRIDE;
    void ValidateCurrentObserverTarget() OVERRIDE;
    void TravelSpectateTargets(bool bReverse); // spec_next and spec_prev pass into here

    void StopSpectating();

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
    ghostAppearance_t m_playerAppearanceProps;
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

    void SetCurrentProgressTrigger(CTriggerProgress *pCheckpoint) { m_CurrentProgress.Set(pCheckpoint); }
    CTriggerProgress *GetCurrentProgressTrigger() const { return m_CurrentProgress.Get(); }

    void DoMuzzleFlash() OVERRIDE;
    void PostThink() OVERRIDE;

    // Ladder stuff
    float GetGrabbableLadderTime() const { return m_flGrabbableLadderTime; }
    void SetGrabbableLadderTime(float new_time) { m_flGrabbableLadderTime = new_time; }

    void SetLastEyeAngles(const QAngle &ang) { m_qangLastAngle = ang; }
    const QAngle &LastEyeAngles() const { return m_qangLastAngle; }

    void SetIsInAirDueToJump(bool val) { m_bInAirDueToJump = val; }
    bool IsInAirDueToJump() const { return m_bInAirDueToJump; }
  private:
    // Spawn stuff
    bool SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pSpot);

  private:
    CSteamID m_sSpecTargetSteamID;

    bool m_bInAirDueToJump;

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
    CHandle<CTriggerProgress> m_CurrentProgress;

    // for detecting bhop
    friend class CMomentumGameMovement;
    float m_flPunishTime;
    int m_iLastBlock;

    // for strafe sync
    float m_flLastVelocity;

    bool m_bPrevTimerRunning;
    int m_nPrevButtons;

    char m_pszDefaultEntName[128];

    // Used by momentum triggers
    Vector m_vecPreviousOrigins[MAX_PREVIOUS_ORIGINS];

    float m_flTweenVelValue;
    // Trail pointer
    CBaseEntity* m_eTrail;
    bool m_bWasInAir;
    bool m_bShouldLimitSpeed;
};