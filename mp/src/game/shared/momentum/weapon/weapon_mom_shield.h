#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumShield C_MomentumShield
#endif

class CMomentumShield : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumShield, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumShield();

    void PrimaryAttack() OVERRIDE;
    void WeaponIdle() OVERRIDE;

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_SHIELD; }

    float DeployTime() const OVERRIDE { return 0.5f; }
    float PrimaryFireTime() const { return 1.5f; }
    float IdleTime() const { return 2.5f; }
};