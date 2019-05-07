#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumShotgun C_MomentumShotgun
#endif

class CMomentumShotgun : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumShotgun, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumShotgun();

    void PrimaryAttack() OVERRIDE;
#ifdef WEAPONS_USE_AMMO
    bool Reload() OVERRIDE;
#endif
    void WeaponIdle() OVERRIDE;

    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_SHOTGUN; }

  private:
    CMomentumShotgun(const CMomentumShotgun &);

    float m_flPumpTime;
    CNetworkVar(int, m_fInSpecialReload);
};