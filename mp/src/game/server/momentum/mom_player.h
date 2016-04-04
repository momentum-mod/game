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

    void Spawn();
    void Precache();
    void Touch(CBaseEntity *);
    void Think();

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
    void ResetStrafeSync();

    CNetworkVar(int, m_iShotsFired);
    CNetworkVar(int, m_iDirection);
    CNetworkVar(bool, m_bResumeZoom);
    CNetworkVar(int, m_iLastZoom);
    CNetworkVar(bool, m_bAutoBhop);
    CNetworkVar(bool, m_bDidPlayerBhop);
    CNetworkVar(int, m_iSuccessiveBhops);
    CNetworkVar(bool, m_bPlayerInsideStartZone);
    CNetworkVar(bool, m_bPlayerInsideEndZone);
    CNetworkVar(bool, m_bHasPracticeMode);
    CNetworkVar(bool, m_bPlayerFinishedMap);
    CNetworkVar(float, m_flStrafeSync); //eyeangle based, perfect strafes / total strafes
    CNetworkVar(float, m_flStrafeSync2); //acceleration based, strafes speed gained / total strafes
    CNetworkVar(float, m_flLastJumpVel);
    CNetworkVar(bool , m_bTimerIsRunning);
    CNetworkVar(float, m_flStartSpeed);
    CNetworkVar(float, m_flEndSpeed);
    CNetworkVar(int, m_nTotalJumps);
    CNetworkVar(int, m_nTotalStrafes);


    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, float flDistance, int iPenetration,
                    int iBulletType, int iDamage, float flRangeModifier, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    void SetPunishTime(float newTime) { m_flPunishTime = newTime; }

    void SetLastBlock(int lastBlock) { m_iLastBlock = lastBlock; }

    int GetLastBlock() { return m_iLastBlock; }
    float GetPunishTime() { return m_flPunishTime; }

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
    float m_flLastVelocity;
    QAngle m_qangLastAngle;

    int m_nPerfectSyncTicks;
    int m_nStrafeTicks;
    int m_nAccelTicks;

    bool m_bPrevTimerRunning;
    int m_nPrevButtons;
};
#endif // MOMPLAYER_H