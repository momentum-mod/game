#pragma once

#include "weapon_base_gun.h"
#include "mom_df_rocket.h"

#ifdef CLIENT_DLL
#define CMomentumDFLightningGun C_MomentumDFLightningGun
#endif

class CMomentumDFLightningGun : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumDFLightningGun, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumDFLightningGun();

    void PrimaryAttack() OVERRIDE;
    void Precache() OVERRIDE;

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_DF_PLASMAGUN; }
};
