#pragma once

#include "weapon_base_gun.h"
#include "mom_df_rocket.h"

#ifdef CLIENT_DLL
#define CMomentumDFBFG C_MomentumDFBFG
#endif

class CMomentumDFBFG : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumDFBFG, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumDFBFG();

    void PrimaryAttack() OVERRIDE;
    void Precache() OVERRIDE;

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_DF_BFG; }
};
