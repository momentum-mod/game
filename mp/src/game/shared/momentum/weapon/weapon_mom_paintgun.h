#pragma once

#include "cbase.h"
#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
#define CMomentumPaintGun C_MomentumPaintGun
#endif

class CMomentumPaintGun : public CWeaponCSBaseGun
{
  public:
    DECLARE_CLASS(CMomentumPaintGun, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumPaintGun()
    {
        m_flTimeToIdleAfterFire = 0.0f;
        m_flIdleInterval = 0.0f;
    };

    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;

    CSWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_PAINTGUN; }

  private:
    void RifleFire();
};
