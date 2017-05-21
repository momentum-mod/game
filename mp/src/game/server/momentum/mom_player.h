#ifndef MOMPLAYER_H
#define MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_blockfix.h"
#include "momentum/mom_shareddefs.h"
#include "player.h"
#include <GameEventListener.h>
#include <run/mom_entity_run_data.h>
#include <momentum/util/mom_util.h>
#include <run/run_stats.h>
#include <mom_modulecomms.h>

class CMomentumReplayGhostEntity;

// The player can spend this many ticks in the air inside the start zone before their speed is limited
#define MAX_AIRTIME_TICKS 15

// MOM_TODO: Replace this with the custom player model
#define ENTITY_MODEL "models/player/player_shape_base.mdl"

// Change these if you want to change the flashlight sound
#define SND_FLASHLIGHT_ON "CSPlayer.FlashlightOn"
#define SND_FLASHLIGHT_OFF "CSPlayer.FlashlightOff"

// Checkpoints used in the "Checkpoint menu"
struct Checkpoint
{
    bool crouched;
    Vector pos;
    Vector vel;
    QAngle ang;
    char targetName[512];
    char targetClassName[512];

    Checkpoint() : crouched(false), pos(vec3_origin), vel(vec3_origin), ang(vec3_angle)
    {
        targetName[0] = '\0';
        targetClassName[0] = '\0';
    }

    Checkpoint(KeyValues *pKv)
    {
        Q_strncpy(targetName, pKv->GetString("targetName"), sizeof(targetName));
        Q_strncpy(targetClassName, pKv->GetString("targetClassName"), sizeof(targetClassName));
        g_pMomentumUtil->KVLoadVector(pKv, "pos", pos);
        g_pMomentumUtil->KVLoadVector(pKv, "vel", vel);
        g_pMomentumUtil->KVLoadQAngles(pKv, "ang", ang);
        crouched = pKv->GetBool("crouched");
    }
};

class CMomentumPlayer : public CBasePlayer, public CGameEventListener
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
    }

    void FlashlightTurnOff() OVERRIDE
    {
        RemoveEffects(EF_DIMLIGHT);
        EmitSound(SND_FLASHLIGHT_OFF);
    }

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
    void CheckForBhop();
    void UpdateRunStats();
    void UpdateRunSync();
    void UpdateJumpStrafes();
    void UpdateMaxVelocity();
    void UpdateStrafeOffset(float delta);
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

    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance);

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
    bool IsWatchingReplay() const { return m_hObserverTarget.Get() && GetReplayEnt(); }

    CMomentumReplayGhostEntity *GetReplayEnt() const;

    bool IsValidObserverTarget(CBaseEntity *target) OVERRIDE;
    bool SetObserverTarget(CBaseEntity *target) OVERRIDE;
    CBaseEntity *FindNextObserverTarget(bool bReverse) OVERRIDE;
    void CheckObserverSettings() OVERRIDE;

    void StopSpectating();

    // Used by momentum triggers
    Vector GetPrevOrigin(void) const;
    Vector GetPrevOrigin(const Vector &base) const;

    // for calc avg
    int m_nZoneAvgCount[MAX_STAGES];
    float m_flZoneTotalSync[MAX_STAGES], m_flZoneTotalSync2[MAX_STAGES], m_flZoneTotalVelocity[MAX_STAGES][2];
    
    //Overrode for the spectating GUI and weapon dropping
    bool ClientCommand(const CCommand &args) OVERRIDE;
    void MomentumWeaponDrop(CBaseCombatWeapon *pWeapon);

    // Gets the current menu checkpoint index
    int GetCurrentCPMenuStep() const { return m_SrvData.m_iCurrentStepCP; }
    // MOM_TODO: For leaderboard use later on
    bool IsUsingCPMenu() const { return m_SrvData.m_bUsingCPMenu; }
    // Creates a checkpoint on the location of the player
    Checkpoint *CreateCheckpoint();
    // Creates and saves a checkpoint to the checkpoint menu
    void CreateAndSaveCheckpoint();
    // Removes last checkpoint (menu) form the checkpoint lists
    void RemoveLastCheckpoint();
    // Removes every checkpoint (menu) on the checkpoint list
    void RemoveAllCheckpoints();
    // Teleports the player to the checkpoint (menu) with the given index
    void TeleportToCheckpoint(int);
    // Teleports to a provided Checkpoint
    void TeleportToCheckpoint(Checkpoint *pCP);
    // Teleports the player to their current checkpoint
    void TeleportToCurrentCP() { TeleportToCheckpoint(m_SrvData.m_iCurrentStepCP); }
    // Sets the current checkpoint (menu) to the desired one with that index
    void SetCurrentCPMenuStep(int iNewNum) { m_SrvData.m_iCurrentStepCP = iNewNum; }
    // Gets the total amount of menu checkpoints
    int GetCPCount() const { return m_rcCheckpoints.Size(); }
    // Sets wheter or not we're using the CPMenu
    // WARNING! No verification is done. It is up to the caller to don't give false information
    void SetUsingCPMenu(bool bIsUsingCPMenu) { m_SrvData.m_bUsingCPMenu = bIsUsingCPMenu; }

    void SaveCPsToFile(KeyValues *kvInto);
    void LoadCPsFromFile(KeyValues *kvFrom);

    void ToggleDuckThisFrame(bool bState);

    int &GetPerfectSyncTicks() { return m_nPerfectSyncTicks; }
    int &GetStrafeTicks() { return m_nStrafeTicks; }
    int &GetAccelTicks() { return m_nAccelTicks; }

    // Trail Methods

    void Teleport(const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity) OVERRIDE;
    void CreateTrail();
    void RemoveTrail();

  private:
    CountdownTimer m_ladderSurpressionTimer;
    CUtlVector<Checkpoint *> m_rcCheckpoints;
    Vector m_lastLadderNormal;
    Vector m_lastLadderPos;
    EHANDLE g_pLastSpawn;
    bool SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pSpot);

    // for detecting bhop
    float m_flTicksOnGround;
    const int NUM_TICKS_TO_BHOP;
    friend class CMomentumGameMovement;
    float m_flPunishTime;
    int m_iLastBlock;

    // for strafe sync
    float m_flLastVelocity, m_flLastSyncVelocity;
    QAngle m_qangLastAngle;
    int m_nPerfectSyncTicks;
    int m_nStrafeTicks;
    int m_nAccelTicks;
    
    bool m_bKeyChanged;
    bool m_bDirChanged;
    float m_fPrevDtAng;
    int m_nKeyTransTick;
    int m_nAngTransTick;

    bool m_bPrevTimerRunning;
    int m_nPrevButtons;

    char m_pszDefaultEntName[128];

    // Start zone thinkfunc
    int m_nTicksInAir;

    float m_flTweenVelValue;

    // Trail pointer
    CBaseEntity* m_eTrail;
};
#endif // MOMPLAYER_H
