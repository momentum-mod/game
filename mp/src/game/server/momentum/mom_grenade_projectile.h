#pragma once

#include "cbase.h"
#include "weapon/basecsgrenade_projectile.h"

class CMomGrenadeProjectile : public CBaseCSGrenadeProjectile
{
public:
    DECLARE_CLASS(CMomGrenadeProjectile, CBaseCSGrenadeProjectile);

    // Overrides.
    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;
    void BounceSound(void) OVERRIDE;

    // Grenade stuff.
    static CMomGrenadeProjectile* Create(
        const Vector &position,
        const QAngle &angles,
        const Vector &velocity,
        const AngularImpulse &angVelocity,
        CBaseCombatCharacter *pOwner,
        float timer);

};