#pragma once

#include "weapon_base_gun.h"
#include "mom_sticky.h"

#ifdef CLIENT_DLL
#define CMomentumStickyLauncher C_MomentumStickyLauncher
#endif

class CMomentumStickyLauncher : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumStickyLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumStickyLauncher();

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;

    void GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward);

    bool CanDeploy() OVERRIDE;

    CWeaponID GetWeaponID() const OVERRIDE { return WEAPON_STICKYLAUNCHER; }

    float DeployTime() const OVERRIDE { return 0.5f; }

  private:
    void StickyLauncherFire();
};