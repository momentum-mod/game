#pragma once

#include "cbase.h"
#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
#define CMomentumShotgun C_MomentumShotgun
#endif

class CMomentumShotgun : public CWeaponCSBaseGun
{
  public:
    DECLARE_CLASS(CMomentumShotgun, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumShotgun();

    void PrimaryAttack() override;
#ifdef WEAPONS_USE_AMMO
    bool Reload() override;
#endif
    void WeaponIdle() override;

    CSWeaponID GetWeaponID(void) const override { return WEAPON_SHOTGUN; }

  private:
    CMomentumShotgun(const CMomentumShotgun &);

    float m_flPumpTime;
    CNetworkVar(int, m_fInSpecialReload);
};