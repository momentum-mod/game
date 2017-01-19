#ifndef C_MOMPLAYER_H
#define C_MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "c_mom_replay_entity.h"
#include <momentum/mom_shareddefs.h>
#include <run/mom_entity_run_data.h>
#include <run/run_stats.h>


class C_MomentumPlayer : public C_BasePlayer
{
  public:
    DECLARE_CLASS(C_MomentumPlayer, C_BasePlayer);
    DECLARE_CLIENTCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_INTERPOLATION();

    C_MomentumPlayer();
    ~C_MomentumPlayer();

    void PostDataUpdate(DataUpdateType_t updateType) OVERRIDE;
    void OnDataChanged(DataUpdateType_t type) OVERRIDE;
    bool CreateMove(float flInputSampleTime, CUserCmd *pCmd) OVERRIDE;
    virtual void ClientThink(void);

    Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

    void SurpressLadderChecks(const Vector &pos, const Vector &normal);
    bool CanGrabLadder(const Vector &pos, const Vector &normal);
    bool DidPlayerBhop() { return m_bDidPlayerBhop; }
    bool HasAutoBhop() { return m_RunData.m_bAutoBhop; }
    // void ResetStrafeSync();

    bool IsWatchingReplay() const { return m_hObserverTarget.Get() && GetReplayEnt(); }

    // Returns the replay entity that the player is watching (first person only)
    C_MomentumReplayGhostEntity *GetReplayEnt() const
    {
        return dynamic_cast<C_MomentumReplayGhostEntity *>(m_hObserverTarget.Get());
    }

    Vector GetChaseCamViewOffset(CBaseEntity *target) OVERRIDE;

    int m_iShotsFired;
    int m_iDirection;
    bool m_bResumeZoom;
    int m_iLastZoom;
    bool m_bDidPlayerBhop;
    bool m_bHasPracticeMode;
 
    int m_afButtonDisabled;
    int m_fSliding;

    bool m_bUsingCPMenu;
    int m_iCurrentStepCP;
    int m_iCheckpointCount;

    CMOMRunEntityData m_RunData;
    CMomRunStats m_RunStats;

    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, float flDistance, int iPenetration,
                    int iBulletType, int iDamage, float flRangeModifier, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    float m_flStartSpeed;
    float m_flEndSpeed;
    int m_iSuccessiveBhops;

  private:
    CountdownTimer m_ladderSurpressionTimer;
    Vector m_lastLadderNormal;
    Vector m_lastLadderPos;

    bool m_duckUntilOnGround;
    float m_flStamina;

    friend class CMomentumGameMovement;
};

#endif
