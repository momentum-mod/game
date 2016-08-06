#include "cbase.h"
#include "mom_grenade_projectile.h"

#include "tier0/memdbgon.h"

//MOM_TODO: Change this model to be something custom
#define GRENADE_MODEL "models/Weapons/w_grenade.mdl"

LINK_ENTITY_TO_CLASS(momgrenade_projectile, CMomGrenadeProjectile);
PRECACHE_WEAPON_REGISTER(momgrenade_projectile);

CMomGrenadeProjectile* CMomGrenadeProjectile::Create(
    const Vector &position,
    const QAngle &angles,
    const Vector &velocity,
    const AngularImpulse &angVelocity,
    CBaseCombatCharacter *pOwner,
    float timer)
{
    auto *pGrenade = static_cast<CMomGrenadeProjectile*>(CBaseEntity::Create("momgrenade_projectile", position, angles, pOwner));

    // Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
    // one second before detonation.
    pGrenade->SetDetonateTimerLength(1.5);
    pGrenade->SetAbsVelocity(velocity);
    pGrenade->SetupInitialTransmittedGrenadeVelocity(velocity);
    pGrenade->SetThrower(pOwner);

    pGrenade->SetGravity(GetGrenadeGravity());
    pGrenade->SetFriction(GetGrenadeFriction());
    pGrenade->SetElasticity(GetGrenadeElasticity());

    pGrenade->m_flDamage = 100;
    pGrenade->m_DmgRadius = pGrenade->m_flDamage * 3.5f;
    pGrenade->ApplyLocalAngularVelocityImpulse(angVelocity);

    // make NPCs afaid of it while in the air
    pGrenade->SetThink(&CMomGrenadeProjectile::DangerSoundThink);
    pGrenade->SetNextThink(gpGlobals->curtime);

    return pGrenade;
}

void CMomGrenadeProjectile::Spawn()
{
    SetModel(GRENADE_MODEL);
    BaseClass::Spawn();
}

void CMomGrenadeProjectile::Precache()
{
    PrecacheModel(GRENADE_MODEL);

    PrecacheScriptSound("MOMGrenade.Bounce");

    BaseClass::Precache();
}

void CMomGrenadeProjectile::BounceSound(void)
{
    EmitSound("MOMGrenade.Bounce");
}