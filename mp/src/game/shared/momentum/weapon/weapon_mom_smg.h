#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumSMG C_MomentumSMG
#endif

class CMomentumSMG : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumSMG, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumSMG();

    void PrimaryAttack() OVERRIDE;

    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_SMG; }
};