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
        m_flTimeToIdleAfterFire = 1.9f;
        m_flIdleInterval = 20.0f;
    };

    void PrimaryAttack() OVERRIDE;

    CSWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_PAINTGUN; }

  private:
    void RifleFire();
};
