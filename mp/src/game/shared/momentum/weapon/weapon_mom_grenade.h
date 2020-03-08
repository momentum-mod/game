#pragma once

#include "weapon_base.h"

#ifdef CLIENT_DLL
#define CMomentumGrenade C_MomentumGrenade
#endif

class CMomentumGrenade : public CWeaponBase
{
  public:
    DECLARE_CLASS(CMomentumGrenade, CWeaponBase);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumGrenade();

    WeaponID_t GetWeaponID() const OVERRIDE { return WEAPON_GRENADE; }

    bool Deploy() override;
    bool Holster(CBaseCombatWeapon *pSwitchingTo) override;

    void PrimaryAttack() override;
    void SecondaryAttack() override;

    bool Reload() override;

    void ItemPostFrame() override;

    void DecrementAmmo(CBaseCombatCharacter *pOwner);
    virtual void StartGrenadeThrow();
    virtual void ThrowGrenade();

    bool IsPinPulled() const { return m_bPinPulled; }
    bool IsBeingThrown() const { return m_fThrowTime > 0; }

#ifdef GAME_DLL
    bool AllowsAutoSwitchFrom() const override { return !m_bPinPulled; }
    int CapabilitiesGet() override { return bits_CAP_WEAPON_RANGE_ATTACK1; }
    virtual void EmitGrenade(const Vector &vecSrc, const QAngle &vecAngles, const Vector &vecVel, CBaseEntity *pOwner);
#endif

  protected:
    CNetworkVar(bool, m_bRedraw);    // Draw the weapon again after throwing a grenade
    CNetworkVar(bool, m_bPinPulled); // Set to true when the pin has been pulled but the grenade hasn't been thrown yet.
    CNetworkVar(float, m_fThrowTime); // the time at which the grenade will be thrown.  If this value is 0 then the time hasn't been set yet.
};