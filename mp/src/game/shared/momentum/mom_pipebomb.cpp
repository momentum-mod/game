#include "cbase.h"
#include "mom_pipebomb.h"

#ifndef CLIENT_DLL
#include "Sprite.h"
#include "explode.h"
#include "momentum/mom_triggers.h"
#endif

#include "tier0/memdbgon.h"

#define MOM_PIPEBOMB_RADIUS 146.0f
#define MOM_PIPEBOMB_INITIAL_SPEED 900.0f
#define MOM_PIPEBOMB_MAX_SPEED 2400.0f
#define MOM_PIPEBOMB_GRAVITY 0.5f
#define MOM_PIPEBOMB_FRICTION 0.8f
#define MOM_PIPEBOMB_ELASTICITY 0.45f

#ifndef CLIENT_DLL

BEGIN_DATADESC(CMomPipebomb)
// Fields
DEFINE_FIELD(m_hOwner, FIELD_EHANDLE), DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
    DEFINE_FIELD(m_flDamage, FIELD_FLOAT), DEFINE_FIELD(m_bTouched, FIELD_BOOLEAN),

    // Functions
    DEFINE_ENTITYFUNC(Touch), END_DATADESC();
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(MomPipebomb, DT_MomPipebomb)

BEGIN_NETWORK_TABLE(CMomPipebomb, DT_MomPipebomb)
#ifdef CLIENT_DLL
RecvPropVector(RECVINFO(m_vInitialVelocity))
//RecvPropInt(RECVINFO(m_bTouched))
#else
SendPropVector(SENDINFO(m_vInitialVelocity),
               20,    // nbits
               0,     // flags
               -3000, // low value
               3000   // high value
               )
//SendPropBool(SENDINFO(m_bTouched))
#endif
    END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_pipebomb, CMomPipebomb);
PRECACHE_WEAPON_REGISTER(momentum_pipebomb);

CMomPipebomb::CMomPipebomb()
{
    m_vInitialVelocity.Init();

#ifdef CLIENT_DLL
    m_flSpawnTime = 0.0f;
#else
    m_flDamage = 0.0f;
    m_flCreationTime = 0.0f;
    m_flChargeTime = 0.0f;
    m_bTouched = false;
    m_hOwner = nullptr;
    m_hRocketTrail = nullptr;
    m_bTouched = false;
#endif
}

#ifdef CLIENT_DLL

void CMomPipebomb::PostDataUpdate(DataUpdateType_t type)
{
    BaseClass::PostDataUpdate(type);

    if (type == DATA_UPDATE_CREATED)
    {
        // Now stick our initial velocity into the interpolation history
        CInterpolatedVar<Vector> &interpolator = GetOriginInterpolator();
        interpolator.ClearHistory();

        CInterpolatedVar<QAngle> &rotInterpolator = GetRotationInterpolator();
        rotInterpolator.ClearHistory();

        float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

        // Add a sample 1 second back.
        Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
        interpolator.AddToHead(changeTime - 1.0f, &vCurOrigin, false);

        QAngle vCurAngles = GetLocalAngles();
        rotInterpolator.AddToHead(changeTime - 1.0f, &vCurAngles, false);

        // Add the current sample.
        vCurOrigin = GetLocalOrigin();
        interpolator.AddToHead(changeTime, &vCurOrigin, false);

        rotInterpolator.AddToHead(changeTime, &vCurAngles, false);
    }
}

int CMomPipebomb::DrawModel(int flags)
{
    // Don't draw the pipebomb during the first 0.1 seconds.
    if (gpGlobals->curtime - m_flSpawnTime < 0.1f)
    {
        return 0;
    }

    return BaseClass::DrawModel(flags);
}

void CMomPipebomb::Spawn()
{
    m_flSpawnTime = gpGlobals->curtime;
    BaseClass::Spawn();
}

#else

void CMomPipebomb::Spawn()
{
    BaseClass::Spawn();

    UseClientSideAnimation();
    SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
    SetSolidFlags(FSOLID_NOT_STANDABLE);
    SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
    SetSolid(SOLID_BBOX);
    AddEffects(EF_NOSHADOW);
    SetSize(Vector(-2, -2, -2), Vector(2, 2, 2));
    //UTIL_SetSize(this, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f));
    AddFlag(FL_GRENADE);

	VPhysicsInitNormal(SOLID_BBOX, 0, false);

    m_bTouched = false;
    m_flCreationTime = gpGlobals->curtime;
    m_takedamage = DAMAGE_NO;
    SetGravity(0.5f);
    SetFriction(MOM_PIPEBOMB_FRICTION);
    SetElasticity(MOM_PIPEBOMB_ELASTICITY);

    SetTouch(&CMomPipebomb::PipebombTouch);
    SetThink(&CMomPipebomb::DetonateThink);
    SetNextThink(gpGlobals->curtime + 0.2);
}

void CMomPipebomb::Precache()
{
    BaseClass::Precache();
    PrecacheModel("models/weapons/w_models/w_stickybomb.mdl");
    // PrecacheScriptSound("Pipebomb.Bounce");
}

void CMomPipebomb::SetupInitialTransmittedGrenadeVelocity(const Vector &velocity) { m_vInitialVelocity = velocity; }

void CMomPipebomb::Destroy(bool bNoGrenadeZone)
{
    SetThink(&BaseClass::SUB_Remove);
    SetNextThink(gpGlobals->curtime);
    SetTouch(NULL);
    AddEffects(EF_NODRAW);
    DestroyTrail();

    if (bNoGrenadeZone)
    {
        CSprite *pGlowSprite = CSprite::SpriteCreate(NOGRENADE_SPRITE, GetAbsOrigin(), false);
        if (pGlowSprite)
        {
            pGlowSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxFadeFast);
            pGlowSprite->SetThink(&CSprite::SUB_Remove);
            pGlowSprite->SetNextThink(gpGlobals->curtime + 1.0);
        }
    }
}

void CMomPipebomb::Pulse() 
{
    if (m_bPulsed == false)
    {
        if ((gpGlobals->curtime - m_flCreationTime) >= 0.8f)
        {
			//ParticleProp()->Create("stickybomb_pulse_red", PATTACH_ABSORIGIN);
			m_bPulsed = true;
        }
    }
}

void CMomPipebomb::RemovePipebomb(bool bBlinkOut)
{
    // Kill it
    SetThink(&BaseClass::SUB_Remove);
    SetNextThink(gpGlobals->curtime);
    SetTouch(NULL);
    AddEffects(EF_NODRAW);

    if (bBlinkOut)
    {
        // Sprite flash
        CSprite *pGlowSprite = CSprite::SpriteCreate(NOGRENADE_SPRITE, GetAbsOrigin(), false);
        if (pGlowSprite)
        {
            pGlowSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxFadeFast);
            pGlowSprite->SetThink(&CSprite::SUB_Remove);
            pGlowSprite->SetNextThink(gpGlobals->curtime + 1.0);
        }
    }
}

bool CMomPipebomb::ShouldNotDetonate(void) { return CNoGrenadesZone::IsInsideNoGrenadesZone(this); }

void CMomPipebomb::Detonate()
{
    trace_t tr;
    Vector vecSpot;

    SetThink(NULL);
    vecSpot = GetAbsOrigin() + Vector(0, 0, 0);
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

    //Explode(&tr, pOther);

    if (ShouldNotDetonate())
    {
        RemovePipebomb(true);
        return;
    }

    if (m_bFizzle)
    {
        // g_pEffects->Sparks(GetAbsOrigin());
        RemovePipebomb(true);
        return;
    }
}

void CMomPipebomb::DestroyTrail()
{
    if (m_hRocketTrail)
    {
        m_hRocketTrail->SetLifetime(0.1f);
        m_hRocketTrail->SetParent(nullptr);
        m_hRocketTrail = nullptr;
    }
}

void CMomPipebomb::Explode(trace_t *pTrace, CBaseEntity *pOther)
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

    Vector vecOrigin = GetAbsOrigin();
    CBaseEntity *pOwner = GetOwnerEntity();

    float flDamage = GetDamage();
    float flRadius = GetRadius();

    // Create explosion effect with no damage
    ExplosionCreate(vecOrigin, GetAbsAngles(), pOwner, flDamage, flRadius, false);

    // Damage
    CTakeDamageInfo info(this, pOwner, vec3_origin, vecOrigin, flDamage, GetDamageType());
    RadiusDamage(info, vecOrigin, flRadius, CLASS_NONE, nullptr);

    DestroyTrail();

    m_hOwner = nullptr;

    if (!pOther->IsPlayer())
    {
        UTIL_DecalTrace(pTrace, "Scorch");
    }

    // Remove the rocket
    UTIL_Remove(this);
}

void CMomPipebomb::Fizzle(void) { m_bFizzle = true; }

void CMomPipebomb::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
    BaseClass::VPhysicsCollision(index, pEvent);

    int otherIndex = !index;
    CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

    if (!pHitEntity)
        return;

    bool bIsDynamicProp = (NULL != dynamic_cast<CDynamicProp *>(pHitEntity));

    // Pipebombs stick to the world when they touch it
    if (pHitEntity && (pHitEntity->IsWorld() || bIsDynamicProp) && gpGlobals->curtime > 0)
    {
        m_bTouched = true;
        VPhysicsGetObject()->EnableMotion(false);

        // Save impact data for explosions.
        //m_bUseImpactNormal = true;
        //pEvent->pInternalData->GetSurfaceNormal(m_vecImpactNormal);
        //m_vecImpactNormal.Negate();
    }
}

void CMomPipebomb::PipebombTouch(CBaseEntity *pOther)
{
    Assert(pOther);

    // Don't touch triggers
    if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
        return;

    // Handle hitting skybox (disappear).
    const trace_t *pTrace = &GetTouchTrace();
    if (pTrace->surface.flags & SURF_SKY)
    {
        DestroyTrail();

        m_hOwner = nullptr;
        UTIL_Remove(this);
        return;
    }

	bool bIsDynamicProp = (NULL != dynamic_cast<CDynamicProp *>(pOther));

    // Pipebombs stick to the world when they touch it
    if (pOther && (pOther->IsWorld() || bIsDynamicProp) && gpGlobals->curtime > 0)
    {
        m_bTouched = true;
        VPhysicsGetObject()->EnableMotion(false);

        // Save impact data for explosions.
        // m_bUseImpactNormal = true;
        // pEvent->pInternalData->GetSurfaceNormal(m_vecImpactNormal);
        // m_vecImpactNormal.Negate();
    }

    // Explode
    //trace_t trace;
    //memcpy(&trace, pTrace, sizeof(trace_t));
    //Explode(&trace, pOther);
}

// TODO: Replace with pipebomb trail
void CMomPipebomb::CreateSmokeTrail()
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

void CMomPipebomb::DetonateThink(void)
{
    if (!IsInWorld())
    {
        Remove();
        return;
    }

	/*
    if (gpGlobals->curtime > m_flCollideWithTeammatesTime && m_bCollideWithTeammates == false)
    {
        m_bCollideWithTeammates = true;
    }
	*/

	/*
    if (gpGlobals->curtime > 3.0f) //m_flDetonateTime (grenade launcher timer?)
    {
        Detonate();
        return;
    }
	*/
    SetNextThink(gpGlobals->curtime + 0.2);
}

//-----------------------------------------------------------------------------
// Purpose: Spawn a new pipebomb
//
// Input  : &vecOrigin -
//          &vecAngles -
//          *pentOwner -
//
// Output : CMomPipebomb
//-----------------------------------------------------------------------------
CMomPipebomb *CMomPipebomb::EmitPipebomb(const Vector &vecOrigin, const Vector &velocity, const QAngle &vecAngles,
                                         CBaseEntity *pentOwner /*= nullptr*/)
{
    CMomPipebomb *pPipebomb =
       static_cast<CMomPipebomb *>(CreateNoSpawn("momentum_pipebomb", vecOrigin, vecAngles, pentOwner));
    AngularImpulse angVelocity =
        AngularImpulse(600, random->RandomInt(-1200, 1200), 0);
    Vector vecForward;
    AngleVectors(vecAngles, &vecForward);
    //const Vector velocity = (vecForward * 900);

    pPipebomb->SetModel("models/weapons/w_models/w_stickybomb.mdl");
    DispatchSpawn(pPipebomb);

    
	//IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
    //pPhysicsObject->AddVelocity(&velocity, &angVelocity);

    pPipebomb->SetupInitialTransmittedGrenadeVelocity(velocity);
    pPipebomb->SetAbsVelocity(velocity);
    pPipebomb->ApplyLocalAngularVelocityImpulse(angVelocity);
    pPipebomb->SetThrower(pentOwner);

    QAngle angles;
    VectorAngles(velocity, angles);
    pPipebomb->SetAbsAngles(angles);

    pPipebomb->SetDamage(112.0f);
    // NOTE: Rocket/Pipebomb explosion radius is 146.0f in TF2, but 121.0f is used for self damage (see RadiusDamage in
    // mom_gamerules)
    pPipebomb->SetRadius(146.0f);

    // pPipebomb->CreateSmokeTrail();
    return pPipebomb;
}
#endif