#include "cbase.h"

#include "mom_df_rocket.h"

#include "mom_shareddefs.h"
#include "weapon/weapon_def.h"
#include "weapon/weapon_shareddefs.h"
#include "movevars_shared.h"

#ifndef CLIENT_DLL
#include "momentum/fx_mom_shared.h"
#include "momentum/mom_triggers.h"
#endif

#include "mom_system_gamemode.h"
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomDFRocket, DT_MomDFRocket);

BEGIN_NETWORK_TABLE(CMomDFRocket, DT_MomDFRocket)
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_df_rocket, CMomDFRocket);
PRECACHE_WEAPON_REGISTER(momentum_df_rocket);

#ifdef GAME_DLL
static MAKE_TOGGLE_CONVAR(mom_df_sound_trail_enable, "1", FCVAR_ARCHIVE, "Toggles the rocket trail sound. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_df_sound_fizzle_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggles the rocket fizzle sound. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_df_decals_enable, "1", FCVAR_ARCHIVE, "Toggles creating decals on rocket explosion. 0 = OFF, 1 = ON\n");
#else
static MAKE_TOGGLE_CONVAR(mom_df_particle_trail_enable, "1", FCVAR_ARCHIVE, "Toggles the rocket trail particle. 0 = OFF, 1 = ON\n");
static MAKE_CONVAR(mom_df_rocket_drawdelay, "0.2", FCVAR_ARCHIVE, "Determines how long it takes for rockets to start being drawn after spawning.\n", 0, 1);
#endif

void CMomDFRocket::Spawn()
{
    BaseClass::Spawn();

#ifdef GAME_DLL
    SetModel(g_pWeaponDef->GetWeaponModel(WEAPON_DF_ROCKETLAUNCHER, "rocket"));

    SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
    AddEFlags(EFL_NO_WATER_VELOCITY_CHANGE);
    AddEffects(EF_BONEMERGE);
    SetSize(Vector(0, 0, 0), Vector(0, 0, 0));

    SetGravity(0.0f);

    SetTouch(&CMomDFRocket::RocketTouch);
    SetNextThink(gpGlobals->curtime);
#endif
}

#ifdef CLIENT_DLL

float CMomDFRocket::GetDrawDelayTime()
{
    return mom_df_rocket_drawdelay.GetFloat();
}

void CMomDFRocket::CreateTrailParticles()
{
    if (!mom_df_particle_trail_enable.GetBool())
        return;

    ParticleProp()->Create(g_pWeaponDef->GetWeaponParticle(WEAPON_ROCKETLAUNCHER, "RocketTrail"), PATTACH_POINT_FOLLOW, "trail");
}

#else

void CMomDFRocket::StopTrailSound()
{
    StopSound(g_pWeaponDef->GetWeaponSound(WEAPON_ROCKETLAUNCHER, "RocketTrail"));
}

float CMomDFRocket::GetDamageAmount()
{
    return damage[type];
}

void CMomDFRocket::Destroy()
{
    StopTrailSound();

    BaseClass::Destroy();
}

void CMomDFRocket::PlayFizzleSound()
{
    if (mom_df_sound_fizzle_enable.GetBool())
    {
        EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_DF_ROCKETLAUNCHER, "RocketFizzle"));
    }
}

bool CMomDFRocket::DFCanDamage(const CTakeDamageInfo &info, CBaseEntity *other, const Vector &origin)
{
    const int MASK_RADIUS_DAMAGE = MASK_SHOT & (~CONTENTS_HITBOX);
    Vector dest, midpoint;
    Vector otherMins, otherMaxs;
    trace_t trace;

    otherMins = other->GetAbsOrigin() + other->WorldAlignMins();
    otherMaxs = other->GetAbsOrigin() + other->WorldAlignMaxs();

    VectorAdd(otherMins, otherMaxs, midpoint);
    VectorScale(midpoint, 0.5f, midpoint);

    VectorCopy(midpoint, dest);
    UTIL_TraceLine(origin, dest, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &trace);
    if (trace.fraction == 1.0)
    {
        return true;
    }

    VectorCopy(midpoint, dest);
    dest[0] += 15.0;
    dest[1] += 15.0;
    UTIL_TraceLine(origin, dest, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &trace);
    if (trace.fraction == 1.0)
    {
        return true;
    }

    VectorCopy(midpoint, dest);
    dest[0] += 15.0;
    dest[1] -= 15.0;
    UTIL_TraceLine(origin, dest, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &trace);
    if (trace.fraction == 1.0)
    {
        return true;
    }

    VectorCopy(midpoint, dest);
    dest[0] -= 15.0;
    dest[1] += 15.0;
    UTIL_TraceLine(origin, dest, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &trace);
    if (trace.fraction == 1.0)
    {
        return true;
    }

    VectorCopy(midpoint, dest);
    dest[0] -= 15.0;
    dest[1] -= 15.0;
    UTIL_TraceLine(origin, dest, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &trace);
    if (trace.fraction == 1.0)
    {
        return true;
    }

    return false;
}


void CMomDFRocket::DFRadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore,
                    CBaseEntity *pEntityIgnore)
{
    CBaseEntity *pEntity = NULL;
    float flAdjustedDamage, falloff;

    Vector vecSrc = vecSrcIn;
    Vector dist;
    Vector otherMins, otherMaxs;
    Vector dir;

    vecSrc.z += 1;

    if (flRadius)
        falloff = info.GetDamage() / flRadius;
    else
        falloff = 1.0;

    for (CEntitySphereQuery sphere(vecSrc, flRadius); (pEntity = sphere.GetCurrentEntity()) != NULL;
         sphere.NextEntity())
    {
        if (pEntity == pEntityIgnore)
            continue;

        if (pEntity->m_takedamage == DAMAGE_NO)
            continue;

        otherMins = pEntity->GetAbsOrigin() + pEntity->WorldAlignMins();
        otherMaxs = pEntity->GetAbsOrigin() + pEntity->WorldAlignMaxs();

        // find the distance to the edge of the bounding box
        for (int i = 0; i < 3; i++)
        {
            if (vecSrc[i] < otherMins[i])
            {
                dist[i] = otherMins[i] - vecSrc[i];
            }
            else if (vecSrc[i] > otherMaxs[i])
            {
                dist[i] = vecSrc[i] - otherMaxs[i];
            }
            else
            {
                dist[i] = 0;
            }
        }

        if (dist.Length() > flRadius)
        {
            continue;
        }

        flAdjustedDamage = info.GetDamage() * (1.0 - (dist.Length() / flRadius));

        if (DFCanDamage(info, pEntity, vecSrc))
        {
            CTakeDamageInfo adjustedInfo = info;
            adjustedInfo.SetDamage(flAdjustedDamage);
            pEntity->TakeDamage(adjustedInfo);
        }
    }
}

void CMomDFRocket::Explode(trace_t *pTrace, CBaseEntity *pOther)
{
    if (CNoGrenadesZone::IsInsideNoGrenadesZone(this))
    {
        Fizzle();
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
    TE_TFExplosion(filter, vecOrigin, pTrace->plane.normal, WEAPON_DF_ROCKETLAUNCHER);

    // Damage
    const CTakeDamageInfo info(this, GetOwnerEntity(), vec3_origin, vecOrigin, GetDamage(), GetDamageType());
    DFRadiusDamage(info, vecOrigin, splashRadius[type], CLASS_NONE, nullptr);

    StopTrailSound();

    if (mom_df_decals_enable.GetBool() && pOther && !pOther->IsPlayer())
    {
        UTIL_DecalTrace(pTrace, "RocketScorch");
    }

    UTIL_Remove(this);
}

void CMomDFRocket::RocketTouch(CBaseEntity *pOther)
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

CMomDFRocket *CMomDFRocket::EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner,
                                       DFProjectileType_t projType)
{
    const auto pRocket = dynamic_cast<CMomDFRocket *>(CreateNoSpawn("momentum_df_rocket", vecOrigin, vecAngles, pOwner));
    if (!pRocket)
    {
        return nullptr;
    }

    pRocket->type = projType;

    DispatchSpawn(pRocket);

    Vector vecForward;
    AngleVectors(vecAngles, &vecForward);
    VectorNormalize(vecForward);

    Vector velocity = vecForward * speed[projType];
    if (projType == DF_ROCKET && sv_cpm_physics.GetBool())
    {
        velocity = vecForward * 1000;
    }

    QAngle angles;
    VectorAngles(velocity, angles);

    pRocket->InitExplosive(pOwner, velocity, angles);

    if (mom_df_sound_trail_enable.GetBool())
    {
        pRocket->EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_DF_ROCKETLAUNCHER, "RocketTrail"));
    }

    return pRocket;
}
#endif