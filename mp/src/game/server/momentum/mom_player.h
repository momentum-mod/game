#ifndef MOMPLAYER_H
#define MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_blockfix.h"
#include "mom_entity_run_data.h"
#include "momentum/mom_shareddefs.h"
#include "player.h"
#include "util/run_stats.h"

class CMomentumReplayGhostEntity;

// The player can spend this many ticks in the air inside the start zone before their speed is limited
#define MAX_AIRTIME_TICKS 15

class CMomentumPlayer : public CBasePlayer
{
  public:
    DECLARE_CLASS(CMomentumPlayer, CBasePlayer);

    CMomentumPlayer();
    ~CMomentumPlayer(void);

    static CMomentumPlayer *CreatePlayer(const char *className, edict_t *ed)
    {
        s_PlayerEdict = ed;
        return static_cast<CMomentumPlayer *>(CreateEntityByName(className));
    }

    DECLARE_SERVERCLASS();
    DECLARE_DATADESC();

    int FlashlightIsOn() override { return IsEffectActive(EF_DIMLIGHT); }

    void FlashlightTurnOn() override
    {
        AddEffects(EF_DIMLIGHT);
        EmitSound("HL2Player.FlashLightOn"); // MOM_TODO: change this?
    }

    void FlashlightTurnOff() override
    {
        RemoveEffects(EF_DIMLIGHT);
        EmitSound("HL2Player.FlashLightOff"); // MOM_TODO: change this?
    }

    void Spawn() override;
    void Precache() override;

    // MOM_TODO: This is called when the player spawns so that HUD elements can be updated
    // void InitHUD() override;

    void CommitSuicide(bool bExplode = false, bool bForce = false) override{};

    void CommitSuicide(const Vector &vecForce, bool bExplode = false, bool bForce = false) override{};

    bool CanBreatheUnderwater() const override { return true; }

    // LADDERS
    void SurpressLadderChecks(const Vector &pos, const Vector &normal);
    bool CanGrabLadder(const Vector &pos, const Vector &normal);
    Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

    // SPAWNING
    CBaseEntity *EntSelectSpawnPoint() override;

    // used by CMomentumGameMovement
    bool m_duckUntilOnGround;
    float m_flStamina;

    void EnableAutoBhop();
    void DisableAutoBhop();
    bool HasAutoBhop() const { return m_RunData.m_bAutoBhop; }
    bool DidPlayerBhop() const { return m_bDidPlayerBhop; }
    // think function for detecting if player bhopped
    void CheckForBhop();
    void UpdateRunStats();
    // slows down the player in a tween-y fashion
    void TweenSlowdownPlayer();
    void ResetRunStats();
    void CalculateAverageStats();
    void LimitSpeedInStartZone();

    // These are used for weapon code, MOM_TODO: potentially remove?
    CNetworkVar(int, m_iShotsFired);
    CNetworkVar(int, m_iDirection);
    CNetworkVar(bool, m_bResumeZoom);
    CNetworkVar(int, m_iLastZoom);

    CNetworkVar(bool, m_bDidPlayerBhop);   // Did the player bunnyhop successfully?
    CNetworkVar(int, m_iSuccessiveBhops);  // How many successive bhops this player has
    CNetworkVar(bool, m_bHasPracticeMode); // Is the player in practice mode?

    CNetworkVarEmbedded(CMOMRunEntityData, m_RunData); // Current run data, used for hud elements
    CNetworkVarEmbedded(CMomRunStats, m_RunStats); // Run stats, also used for hud elements

    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, float flDistance, int iPenetration,
                    int iBulletType, int iDamage, float flRangeModifier, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    // Used by g_MOMBlockFix door/button fix code
    void Touch(CBaseEntity *) override;
    int GetLastBlock() const { return m_iLastBlock; }
    float GetPunishTime() const { return m_flPunishTime; }
    void SetPunishTime(float newTime) { m_flPunishTime = newTime; }
    void SetLastBlock(int lastBlock) { m_iLastBlock = lastBlock; }

    //Replay stuff
    bool IsWatchingReplay() const { return m_hObserverTarget.Get() && GetReplayEnt(); }

	CMomentumReplayGhostEntity *GetReplayEnt() const;

    bool IsValidObserverTarget(CBaseEntity *target) override;
    bool SetObserverTarget(CBaseEntity *target) override;
    CBaseEntity *FindNextObserverTarget(bool bReverse) override;

    void StopSpectating();

    // Used by momentum triggers
    Vector GetPrevOrigin(void);
    Vector GetPrevOrigin(const Vector &base);

    // for calc avg
    int m_nZoneAvgCount[MAX_STAGES];
    float m_flZoneTotalSync[MAX_STAGES], m_flZoneTotalSync2[MAX_STAGES], m_flZoneTotalVelocity[MAX_STAGES][2];

  private:
    CountdownTimer m_ladderSurpressionTimer;
    Vector m_lastLadderNormal;
    Vector m_lastLadderPos;
    EHANDLE g_pLastSpawn;
    bool SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pSpot);

    // for detecting bhop
    float m_flTicksOnGround;
    const int NUM_TICKS_TO_BHOP = 10;
    friend class CMomentumGameMovement;
    float m_flPunishTime;
    int m_iLastBlock;

    // for strafe sync
    float m_flLastVelocity, m_flLastSyncVelocity;
    QAngle m_qangLastAngle;

    int m_nPerfectSyncTicks;
    int m_nStrafeTicks;
    int m_nAccelTicks;

    bool m_bPrevTimerRunning;
    int m_nPrevButtons;

    // Start zone thinkfunc
    int m_nTicksInAir;

    float m_flTweenVelValue = 1.0f;
};
#endif // MOMPLAYER_H