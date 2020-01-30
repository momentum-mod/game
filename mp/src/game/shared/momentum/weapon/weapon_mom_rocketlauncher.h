#pragma once

#include "weapon_base_gun.h"
#include "mom_rocket.h"

#ifdef CLIENT_DLL
#define CMomentumRocketLauncher C_MomentumRocketLauncher
#endif

class CMomentumRocketLauncher : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumRocketLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumRocketLauncher();

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;

    void GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward);

    void SetModelType(bool bTF2Model);
    const char *GetViewModel(int viewmodelindex = 0) const OVERRIDE;
    const char *GetWorldModel() const OVERRIDE;

    bool Deploy() OVERRIDE;
    bool CanDeploy() OVERRIDE;

    WeaponID_t GetWeaponID() const OVERRIDE { return WEAPON_ROCKETLAUNCHER; }

    float DeployTime() const OVERRIDE { return 0.5f; }

  private:
    CNetworkVar(int, m_iTFViewIndex);
    CNetworkVar(int, m_iTFWorldIndex);
    CNetworkVar(int, m_iMomViewIndex);
    CNetworkVar(int, m_iMomWorldIndex);
};