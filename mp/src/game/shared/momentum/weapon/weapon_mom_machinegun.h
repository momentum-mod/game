#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumMachinegun C_MomentumMachinegun
#endif

class CMomentumMachinegun : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumMachinegun, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumMachinegun();

    void PrimaryAttack() OVERRIDE;

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_MACHINEGUN; }
};
