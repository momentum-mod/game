#pragma once

#include "weapon_base.h"

// This is the base class for pistols and rifles.
#if defined(CLIENT_DLL)
#define CWeaponBaseGun C_WeaponBaseGun
#endif

class CWeaponBaseGun : public CWeaponBase
{
  public:
    DECLARE_CLASS(CWeaponBaseGun, CWeaponBase);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CWeaponBaseGun();

    virtual void PrimaryAttack();
    virtual void Spawn();
    virtual bool Deploy();
#ifdef WEAPONS_USE_AMMO
    virtual bool Reload();
#endif
    virtual void WeaponIdle();

    // Derived classes call this to fire a bullet.
    bool BaseGunFire(float flSpread, float flCycleTime, bool bPrimaryMode);

    // Usually plays the shot sound. Guns with silencers can play different sounds.
    virtual void DoFireEffects();
    virtual void ItemPostFrame();

  protected:
    float m_zoomFullyActiveTime;
    float m_flTimeToIdleAfterFire;
    float m_flIdleInterval;
    // Is the weapon currently lowered?
    bool m_bWeaponIsLowered;

  private:
    CWeaponBaseGun(const CWeaponBaseGun &);
};