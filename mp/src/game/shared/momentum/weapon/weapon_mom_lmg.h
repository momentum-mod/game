#pragma once

#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumLMG C_MomentumLMG
#endif

class CMomentumLMG : public CWeaponBaseGun
{
public:
    DECLARE_CLASS(CMomentumLMG, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumLMG();

    void PrimaryAttack() OVERRIDE;
    CWeaponID GetWeaponID() const OVERRIDE { return WEAPON_LMG; }
};