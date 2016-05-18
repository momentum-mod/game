#ifndef MOMPLAYER_H
#define MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_blockfix.h"
#include "momentum/mom_shareddefs.h"
#include "player.h"

class CMomentumPlayer : public CBasePlayer
{
  public:
    DECLARE_CLASS(CMomentumPlayer, CBasePlayer);

    CMomentumPlayer();
    ~CMomentumPlayer(void);

    static CMomentumPlayer *CreatePlayer(const char *className, edict_t *ed)
    {
        CMomentumPlayer::s_PlayerEdict = ed;
        return (CMomentumPlayer *)CreateEntityByName(className);
    }

    DECLARE_SERVERCLASS();
    DECLARE_DATADESC();

    int FlashlightIsOn() { return IsEffectActive(EF_DIMLIGHT); }

    void FlashlightTurnOn()
    {
        AddEffects(EF_DIMLIGHT);
        EmitSound("HL2Player.FlashLightOn"); // MOM_TODO: change this?
    }

    void FlashlightTurnOff()
    {
        RemoveEffects(EF_DIMLIGHT);
        EmitSound("HL2Player.FlashLightOff"); // MOM_TODO: change this?
    }

    void Spawn() override;
    void Precache() override;
    void Touch(CBaseEntity *) override;

    void InitHUD() override;

    virtual void CommitSuicide(bool bExplode = false, bool bForce = false){};
    virtual void CommitSuicide(const Vector &vecForce, bool bExplode = false, bool bForce = false){};

    bool CanBreatheUnderwater() const { return true; }

    // LADDERS
    void SurpressLadderChecks(const Vector &pos, const Vector &normal);
    bool CanGrabLadder(const Vector &pos, const Vector &normal);
    Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

    // SPAWNING
    CBaseEntity *EntSelectSpawnPoint();

    // used by CMomentumGameMovement
    bool m_duckUntilOnGround;
    float m_flStamina;

    void EnableAutoBhop();
    void DisableAutoBhop();
    bool HasAutoBhop() { return m_bAutoBhop; }
    bool DidPlayerBhop() { return m_bDidPlayerBhop; }
    // think function for detecting if player bhopped
    void CheckForBhop();
    void UpdateRunStats();
    void ResetRunStats();
    void CalculateAverageStats();
    void LimitSpeedInStartZone();

    //These are used for weapon code, MOM_TODO: potentially remove?
    CNetworkVar(int, m_iShotsFired);
    CNetworkVar(int, m_iDirection);
    CNetworkVar(bool, m_bResumeZoom);
    CNetworkVar(int, m_iLastZoom);

    CNetworkVar(bool, m_bAutoBhop);// Is the player using auto bhop?
    CNetworkVar(bool, m_bDidPlayerBhop);// Did the player bunnyhop successfully?
    CNetworkVar(int, m_iSuccessiveBhops); //How many successive bhops this player has
    CNetworkVar(float, m_flStrafeSync); //eyeangle based, perfect strafes / total strafes
    CNetworkVar(float, m_flStrafeSync2); //acceleration based, strafes speed gained / total strafes
    CNetworkVar(bool, m_bIsWatchingReplay);
    CNetworkVar(int, m_nReplayButtons);
    CNetworkVar(float, m_flLastJumpVel); //Last jump velocity of the player
    CNetworkVar(int, m_iRunFlags);//The run flags (W only/HSW/Scroll etc) of the player
    CNetworkVar(bool, m_bIsInZone);//This is true if the player is in a CTriggerTimerStage zone
    CNetworkVar(bool, m_bMapFinished);//Did the player finish the map?
    CNetworkVar(int, m_iCurrentStage);//Current stage the player is on
    CNetworkVar(float, m_flLastJumpTime);//The last time that the player jumped

    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, float flDistance, int iPenetration,
                    int iBulletType, int iDamage, float flRangeModifier, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    void SetPunishTime(float newTime) { m_flPunishTime = newTime; }

    void SetLastBlock(int lastBlock) { m_iLastBlock = lastBlock; }
    bool IsValidObserverTarget(CBaseEntity *target) override;

    int GetLastBlock() { return m_iLastBlock; }
    float GetPunishTime() { return m_flPunishTime; }

    //stage stats. index 0 is overall stats
    int m_nStageJumps[MAX_STAGES], m_nStageStrafes[MAX_STAGES];
    float m_flStageStrafeSyncAvg[MAX_STAGES], m_flStageStrafeSync2Avg[MAX_STAGES];

    //These members are 2D arrays so we can store both 2D and 3D velocities in them. Index 0 is 3D and index 1 is 2D
    float m_flStageVelocityMax[MAX_STAGES][2], 
        m_flStageVelocityAvg[MAX_STAGES][2], 
        m_flStageEnterVelocity[MAX_STAGES][2],//The velocity with which you enter the stage (leave the stage start trigger)
        m_flStageExitVelocity[MAX_STAGES][2];//The velocity with which you exit this stage (this stage -> next)

    //for calc avg
    int m_nStageAvgCount[MAX_STAGES];
    float m_flStageTotalSync[MAX_STAGES], m_flStageTotalSync2[MAX_STAGES],
        m_flStageTotalVelocity[MAX_STAGES][2];

    //bool m_bInsideStartZone;
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

    //for strafe sync
    float m_flLastVelocity, m_flLastSyncVelocity;
    QAngle m_qangLastAngle;

    int m_nPerfectSyncTicks;
    int m_nStrafeTicks;
    int m_nAccelTicks;

    bool m_bPrevTimerRunning;
    int m_nPrevButtons;

    //Start zone thinkfunc
    int m_nTicksInAir;
    const int MAX_AIRTIME_TICKS = 15; //The player can spend this many ticks in the air inside the start zone before their speed is limited
};
#endif // MOMPLAYER_H