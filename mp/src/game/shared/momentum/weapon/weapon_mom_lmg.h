#pragma once

#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
#define CMomentumLMG C_MomentumLMG
#endif

class CMomentumLMG : public CWeaponCSBaseGun
{
public:
    DECLARE_CLASS(CMomentumLMG, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumLMG();

    void PrimaryAttack() OVERRIDE;
    CSWeaponID GetWeaponID() const OVERRIDE { return WEAPON_LMG; }

private:
    CMomentumLMG(const CMomentumLMG &);
    void LMGFire();
};