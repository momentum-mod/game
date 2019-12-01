#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumRifle C_MomentumRifle
#endif

class CMomentumRifle : public CWeaponBaseGun
{
public:
    DECLARE_CLASS(CMomentumRifle, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumRifle();

    void PrimaryAttack() OVERRIDE;

    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_RIFLE; }
};