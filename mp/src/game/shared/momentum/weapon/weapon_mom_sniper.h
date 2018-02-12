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

    CMomentumSniper();

    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;

    float GetMaxSpeed() const OVERRIDE;

    CSWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_SNIPER; }

  private:
    void SniperFire();

    int m_iRequestedFOV;
};