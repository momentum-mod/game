#pragma once

#include "cbase.h"
#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
#define CMomentumSMG C_MomentumSMG
#endif

class CMomentumSMG : public CWeaponCSBaseGun
{
  public:
    DECLARE_CLASS(CMomentumSMG, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumSMG()
    {
        m_flIdleInterval = 20.0f;
        m_flTimeToIdleAfterFire = 2.0f;
    }

    void PrimaryAttack() override;

    CSWeaponID GetWeaponID(void) const override { return WEAPON_SMG; }

  private:
    void SMGFire();
};