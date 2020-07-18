#pragma once

#include "weapon_base.h"

#ifdef CLIENT_DLL
#define CMomentumConcGrenade C_MomentumConcGrenade
#endif

class CMomentumConcGrenade : public CWeaponBase
{
  public:
    DECLARE_CLASS(CMomentumConcGrenade, CWeaponBase);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumConcGrenade();

    WeaponID_t GetWeaponID() const override { return WEAPON_CONCGRENADE; }

    bool Deploy() override;
    bool Holster(CBaseCombatWeapon *pSwitchingTo) override;

    void PrimaryAttack() override;
    void SecondaryAttack() override;

    void ItemPostFrame() override;

    void DrawRadius();

    void StartGrenadeThrow();
    void ThrowGrenade(float flTimer, float flSpeed = 630.0f);

    float GetGrenadeTimer() { return m_flTimer; }
    float GetMaxTimer() { return 3.81f; }

#ifdef GAME_DLL
    bool AllowsAutoSwitchFrom() const override { return !m_bPrimed; }
    int CapabilitiesGet() override { return bits_CAP_WEAPON_RANGE_ATTACK1; }
#endif

  protected:
    CNetworkVar(bool, m_bPrimed);      // Set to true when the pin has been pulled but the grenade hasn't been thrown yet.
    CNetworkVar(float, m_flThrowTime); // The time at which the grenade will be thrown.  If this value is 0 then the time hasn't been set yet.
    CNetworkVar(float, m_flTimer);     // Once the grenade is cooked, this gets incremented until the grenade is meant to go off.
};