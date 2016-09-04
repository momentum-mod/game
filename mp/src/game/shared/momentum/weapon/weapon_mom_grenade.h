#pragma once

#include "cbase.h"
#include "weapon_basecsgrenade.h"

#ifdef CLIENT_DLL
#define CMomentumGrenade C_MomentumGrenade
#endif

class CMomentumGrenade : public CBaseCSGrenade
{
public:
    DECLARE_CLASS(CMomentumGrenade, CBaseCSGrenade);
    DECLARE_PREDICTABLE();
    DECLARE_NETWORKCLASS();

    CMomentumGrenade() {};

    CSWeaponID GetWeaponID(void) const override { return WEAPON_GRENADE; }

#ifdef GAME_DLL

    void EmitGrenade(Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse,
                             CBasePlayer *pPlayer) override;
#endif
};