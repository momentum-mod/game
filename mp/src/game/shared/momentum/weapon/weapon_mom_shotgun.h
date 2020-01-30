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

    WeaponID_t GetWeaponID(void) const OVERRIDE { return WEAPON_SHOTGUN; }

    float DeployTime() const OVERRIDE { return 0.5f; }
    float PrimaryFireTime() const { return 0.625f; }
    float PumpTime() const { return 0.5f; }
    float IdleTime() const { return 2.5f; }

  private:
    CMomentumShotgun(const CMomentumShotgun &);

    float m_flPumpTime;
    CNetworkVar(int, m_fInSpecialReload);
};