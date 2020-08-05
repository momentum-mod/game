#pragma once

#include "weapon_base.h"

#if defined(CLIENT_DLL)
#define CBlank C_Blank
#endif

// ----------------------------------------------------------------------------- //
// CBlank class definition.
// ----------------------------------------------------------------------------- //

class CBlank : public CWeaponBase
{
  public:
    DECLARE_CLASS(CBlank, CWeaponBase);
    DECLARE_NETWORKCLASS();

    CBlank();

    // We say yes to this so the weapon system lets us switch to it.
    bool HasPrimaryAmmo() override { return true; }
    bool CanBeSelected() override { return true; }

    void PrimaryAttack() override;
    void SecondaryAttack() override;

    void ItemPostFrame() override;

    void WeaponIdle() override;

    WeaponID_t GetWeaponID() const override { return WEAPON_BLANK; }

#ifdef CLIENT_DLL
   bool IsOverridingViewmodel() override;
   int DrawOverriddenViewmodel(C_BaseViewModel* pViewmodel, int flags) override;
#endif

};
