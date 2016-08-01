#pragma once

#include "cbase.h"
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

    CMomentumLMG()
    {
        m_flTimeToIdleAfterFire = 1.6f;
        m_flIdleInterval = 20.0f;
    };

    void PrimaryAttack() override;
    CSWeaponID GetWeaponID(void) const override { return WEAPON_LMG; }

private:
    CMomentumLMG(const CMomentumLMG &);
    void LMGFire();
};