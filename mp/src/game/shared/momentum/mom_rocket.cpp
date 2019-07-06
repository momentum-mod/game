#include "cbase.h"
#include "mom_rocket.h"

#ifndef CLIENT_DLL
#include "explode.h"
#endif

#include "tier0/memdbgon.h"

#ifndef CLIENT_DLL

BEGIN_DATADESC(CMomRocket)
    // Fields
    DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
    DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
    DEFINE_FIELD(m_flDamage, FIELD_FLOAT),

    // Functions
    DEFINE_ENTITYFUNC(Touch),
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(MomRocket, DT_MomRocket)
BEGIN_NETWORK_TABLE(CMomRocket, DT_MomRocket)
#ifdef CLIENT_DLL
RecvPropVector(RECVINFO(m_vInitialVelocity))
#else
SendPropVector(SENDINFO(m_vInitialVelocity),
               20,    // nbits
               0,     // flags
               -3000, // low value
               3000   // high value
               )
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(momentum_rocket, CMomRocket);
PRECACHE_WEAPON_REGISTER(momentum_rocket);

CMomRocket::CMomRocket()
{
    m_vInitialVelocity.Init();

#ifdef CLIENT_DLL
    m_flSpawnTime = 0.0f;
#else
    m_flDamage = 0.0f;
    m_hOwner = NULL;
    m_hRocketTrail = NULL;
#endif
}

#ifdef CLIENT_DLL

void CMomRocket::PostDataUpdate(DataUpdateType_t type)
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

int CMomRocket::DrawModel(int flags)
{
    // Don't draw rocket during the first 0.2 seconds if our own.
    if (GetOwnerEntity() == C_BasePlayer::GetLocalPlayer())
    {
        if (gpGlobals->curtime - m_flSpawnTime < 0.2)
        {
            return 0;
        }
    }

    return BaseClass::DrawModel(flags);
}

void CMomRocket::Spawn()
{
    m_flSpawnTime = gpGlobals->curtime;
    BaseClass::Spawn();
}

#else

void CMomRocket::Spawn()
{
    BaseClass::Spawn();

    SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
    SetSolidFlags(FSOLID_NOT_STANDABLE);
    SetMoveType(MOVETYPE_FLY);
    SetSolid(SOLID_BBOX); // So it will collide with physics props!
    // smaller, cube bounding box so we rest on the ground
    SetSize(Vector(-2, -2, -2), Vector(2, 2, 2));
}

void CMomRocket::Precache()
{
    BaseClass::Precache();
    // MOM_TODO:
    // Replace HL2 missile model
    PrecacheModel("models/weapons/w_missile.mdl");
    PrecacheScriptSound("Missile.Ignite");
}

void CMomRocket::SetupInitialTransmittedGrenadeVelocity(const Vector &velocity)
{
    m_vInitialVelocity = velocity;
}

void CMomRocket::Explode(trace_t *pTrace, CBaseEntity *pOther)
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

void CMomRocket::Touch(CBaseEntity *pOther)
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

void CMomRocket::CreateSmokeTrail()
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
// Output : CMomRocket
//-----------------------------------------------------------------------------
CMomRocket *CMomRocket::EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL)
{
    CMomRocket *pRocket = (CMomRocket *)CBaseEntity::CreateNoSpawn("momentum_rocket", vecOrigin, vecAngles, pentOwner);
    pRocket->SetModel("models/weapons/w_missile.mdl");
    DispatchSpawn(pRocket);

    Vector vecForward;
    AngleVectors(pRocket->GetLocalAngles(), &vecForward);
    pRocket->SetAbsVelocity(vecForward * MOM_ROCKET_SPEED);
    pRocket->SetupInitialTransmittedGrenadeVelocity(vecForward * MOM_ROCKET_SPEED);
    pRocket->SetThrower(pentOwner);

    pRocket->SetDamage(90.0f);    

    pRocket->CreateSmokeTrail();
    pRocket->EmitSound("Missile.Ignite");

    pRocket->SetTouch(&CMomRocket::Touch);
    pRocket->SetThink(NULL);

    return pRocket;
}
#endif