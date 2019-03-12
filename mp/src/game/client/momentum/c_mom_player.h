#pragma once

#include <mom_modulecomms.h>
#include <run/run_stats.h>
#include "c_mom_triggers.h"

class C_MomentumOnlineGhostEntity;
class C_MomentumReplayGhostEntity;

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
    void Spawn() OVERRIDE;
    virtual void ClientThink(void);

    bool DidPlayerBhop() { return m_SrvData.m_bDidPlayerBhop; }
    bool HasAutoBhop() { return m_SrvData.m_RunData.m_bAutoBhop; }
    // void ResetStrafeSync();

    bool IsWatchingReplay() const { return m_hObserverTarget.Get() && GetReplayEnt(); }

    // Returns the replay entity that the player is watching (first person only)
    int GetSpecEntIndex() const;
    C_MomentumReplayGhostEntity* GetReplayEnt() const;
    C_MomentumOnlineGhostEntity* GetOnlineGhostEnt() const;

    // Overridden for ghost spectating
    Vector GetChaseCamViewOffset(CBaseEntity *target) OVERRIDE;

    int m_afButtonDisabled;

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
    // Ladder stuff
    float m_flGrabbableLadderTime;

    bool m_duckUntilOnGround;
    float m_flStamina;

    C_MomentumOnlineGhostEntity *m_pViewTarget;
    C_MomentumOnlineGhostEntity *m_pSpectateTarget;

    friend class CMomentumGameMovement;
};