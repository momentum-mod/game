#pragma once

#include "mom_pipebomb.h"
#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumPipebombLauncher C_MomentumPipebombLauncher
#endif

class CMomentumPipebombLauncher : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumPipebombLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumPipebombLauncher();

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;

    void GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward);

    bool CanDeploy() OVERRIDE;

    CWeaponID GetWeaponID() const OVERRIDE { return WEAPON_PIPEBOMBLAUNCHER; }

  private:
    void PipebombLauncherFire();
};