#pragma once

#include "weapon_base_gun.h"
#include "mom_df_rocket.h"

#ifdef CLIENT_DLL
#define CMomentumDFPlasmaGun C_MomentumDFPlasmaGun
#endif

class CMomentumDFPlasmaGun : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumDFPlasmaGun, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumDFPlasmaGun();

    void PrimaryAttack() OVERRIDE;
    void Precache() OVERRIDE;

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_DF_PLASMAGUN; }
};
