#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumSniper C_MomentumSniper
#endif

class CMomentumSniper : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumSniper, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumSniper();

    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;
    void Drop(const Vector &vecVelocity) OVERRIDE;

    float GetMaxSpeed() const OVERRIDE;

    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_SNIPER; }

  private:
    int m_iRequestedFOV;
};