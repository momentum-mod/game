#ifndef C_MOMPLAYER_H
#define C_MOMPLAYER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include <mom_modulecomms.h>
#include <momentum/mom_shareddefs.h>
#include <run/mom_entity_run_data.h>
#include <run/run_stats.h>
#include "c_mom_online_ghost.h"
#include "c_mom_replay_entity.h"

class C_TriggerSlide;

class C_MomentumPlayer : public C_BasePlayer
{
    friend class CMomentumGameMovement;

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
    void Spawn() OVERRIDE;
    virtual void ClientThink(void);
	void Touch(CBaseEntity *pOther) OVERRIDE;

    bool DidPlayerBhop() { return m_SrvData.m_bDidPlayerBhop; }
    bool HasAutoBhop() { return m_bAutoBhop; }

    bool IsWatchingReplay() const { return m_hObserverTarget.Get() && GetReplayEnt(); }

	// Used by g_MomentumBlockFix door/button fix code
	int GetLastBlock() const { return m_iLastBlock; }
	float GetPunishTime() const { return m_flPunishTime; }
	void SetPunishTime(float newTime) { m_flPunishTime = newTime; }
	void SetLastBlock(int lastBlock) { m_iLastBlock = lastBlock; }

    // Returns the replay entity that the player is watching (first person only)
    C_MomentumReplayGhostEntity *GetReplayEnt() const
    {
        return dynamic_cast<C_MomentumReplayGhostEntity *>(m_hObserverTarget.Get());
    }

    C_MomentumOnlineGhostEntity *GetOnlineGhostEnt() const
    {
        return dynamic_cast<C_MomentumOnlineGhostEntity *>(m_hObserverTarget.Get());
    }

    // Overridden for ghost spectating
    Vector GetChaseCamViewOffset(CBaseEntity *target) OVERRIDE;

    CNetworkVar(int, m_afButtonDisabled);

    StdDataFromServer m_SrvData;
    CMomRunStats m_RunStats;

    CNetworkHandle(C_TriggerSlide, m_CurrentSlideTrigger); 

    void GetBulletTypeParameters(int iBulletType, float &fPenetrationPower, float &flPenetrationDistance,
                                 bool &bIsPaintAmmo);

    void FireBullet(Vector vecSrc, const QAngle &shootAngles, float vecSpread, float flDistance, int iPenetration,
                    int iBulletType, int iDamage, float flRangeModifier, CBaseEntity *pevAttacker, bool bDoEffects,
                    float x, float y);

    void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
                  float lateral_max, int direction_change);

    float m_flStartSpeed;
    float m_flEndSpeed;

    // Ladder stuff
    float GetGrabbableLadderTime() const { return m_flGrabbableLadderTime; }
    void SetGrabbableLadderTime(float new_time) { m_flGrabbableLadderTime = new_time; }
  private:
    float m_flGrabbableLadderTime;
    bool m_duckUntilOnGround;

	float m_flPunishTime;
	int m_iLastBlock;

    CNetworkVar(float, m_flStamina);
    CNetworkVar(bool, m_bAutoBhop);

    C_MomentumOnlineGhostEntity *m_pViewTarget;
    C_MomentumOnlineGhostEntity *m_pSpectateTarget;
};

#endif
