#pragma once

#include "weapon_base_gun.h"
#include "mom_df_grenade.h"

#ifdef CLIENT_DLL
#define CMomentumDFGrenadeLauncher C_MomentumDFGrenadeLauncher
#endif

class CMomentumDFGrenadeLauncher : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumDFGrenadeLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumDFGrenadeLauncher();

    void PrimaryAttack() OVERRIDE;
    void Precache() OVERRIDE;

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_DF_GRENADELAUNCHER; }
};
