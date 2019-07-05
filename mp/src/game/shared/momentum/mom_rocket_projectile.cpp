#include "cbase.h"
#include "mom_rocket_projectile.h"

#ifndef CLIENT_DLL
#include "explode.h"
#endif

#include "tier0/memdbgon.h"

#ifndef CLIENT_DLL

BEGIN_DATADESC(CMomentumRocket)
    // Fields
    DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
    DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
    DEFINE_FIELD(m_flDamage, FIELD_FLOAT),

    // Functions
    DEFINE_FUNCTION(Touch),
END_DATADESC()

LINK_ENTITY_TO_CLASS(momentum_rocket, CMomentumRocket);

CMomentumRocket::CMomentumRocket()
{
    m_hRocketTrail = NULL;
    m_flDamage = 0.0f;
}

void CMomentumRocket::Precache()
{
    // MOM_TODO:
    // Replace HL2 missile model
    PrecacheModel("models/weapons/w_missile.mdl");
    PrecacheScriptSound("Missile.Ignite");
}

void CMomentumRocket::Spawn()
{
    Precache();

    SetSolid(SOLID_BBOX);
    SetMoveType(MOVETYPE_FLY);
    SetModel("models/weapons/w_missile.mdl");
    EmitSound("Missile.Ignite");

    Vector vecForward;
    AngleVectors(GetLocalAngles(), &vecForward);
    SetAbsVelocity(vecForward * MOM_ROCKET_SPEED);

    SetDamage(90.0f);

    SetTouch(&CMomentumRocket::Touch);
    SetThink(NULL);

    CreateSmokeTrail();
}

void CMomentumRocket::Explode(trace_t *pTrace, CBaseEntity *pOther)
{
    // Make invisible
    SetModelName(NULL_STRING);
    SetSolid(SOLID_NONE);
    m_takedamage = DAMAGE_NO;

    // Pull out a bit
    if (pTrace->fraction != 1.0)
    {
        SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
    }

    Vector vecOrigin = GetAbsOrigin();
    CBaseEntity *pOwner = GetOwnerEntity();

    float flDamage = GetDamage();
    float flRadius = GetRadius();

    // Create explosion effect with no damage
    ExplosionCreate(vecOrigin, GetAbsAngles(), pOwner, flDamage, flRadius, false);

    // Damage
    CTakeDamageInfo info(this, pOwner, vec3_origin, vecOrigin, flDamage, GetDamageType());
    RadiusDamage(info, vecOrigin, flRadius, CLASS_NONE, NULL);

    if (m_hRocketTrail)
    {
        m_hRocketTrail->SetLifetime(0.1f);
        m_hRocketTrail->SetParent(NULL);
        m_hRocketTrail = NULL;
    }

    m_hOwner = NULL;

    StopSound("Missile.Ignite");

    // Remove the rocket
    UTIL_Remove(this);
}

void CMomentumRocket::Touch(CBaseEntity *pOther)
{
    Assert(pOther);

    // Don't touch triggers
    if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
        return;

    // Handle hitting skybox (disappear).
    const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
    if (pTrace->surface.flags & SURF_SKY)
    {
        if (m_hRocketTrail)
        {
            m_hRocketTrail->SetLifetime(0.1f);
            m_hRocketTrail->SetParent(NULL);
            m_hRocketTrail = NULL;
        }

        m_hOwner = NULL;
        StopSound("Missile.Ignite");
        UTIL_Remove(this);
        return;
    }

    // Explode
    trace_t trace;
    memcpy(&trace, pTrace, sizeof(trace_t));
    Explode(&trace, pOther);
}

void CMomentumRocket::CreateSmokeTrail()
{
    if (m_hRocketTrail)
        return;

    // Smoke trail.
    if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
    {
        m_hRocketTrail->m_Opacity = 0.2f;
        m_hRocketTrail->m_SpawnRate = 100;
        m_hRocketTrail->m_ParticleLifetime = 0.5f;
        m_hRocketTrail->m_StartColor.Init(0.65f, 0.65f, 0.65f);
        m_hRocketTrail->m_EndColor.Init(0.0, 0.0, 0.0);
        m_hRocketTrail->m_StartSize = 8;
        m_hRocketTrail->m_EndSize = 32;
        m_hRocketTrail->m_SpawnRadius = 4;
        m_hRocketTrail->m_MinSpeed = 2;
        m_hRocketTrail->m_MaxSpeed = 16;

        m_hRocketTrail->SetLifetime(999);
        m_hRocketTrail->FollowEntity(this, "0");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Spawn a new rocket
//
// Input  : &vecOrigin -
//          &vecAngles -
//          *pentOwner -
//
// Output : CMomentumRocket
//-----------------------------------------------------------------------------
CMomentumRocket *CMomentumRocket::EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL)
{
    CMomentumRocket *pRocket = (CMomentumRocket *)CBaseEntity::Create("momentum_rocket", vecOrigin, vecAngles, pentOwner);
    pRocket->Spawn();
    return pRocket;
}
#endif