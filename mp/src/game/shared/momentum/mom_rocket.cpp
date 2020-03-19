#include "cbase.h"

#include "mom_rocket.h"

#include "mom_shareddefs.h"
#include "weapon/weapon_def.h"

#ifndef CLIENT_DLL
#include "momentum/fx_mom_shared.h"
#include "momentum/mom_triggers.h"
#endif

#include "tier0/memdbgon.h"

#define MOM_ROCKET_SPEED 1100.0f

IMPLEMENT_NETWORKCLASS_ALIASED(MomRocket, DT_MomRocket);

BEGIN_NETWORK_TABLE(CMomRocket, DT_MomRocket)
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_rocket, CMomRocket);
PRECACHE_WEAPON_REGISTER(momentum_rocket);

#ifdef GAME_DLL
static MAKE_TOGGLE_CONVAR(mom_rj_use_tf_rocketmodel, "0", FCVAR_ARCHIVE, "Toggles between the TF2 rocket model and the Momentum one. 0 = Momentum, 1 = TF2\n");
static MAKE_TOGGLE_CONVAR(mom_rj_trail_sound_enable, "1", FCVAR_ARCHIVE, "Toggles the rocket trail sound. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_rj_decals_enable, "1", FCVAR_ARCHIVE, "Toggles creating decals on rocket explosion. 0 = OFF, 1 = ON\n");
#else
static MAKE_CONVAR(mom_rj_trail, "1", FCVAR_ARCHIVE, "Toggles between the TF2 rocket trail and the Momentum one. 0 = None, 1 = Momentum, 2 = TF2\n", 0, 3);
static MAKE_CONVAR(mom_rj_rocket_drawdelay, "0.2", FCVAR_ARCHIVE, "Determines how long it takes for rockets to start being drawn after spawning.\n", 0, 1);
#endif

void CMomRocket::Spawn()
{
    BaseClass::Spawn();

#ifdef GAME_DLL
    SetModel(g_pWeaponDef->GetWeaponModel(WEAPON_ROCKETLAUNCHER, "rocket"));

    SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
    AddEFlags(EFL_NO_WATER_VELOCITY_CHANGE);
    SetSize(Vector(0, 0, 0), Vector(0, 0, 0));

    SetGravity(0.0f);

    SetTouch(&CMomRocket::RocketTouch);
    SetNextThink(gpGlobals->curtime);
#endif
}

#ifdef CLIENT_DLL

float CMomRocket::GetDrawDelayTime()
{
    return mom_rj_rocket_drawdelay.GetFloat();
}

void CMomRocket::CreateTrailParticles()
{
    if (mom_rj_trail.GetInt() == 0)
        return;

    ParticleProp()->Create(g_pWeaponDef->GetWeaponParticle(WEAPON_ROCKETLAUNCHER, "RocketTrail"), PATTACH_POINT_FOLLOW, "trail");
}

#else

void CMomRocket::StopTrailSound()
{
    StopSound(g_pWeaponDef->GetWeaponSound(WEAPON_ROCKETLAUNCHER, "RocketTrail"));
}

void CMomRocket::Destroy(bool bShowFizzleSprite)
{
    StopTrailSound();

    BaseClass::Destroy(bShowFizzleSprite);
}

void CMomRocket::Explode(trace_t *pTrace, CBaseEntity *pOther)
{
    if (CNoGrenadesZone::IsInsideNoGrenadesZone(this))
    {
        Destroy(true);
        return;
    }

    // Make invisible
    SetModelName(NULL_STRING);
    SetSolid(SOLID_NONE);
    m_takedamage = DAMAGE_NO;

    // Pull out a bit
    if (!CloseEnough(pTrace->fraction, 1.0f))
    {
        SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
    }

    const auto vecOrigin = GetAbsOrigin();

    // Effect
    CPVSFilter filter(vecOrigin);
    TE_TFExplosion(filter, vecOrigin, pTrace->plane.normal, WEAPON_ROCKETLAUNCHER);

    // Damage
    const CTakeDamageInfo info(this, GetOwnerEntity(), vec3_origin, vecOrigin, GetDamage(), GetDamageType());
    RadiusDamage(info, vecOrigin, MOM_EXPLOSIVE_RADIUS, CLASS_NONE, nullptr);

    StopTrailSound();

    if (mom_rj_decals_enable.GetBool() && pOther && !pOther->IsPlayer())
    {
        UTIL_DecalTrace(pTrace, "RocketScorch");
    }

    UTIL_Remove(this);
}

void CMomRocket::RocketTouch(CBaseEntity *pOther)
{
    Assert(pOther);

    // Don't touch triggers
    if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
        return;

    // Handle hitting skybox (disappear).
    const trace_t *pTrace = &GetTouchTrace();
    if (pTrace->surface.flags & SURF_SKY)
    {
        StopTrailSound();

        UTIL_Remove(this);
        return;
    }

    // Explode
    trace_t trace;
    memcpy(&trace, pTrace, sizeof(trace_t));
    Explode(&trace, pOther);
}

CMomRocket *CMomRocket::EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner)
{
    const auto pRocket = dynamic_cast<CMomRocket *>(CreateNoSpawn("momentum_rocket", vecOrigin, vecAngles, pOwner));
    if (!pRocket)
        return nullptr;

    DispatchSpawn(pRocket);

    Vector vecForward;
    AngleVectors(vecAngles, &vecForward);

    const Vector velocity = vecForward * MOM_ROCKET_SPEED;

    QAngle angles;
    VectorAngles(velocity, angles);

    pRocket->InitExplosive(pOwner, velocity, angles);

    if (mom_rj_trail_sound_enable.GetBool())
    {
        pRocket->EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_ROCKETLAUNCHER, "RocketTrail"));
    }

    return pRocket;
}
#endif