#pragma once

#include "weapon_base_gun.h"
#include "mom_rocket_projectile.h"

#ifdef CLIENT_DLL
#define CMomentumRocketLauncher C_MomentumRocketLauncher
#endif

class CMomentumRocketLauncher : public CWeaponBase
{
  public:
    DECLARE_CLASS(CMomentumRocketLauncher, CWeaponBase);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumRocketLauncher() {}

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;

    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_ROCKETLAUNCHER; }

  private:
    void RocketLauncherFire();
};