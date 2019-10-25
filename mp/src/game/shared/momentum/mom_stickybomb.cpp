#include "cbase.h"
#include "mom_stickybomb.h"
#include "weapon/weapon_mom_stickybomblauncher.h"

#ifndef CLIENT_DLL
#include "Sprite.h"
#include "explode.h"
#include "momentum/mom_triggers.h"
#endif

#include "tier0/memdbgon.h"

#define MOM_STICKYBOMB_RADIUS 146.0f
#define MOM_STICKYBOMB_INITIAL_SPEED 900.0f
#define MOM_STICKYBOMB_MAX_SPEED 2400.0f
#define MOM_STICKYBOMB_GRAVITY 1.0f
#define MOM_STICKYBOMB_FRICTION 0.8f
#define MOM_STICKYBOMB_ELASTICITY 0.45f

#ifndef CLIENT_DLL

    BEGIN_DATADESC(CMomStickybomb)
        // Fields
        DEFINE_FIELD(m_hOwner, FIELD_EHANDLE), DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
        DEFINE_FIELD(m_flDamage, FIELD_FLOAT), DEFINE_FIELD(m_bTouched, FIELD_BOOLEAN),

    // Functions
    DEFINE_ENTITYFUNC(Touch), END_DATADESC();
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(MomStickybomb, DT_MomStickybomb)

BEGIN_NETWORK_TABLE(CMomStickybomb, DT_MomStickybomb)
#ifdef CLIENT_DLL
RecvPropVector(RECVINFO(m_vInitialVelocity))
// RecvPropInt(RECVINFO(m_bTouched))
#else
SendPropVector(SENDINFO(m_vInitialVelocity),
               20,    // nbits
               0,     // flags
               -3000, // low value
               3000   // high value
               )
// SendPropBool(SENDINFO(m_bTouched))
#endif
    END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_stickybomb, CMomStickybomb);
PRECACHE_WEAPON_REGISTER(momentum_stickybomb);

CMomStickybomb::CMomStickybomb()
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

void CMomStickybomb::PostDataUpdate(DataUpdateType_t type)
{
    BaseClass::PostDataUpdate(type);

    if (type == DATA_UPDATE_CREATED)
    {
        m_flCreationTime = gpGlobals->curtime;
        // ParticleProp()->Create("stickybombtrail_red", PATTACH_ABSORIGIN_FOLLOW);
        m_bPulsed = false;
        // CMomentumStickybombLauncher *pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(m_hLauncher);

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

int CMomStickybomb::DrawModel(int flags)
{
    // Don't draw the stickybomb during the first 0.1 seconds.
    if (gpGlobals->curtime - m_flSpawnTime < 0.1f)
    {
        return 0;
    }

    return BaseClass::DrawModel(flags);
}

void CMomStickybomb::Spawn()
{
    m_flSpawnTime = gpGlobals->curtime;
    BaseClass::Spawn();
}

#else

void CMomStickybomb::Spawn()
{
    BaseClass::Spawn();

    UseClientSideAnimation();
    SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
    SetSolidFlags(FSOLID_NOT_STANDABLE);
    SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
    SetSolid(SOLID_BBOX);
    AddEffects(EF_NOSHADOW);
    SetSize(Vector(-2, -2, -2), Vector(2, 2, 2));
    AddFlag(FL_GRENADE);

    VPhysicsInitNormal(SOLID_BBOX, 0, false);

    m_bTouched = false;
    m_flCreationTime = gpGlobals->curtime;
    m_takedamage = DAMAGE_NO;
    SetGravity(MOM_STICKYBOMB_GRAVITY);
    SetFriction(MOM_STICKYBOMB_FRICTION);
    SetElasticity(MOM_STICKYBOMB_ELASTICITY);

    SetTouch(&CMomStickybomb::StickybombTouch);
    SetThink(&CMomStickybomb::StickybombThink);
    SetNextThink(gpGlobals->curtime + 0.2);
}

void CMomStickybomb::Precache()
{
    BaseClass::Precache();
    PrecacheModel("models/weapons/w_models/w_stickybomb.mdl");
    // PrecacheParticleSystem("stickybombtrail_red");
    // PrecacheScriptSound("Stickybomb.Bounce");
}

void CMomStickybomb::SetupInitialTransmittedGrenadeVelocity(const Vector &velocity) { m_vInitialVelocity = velocity; }

CMomStickybomb *CMomStickybomb::Create(const Vector &position, const QAngle &angles, const Vector &velocity,
                                       const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner)
{
    CMomStickybomb *pStickybomb =
        static_cast<CMomStickybomb *>(CBaseEntity::CreateNoSpawn("momentum_stickybomb", position, angles, pOwner));

    // Vector vecForward;
    // AngleVectors(vecAngles, &vecForward);

    // QAngle angles;
    // VectorAngles(velocity, angles);

    if (pStickybomb)
    {
        DispatchSpawn(pStickybomb);
        pStickybomb->SetModel("models/weapons/w_models/w_stickybomb.mdl");
        pStickybomb->InitStickybomb(velocity, angVelocity, pOwner);
        pStickybomb->ApplyLocalAngularVelocityImpulse(angVelocity);
        pStickybomb->SetAbsVelocity(velocity);
        pStickybomb->SetThrower(pOwner);
        pStickybomb->SetAbsAngles(angles);

        pStickybomb->SetDamage(120.0f);
        // NOTE: Rocket/Stickybomb explosion radius is 146.0f in TF2, but 121.0f is used for self damage (see
        // RadiusDamage in mom_gamerules)
        pStickybomb->SetRadius(146.0f);

        // pStickybomb->CreateSmokeTrail();
    }

    return pStickybomb;
}

void CMomStickybomb::InitStickybomb(const Vector &velocity, const AngularImpulse &angVelocity,
                                    CBaseCombatCharacter *pOwner)
{
    // SetOwnerEntity(NULL);
    // SetThrower(pOwner);

    SetupInitialTransmittedGrenadeVelocity(velocity);

    // SetGravity(0.4f /*BaseClass::GetGrenadeGravity()*/);
    // SetFriction(0.2f /*BaseClass::GetGrenadeFriction()*/);
    // SetElasticity(0.45f /*BaseClass::GetGrenadeElasticity()*/);

    IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
    if (pPhysicsObject)
    {
        pPhysicsObject->AddVelocity(&velocity, &angVelocity);
    }
}

void CMomStickybomb::Pulse()
{
    if (m_bPulsed == false)
    {
        if ((gpGlobals->curtime - m_flCreationTime) >= 0.8f)
        {
            // ParticleProp()->Create("stickybomb_pulse_red", PATTACH_ABSORIGIN);
            m_bPulsed = true;
        }
    }
}

void CMomStickybomb::RemoveStickybomb(bool bNoGrenadeZone)
{
    // Kill it
    SetThink(&BaseClass::SUB_Remove);
    SetNextThink(gpGlobals->curtime);
    SetTouch(NULL);
    AddEffects(EF_NODRAW);

    if (bNoGrenadeZone)
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

void CMomStickybomb::Detonate()
{
    trace_t tr;
    Vector vecSpot;

    SetThink(NULL);

    vecSpot = GetAbsOrigin() + Vector(0, 0, 0);
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

    Explode(&tr, tr.m_pEnt);

    if (CNoGrenadesZone::IsInsideNoGrenadesZone(this))
    {
        RemoveStickybomb(true);
        return;
    }

    if (m_bFizzle)
    {
        //g_pEffects->Sparks(GetAbsOrigin());
        RemoveStickybomb(true);
        return;
    }
}

void CMomStickybomb::DestroyTrail()
{
    if (m_hRocketTrail)
    {
        m_hRocketTrail->SetLifetime(0.1f);
        m_hRocketTrail->SetParent(nullptr);
        m_hRocketTrail = nullptr;
    }
}

void CMomStickybomb::Explode(trace_t *pTrace, CBaseEntity *pOther)
{
    if (CNoGrenadesZone::IsInsideNoGrenadesZone(this))
    {
        RemoveStickybomb(true);
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

	if (pOther == nullptr)
    {
        return;
	}

    //if (!pOther->IsPlayer())
    //{
    //    UTIL_DecalTrace(pTrace, "Scorch");
    //}

    UTIL_Remove(this);
}

void CMomStickybomb::Fizzle(void) { m_bFizzle = true; }

void CMomStickybomb::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
    BaseClass::VPhysicsCollision(index, pEvent);

    int otherIndex = !index;
    CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

    if (!pHitEntity)
        return;

    bool bIsDynamicProp = (NULL != dynamic_cast<CDynamicProp *>(pHitEntity));

    // Stickybombs stick to the world when they touch it
    if (pHitEntity && (pHitEntity->IsWorld() || bIsDynamicProp) && gpGlobals->curtime > 0)
    {
        m_bTouched = true;
        VPhysicsGetObject()->EnableMotion(false);

        // Save impact data for explosions.
        // m_bUseImpactNormal = true;
        // pEvent->pInternalData->GetSurfaceNormal(m_vecImpactNormal);
        // m_vecImpactNormal.Negate();
    }
}

void CMomStickybomb::StickybombTouch(CBaseEntity *pOther)
{
    Assert(pOther);

    // Don't touch triggers
    if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
        return;

    // Handle hitting skybox (disappear).
    /*
    const trace_t *pTrace = &GetTouchTrace();
    if (pTrace->surface.flags & SURF_SKY)
    {
        // DestroyTrail();

        m_hOwner = nullptr;
        UTIL_Remove(this);
        return;
    }*/
}

// TODO: Replace with stickybomb trail
void CMomStickybomb::CreateSmokeTrail()
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

void CMomStickybomb::StickybombThink()
{
    if (!IsInWorld())
    {
        Remove();
        return;
    }
    SetNextThink(gpGlobals->curtime + 0.2);
}
#endif