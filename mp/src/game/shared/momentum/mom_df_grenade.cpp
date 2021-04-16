#include "cbase.h"
#include "mom_df_grenade.h"

#include "weapon/weapon_def.h"

#ifndef CLIENT_DLL
#include "te_effect_dispatch.h"
#include "momentum/fx_mom_shared.h"
#include "momentum/mom_triggers.h"
#include "momentum/mom_player.h"
#endif

#include "tier0/memdbgon.h"

#define DF_GRENADE_GRAVITY         0.8f
#define DF_GRENADE_ELASTICITY      0.6f
#define DF_GRENADE_FRICTION        0.4f

extern short g_sModelIndexFireball;   // (in combatweapon.cpp) holds the index for the fireball
extern short g_sModelIndexWExplosion; // (in combatweapon.cpp) holds the index for the underwater explosion
extern short g_sModelIndexSmoke;      // (in combatweapon.cpp) holds the index for the smoke cloud

IMPLEMENT_NETWORKCLASS_ALIASED(MomDFGrenade, DT_MomDFGrenade);

BEGIN_NETWORK_TABLE(CMomDFGrenade, DT_MomDFGrenade)
#ifdef CLIENT_DLL
    RecvPropVector(RECVINFO(m_vInitialVelocity))
#else
    SendPropVector(SENDINFO(m_vInitialVelocity), 20, 0, -3000, 3000)
#endif
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_df_grenade, CMomDFGrenade);
PRECACHE_WEAPON_REGISTER(momentum_df_grenade);

void CMomDFGrenade::Spawn()
{
    BaseClass::Spawn();
#ifdef CLIENT_DLL
    m_flSpawnTime = gpGlobals->curtime;
#else
    SetModel(g_pWeaponDef->GetWeaponModel(WEAPON_GRENADE, "world"));

    SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
    SetSolidFlags(FSOLID_NOT_STANDABLE);
    SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
    SetSolid(SOLID_BBOX); // So it will collide with physics props!
    // smaller, cube bounding box so we rest on the ground
    SetSize(Vector(-2, -2, -2), Vector(2, 2, 2));
#endif
}

#ifdef CLIENT_DLL

void CMomDFGrenade::PostDataUpdate(DataUpdateType_t type)
{
    BaseClass::PostDataUpdate(type);

    if (type == DATA_UPDATE_CREATED)
    {
        // Now stick our initial velocity into the interpolation history
        CInterpolatedVar<Vector> &interpolator = GetOriginInterpolator();

        interpolator.ClearHistory();
        float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

        // Add a sample 1 second back.
        Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
        interpolator.AddToHead(changeTime - 1.0, &vCurOrigin, false);

        // Add the current sample.
        vCurOrigin = GetLocalOrigin();
        interpolator.AddToHead(changeTime, &vCurOrigin, false);
    }
}

#else

CMomDFGrenade *CMomDFGrenade::Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner)
{
    const auto pGrenade = dynamic_cast<CMomDFGrenade *>(CreateNoSpawn("momentum_df_grenade", position, angles, pOwner));
    DispatchSpawn(pGrenade);

    pGrenade->SetDetonateTimerLength(2.5f);
    pGrenade->SetAbsVelocity(velocity);
    pGrenade->SetupInitialTransmittedGrenadeVelocity(velocity);
    pGrenade->SetThrower(pOwner);

    pGrenade->SetGravity(DF_GRENADE_GRAVITY);
    pGrenade->SetFriction(DF_GRENADE_FRICTION);
    pGrenade->SetElasticity(DF_GRENADE_ELASTICITY);

    if (pOwner->IsPlayer())
    {
        const auto pMomPlayer = static_cast<CMomentumPlayer*>(pOwner);
        pGrenade->SetDamage(pMomPlayer->m_bHasPracticeMode ? 0.0f : 100.0f);
    }
    else
    {
        pGrenade->SetDamage(0.0f);
    }

    pGrenade->SetDamageRadius(150);
    pGrenade->ApplyLocalAngularVelocityImpulse(angVelocity);

    pGrenade->SetThink(&CMomDFGrenade::DetonateThink);
    pGrenade->SetNextThink(gpGlobals->curtime);

    return pGrenade;
}

void CMomDFGrenade::DetonateThink()
{
    if (gpGlobals->curtime > m_flDetonateTime)
    {
        trace_t tr;
        Vector vecSpot; // trace starts here!

        SetThink(NULL);

        vecSpot = GetAbsOrigin() + Vector(0, 0, 8);
        UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

        if (tr.startsolid)
        {
            // Since we blindly moved the explosion origin vertically, we may have inadvertently moved the explosion
            // into a solid, in which case nothing is going to be harmed by the grenade's explosion because all
            // subsequent traces will startsolid. If this is the case, we do the downward trace again from the actual
            // origin of the grenade. (sjb) 3/8/2007  (for ep2_outland_09)
            UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -32), MASK_SHOT_HULL, this,
                           COLLISION_GROUP_NONE, &tr);
        }

        Explode(&tr);
        return;
    }

    SetNextThink(gpGlobals->curtime + 0.2);
}

void CMomDFGrenade::SetDetonateTimerLength(float timer)
{
    m_flDetonateTime = gpGlobals->curtime + timer;
}

void CMomDFGrenade::ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity)
{
    // Assume all surfaces have the same elasticity
    float flSurfaceElasticity = 1.0f;

    // if its breakable glass and we kill it, don't bounce.
    // give some damage to the glass, and if it breaks, pass
    // through it.
    bool breakthrough = false;

    if (trace.m_pEnt && FClassnameIs(trace.m_pEnt, "func_breakable"))
    {
        breakthrough = true;
    }

    if (trace.m_pEnt && FClassnameIs(trace.m_pEnt, "func_breakable_surf"))
    {
        breakthrough = true;
    }

    if (breakthrough)
    {
        CTakeDamageInfo info(this, this, 10, DMG_CLUB);
        trace.m_pEnt->DispatchTraceAttack(info, GetAbsVelocity(), &trace);

        ApplyMultiDamage();

        if (trace.m_pEnt->m_iHealth <= 0)
        {
            // slow our flight a little bit
            Vector vel = GetAbsVelocity() * 0.4f;

            SetAbsVelocity(vel);
            return;
        }
    }

    float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
    flTotalElasticity = clamp(flTotalElasticity, 0.0f, 0.9f);

    // NOTE: A backoff of 2.0f is a reflection
    Vector vecAbsVelocity;
    PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f);
    vecAbsVelocity *= flTotalElasticity;

    // Get the total velocity (player + conveyors, etc.)
    VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);
    float flSpeedSqr = DotProduct(vecVelocity, vecVelocity);

    // Stop if on ground.
    if (trace.plane.normal.z > 0.7f) // Floor
    {
        // Verify that we have an entity.
        CBaseEntity *pEntity = trace.m_pEnt;
        Assert(pEntity);

        SetAbsVelocity(vecAbsVelocity);

        if (flSpeedSqr < (30 * 30))
        {
            if (pEntity->IsStandable())
            {
                SetGroundEntity(pEntity);
            }

            // Reset velocities.
            SetAbsVelocity(vec3_origin);
            SetLocalAngularVelocity(vec3_angle);

            // align to the ground so we're not standing on end
            QAngle angle;
            VectorAngles(trace.plane.normal, angle);

            // rotate randomly in yaw
            angle[1] = random->RandomFloat(0, 360);

            // TODO: rotate around trace.plane.normal

            SetAbsAngles(angle);
        }
        else
        {
            Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;
            Vector vecBaseDir = GetBaseVelocity();
            VectorNormalize(vecBaseDir);
            float flScale = vecDelta.Dot(vecBaseDir);

            VectorScale(vecAbsVelocity, (1.0f - trace.fraction) * gpGlobals->frametime, vecVelocity);
            VectorMA(vecVelocity, (1.0f - trace.fraction) * gpGlobals->frametime, GetBaseVelocity() * flScale,
                     vecVelocity);
            PhysicsPushEntity(vecVelocity, &trace);
        }
    }
    else
    {
        // If we get *too* slow, we'll stick without ever coming to rest because
        // we'll get pushed down by gravity faster than we can escape from the wall.
        if (flSpeedSqr < (30 * 30))
        {
            // Reset velocities.
            SetAbsVelocity(vec3_origin);
            SetLocalAngularVelocity(vec3_angle);
        }
        else
        {
            SetAbsVelocity(vecAbsVelocity);
        }
    }

    EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_GRENADE, "bounce"));
}

void CMomDFGrenade::SetupInitialTransmittedGrenadeVelocity(const Vector &velocity)
{
    m_vInitialVelocity = velocity;
}

bool CMomDFGrenade::DFCanDamage(const CTakeDamageInfo &info, CBaseEntity *other, const Vector &origin)
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

void CMomDFGrenade::DFRadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore,
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
                dist[i] = otherMins[i] - vecSrc[i];
            if (vecSrc[i] > otherMaxs[i])
                dist[i] = vecSrc[i] - otherMaxs[i];
            else
                dist[i] = 0;
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

void CMomDFGrenade::Explode(trace_t *pTrace)
{
    if (CNoGrenadesZone::IsInsideNoGrenadesZone(this))
    {
        //Fizzle();
        UTIL_Remove(this);
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
    //CPVSFilter filter(vecOrigin);
    //TE_TFExplosion(filter, vecOrigin, pTrace->plane.normal, WEAPON_DF_ROCKETLAUNCHER);
    
	int contents = UTIL_PointContents(vecOrigin);
    Vector vecNormal = pTrace->plane.normal;
	surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
	CPASFilter filter( vecOrigin );

	te->Explosion( filter, -1.0, // don't apply cl_interp delay
		&vecOrigin,
		!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
		m_DmgRadius * .03, 
		25,
		TE_EXPLFLAG_NONE,
		m_DmgRadius,
		m_flDamage,
		&vecNormal,
		(char) pdata->game.material );

    // Damage
    const CTakeDamageInfo info(this, GetOwnerEntity(), vec3_origin, vecOrigin, 100, GetDamageType());
    DFRadiusDamage(info, vecOrigin, 150, CLASS_NONE, nullptr);
       
    UTIL_DecalTrace(pTrace, "RocketScorch");

    UTIL_Remove(this);
}

#define MAX_WATER_SURFACE_DISTANCE 512

void CMomDFGrenade::Splash()
{
    Vector centerPoint = GetAbsOrigin();
    Vector normal(0, 0, 1);

    // Find our water surface by tracing up till we're out of the water
    trace_t tr;
    Vector vecTrace(0, 0, MAX_WATER_SURFACE_DISTANCE);
    UTIL_TraceLine(centerPoint, centerPoint + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr);

    // If we didn't start in water, we're above it
    if (tr.startsolid == false)
    {
        // Look downward to find the surface
        vecTrace.Init(0, 0, -MAX_WATER_SURFACE_DISTANCE);
        UTIL_TraceLine(centerPoint, centerPoint + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr);

        // If we hit it, setup the explosion
        if (tr.fraction < 1.0f)
        {
            centerPoint = tr.endpos;
        }
        else
        {
            // NOTENOTE: We somehow got into a splash without being near water?
            Assert(0);
        }
    }
    else if (tr.fractionleftsolid)
    {
        // Otherwise we came out of the water at this point
        centerPoint = centerPoint + (vecTrace * tr.fractionleftsolid);
    }
    else
    {
        // Use default values, we're really deep
    }

    CEffectData data;
    data.m_vOrigin = centerPoint;
    data.m_vNormal = normal;
    data.m_flScale = random->RandomFloat(1.0f, 2.0f);

    if (GetWaterType() & CONTENTS_SLIME)
    {
        data.m_fFlags |= FX_WATER_IN_SLIME;
    }

    DispatchEffect("gunshotsplash", data);
}
#endif