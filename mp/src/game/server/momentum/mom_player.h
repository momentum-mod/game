#ifndef MOMPLAYER_H
#define MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_ghostdefs.h"
#include "mom_shareddefs.h"
#include "GameEventListener.h"
#include "mom_modulecomms.h"
#include "IMovementListener.h"

class CTriggerOnehop;
class CTriggerCheckpoint; // MOM_TODO: Will change with the linear map support

// The player can spend this many ticks in the air inside the start zone before their speed is limited
#define MAX_AIRTIME_TICKS 15
#define NUM_TICKS_TO_BHOP 10 // The number of ticks a player can be on a ground before considered "not bunnyhopping"
#define MAX_PREVIOUS_ORIGINS 3 // The number of previous origins saved

class CMomentumGhostBaseEntity;

class CMomentumPlayer : public CBasePlayer, public CGameEventListener, public IMovementListener
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

    void FlashlightTurnOn() OVERRIDE
    {
        AddEffects(EF_DIMLIGHT);
        EmitSound(SND_FLASHLIGHT_ON);
        m_playerAppearanceProps.FlashlightOn = true;
        SendAppearance();
    }

    void FlashlightTurnOff() OVERRIDE
    {
        RemoveEffects(EF_DIMLIGHT);
        EmitSound(SND_FLASHLIGHT_OFF);
        m_playerAppearanceProps.FlashlightOn = false;
        SendAppearance();
    }

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

    // LADDERS
    void SurpressLadderChecks(const Vector &pos, const Vector &normal);
    bool CanGrabLadder(const Vector &pos, const Vector &normal);
    Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

    // SPAWNING
    CBaseEntity *EntSelectSpawnPoint() OVERRIDE;

    // used by CMomentumGameMovement
    bool m_duckUntilOnGround;
    float m_flStamina;

    bool m_bAllowUserTeleports;

    void EnableAutoBhop();
    void DisableAutoBhop();
    bool HasAutoBhop() const { return m_SrvData.m_RunData.m_bAutoBhop; }
    bool DidPlayerBhop() const { return m_SrvData.m_bDidPlayerBhop; }
    // think function for detecting if player bhopped
    void OnPlayerJump() OVERRIDE;
    void OnPlayerLand() OVERRIDE;
    void UpdateRunStats();
    void UpdateRunSync();
    void UpdateJumpStrafes();
    void UpdateMaxVelocity();
    // slows down the player in a tween-y fashion
    void TweenSlowdownPlayer();
    void ResetRunStats();
    void CalculateAverageStats();
    void LimitSpeedInStartZone();

    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_afButtonDisabled);

    StdDataFromServer m_SrvData;
    CMomRunStats m_RunStats;
    //Function pointer to transfer regularly "networked" variables to client.
    //Pointer is acquired in mom_client.cpp
    void (*StdDataToPlayer)(StdDataFromServer* from);

    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance, bool &bPaint);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, float flDistance, int iPenetration,
                    int iBulletType, int iDamage, float flRangeModifier, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    // Used by g_MOMBlockFix door/button fix code
    void Touch(CBaseEntity *) OVERRIDE;
    int GetLastBlock() const { return m_iLastBlock; }
    float GetPunishTime() const { return m_flPunishTime; }
    void SetPunishTime(float newTime) { m_flPunishTime = newTime; }
    void SetLastBlock(int lastBlock) { m_iLastBlock = lastBlock; }

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
    int m_nZoneAvgCount[MAX_STAGES];
    float m_flZoneTotalSync[MAX_STAGES], m_flZoneTotalSync2[MAX_STAGES], m_flZoneTotalVelocity[MAX_STAGES][2];
    
    //Overrode for the spectating GUI and weapon dropping
    bool ClientCommand(const CCommand &args) OVERRIDE;
    void MomentumWeaponDrop(CBaseCombatWeapon *pWeapon);

    void ToggleDuckThisFrame(bool bState);

    int &GetPerfectSyncTicks() { return m_nPerfectSyncTicks; }
    int &GetStrafeTicks() { return m_nStrafeTicks; }
    int &GetAccelTicks() { return m_nAccelTicks; }

    // Trail Methods
    void Teleport(const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity) OVERRIDE;
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

    void SetCurrentCheckpointTrigger(CTriggerCheckpoint *pCheckpoint) { m_pCurrentCheckpoint = pCheckpoint; }
    CTriggerCheckpoint *GetCurrentCheckpointTrigger() const { return m_pCurrentCheckpoint; }

    CSteamID m_sSpecTargetSteamID;

    bool m_bInAirDueToJump;

    void DoMuzzleFlash() OVERRIDE;
    void PostThink() OVERRIDE;

    // Ladder stuff
    float GetGrabbableLadderTime() const { return m_flGrabbableLadderTime; }
    void SetGrabbableLadderTime(float new_time) { m_flGrabbableLadderTime = new_time; }
  private:
    // Ladder stuff
    CountdownTimer m_ladderSurpressionTimer;
    Vector m_lastLadderNormal;
    Vector m_lastLadderPos;
    float m_flGrabbableLadderTime;

    // Spawn stuff
    EHANDLE g_pLastSpawn;
    bool SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pSpot);

    // Trigger stuff
    CUtlVector<CTriggerOnehop*> m_vecOnehops;
    CTriggerCheckpoint *m_pCurrentCheckpoint;

    // for detecting bhop
    friend class CMomentumGameMovement;
    float m_flPunishTime;
    int m_iLastBlock;

    // for strafe sync
    float m_flLastVelocity;
    QAngle m_qangLastAngle;
    int m_nPerfectSyncTicks;
    int m_nStrafeTicks;
    int m_nAccelTicks;

    bool m_bPrevTimerRunning;
    int m_nPrevButtons;

    char m_pszDefaultEntName[128];

    // Used by momentum triggers
    Vector m_vecPreviousOrigins[MAX_PREVIOUS_ORIGINS];

    // Start zone thinkfunc
    int m_nTicksInAir;

    float m_flTweenVelValue;
    // Trail pointer
    CBaseEntity* m_eTrail;
};
#endif // MOMPLAYER_H
