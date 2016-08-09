#ifndef C_MOMPLAYER_H
#define C_MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "../../shared/momentum/mom_shareddefs.h"
#include "c_mom_replay_entity.h"
#include "../../shared/momentum/mom_entity_run_data.h"
#include "../../shared/momentum/util/run_stats.h"

class C_MomentumPlayer : public C_BasePlayer
{
public:
    DECLARE_CLASS(C_MomentumPlayer, C_BasePlayer);
    DECLARE_CLIENTCLASS();

    C_MomentumPlayer();
    ~C_MomentumPlayer();

    Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

    void SurpressLadderChecks(const Vector& pos, const Vector& normal);
    bool CanGrabLadder(const Vector& pos, const Vector& normal);
    bool DidPlayerBhop() { return m_bDidPlayerBhop; }
    bool HasAutoBhop() { return m_RunData.m_bAutoBhop; }
    //void ResetStrafeSync();

    bool IsWatchingReplay() const
    {
        return m_hObserverTarget.Get() && GetReplayEnt();
    }

    //Returns the replay entity that the player is watching (first person only)
    C_MomentumReplayGhostEntity *GetReplayEnt() const
    {
        return dynamic_cast<C_MomentumReplayGhostEntity*>(m_hObserverTarget.Get());
    }

    int m_iShotsFired;
    int m_iDirection;
    bool m_bResumeZoom;
    int m_iLastZoom;
    bool m_bDidPlayerBhop;
    bool m_bHasPracticeMode;

    CMOMRunEntityData m_RunData;
    CMomRunStats m_RunStats;

    void GetBulletTypeParameters(
        int iBulletType,
        float &fPenetrationPower,
        float &flPenetrationDistance);

    void FireBullet(
        Vector vecSrc,
        const QAngle &shootAngles,
        float vecSpread,
        float flDistance,
        int iPenetration,
        int iBulletType,
        int iDamage,
        float flRangeModifier,
        CBaseEntity *pevAttacker,
        bool bDoEffects,
        float x,
        float y);

    void KickBack(
        float up_base,
        float lateral_base,
        float up_modifier,
        float lateral_modifier,
        float up_max,
        float lateral_max,
        int direction_change);

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
