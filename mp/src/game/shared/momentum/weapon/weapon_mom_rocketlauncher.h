#pragma once

#include "weapon_base_gun.h"

#ifndef CLIENT_DLL
#include "smoke_trail.h"
#endif

//-----------------------------------------------------------------------------
// Rocket
//-----------------------------------------------------------------------------

#ifndef CLIENT_DLL
class CMomentumRocketLauncher;

class CMomentumRocket : public CBaseCombatCharacter
{
    DECLARE_CLASS(CMomentumRocket, CBaseCombatCharacter);

public:
    CMomentumRocket();
    ~CMomentumRocket();

    void Spawn();
    void Precache();
    void Touch(CBaseEntity *pOther);
    void Explode();

    virtual float GetDamage() { return m_flDamage; }
    virtual void SetDamage(float flDamage) { m_flDamage = flDamage; }

    CHandle<CMomentumRocketLauncher> m_hOwner;

    static CMomentumRocket *EmitRocket(const Vector &vecSrc, const QAngle &vecAngles, CBaseEntity *pentOwner);

protected:
    virtual void DoExplosion();

    void CreateSmokeTrail();

    CHandle<RocketTrail> m_hRocketTrail;
    float m_flDamage;

private:
    DECLARE_DATADESC();
};
#endif

//-----------------------------------------------------------------------------
// Rocket Launcher
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CMomentumRocketLauncher C_MomentumRocketLauncher
#endif

class CMomentumRocketLauncher : public CWeaponBase
{
public:
    DECLARE_CLASS(CMomentumRocketLauncher, CWeaponBase);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumRocketLauncher();

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;

    CWeaponID GetWeaponID(void) const OVERRIDE { return WEAPON_ROCKETLAUNCHER; }

private:
    void RocketLauncherFire();
};