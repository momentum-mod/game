#pragma once

#include "cbase.h"
#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
#define CMomentumSniper C_MomentumSniper
#endif

class CMomentumSniper : public CWeaponCSBaseGun
{
  public:
    DECLARE_CLASS(CMomentumSniper, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumSniper() {}

    void PrimaryAttack() override;
    void SecondaryAttack() override;

    float GetMaxSpeed() const override;

    CSWeaponID GetWeaponID(void) const override { return WEAPON_SNIPER; }

  private:
    void SniperFire();
};