#pragma once

#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
#define CMomentumRifle C_MomentumRifle
#endif

class CMomentumRifle : public CWeaponCSBaseGun
{
public:
    DECLARE_CLASS(CMomentumRifle, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumRifle();

    void PrimaryAttack() OVERRIDE;

    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_RIFLE; }

private:
    void RifleFire();
};