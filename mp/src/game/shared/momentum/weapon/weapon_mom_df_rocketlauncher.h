#pragma once

#include "weapon_base_gun.h"
#include "mom_df_rocket.h"

#ifdef CLIENT_DLL
#define CMomentumDFRocketLauncher C_MomentumDFRocketLauncher
#endif

class CMomentumDFRocketLauncher : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumDFRocketLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumDFRocketLauncher();

    void PrimaryAttack() OVERRIDE;
    void Precache() OVERRIDE;

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_DF_ROCKETLAUNCHER; }
};
