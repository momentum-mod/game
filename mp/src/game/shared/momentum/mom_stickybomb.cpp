#include "cbase.h"
#include "mom_stickybomb.h"
#include "weapon/weapon_mom_stickybomblauncher.h"
#include "mom_shareddefs.h"
#include "weapon/weapon_def.h"

#ifndef CLIENT_DLL
#include "Sprite.h"
#include "explode.h"
#include "momentum/mom_triggers.h"
#include "IEffects.h"
#include "fx_mom_shared.h"
#include "physics_collisionevent.h"
#endif

#include "tier0/memdbgon.h"

#define MOM_STICKYBOMB_MODEL "models/weapons/w_models/w_stickybomb.mdl"
#define MOM_STICKYBOMB_RADIUS 146.0f
#define MOM_STICKYBOMB_INITIAL_SPEED 900.0f
#define MOM_STICKYBOMB_MAX_SPEED 2400.0f
#define MOM_STICKYBOMB_GRAVITY 0.5f
#define MOM_STICKYBOMB_FRICTION 0.8f
#define MOM_STICKYBOMB_ELASTICITY 0.45f

#ifndef CLIENT_DLL
BEGIN_DATADESC(CMomStickybomb)
    // Fields
    DEFINE_FIELD(m_hLauncher, FIELD_EHANDLE),
    DEFINE_FIELD(m_flDamage, FIELD_FLOAT),
    DEFINE_FIELD(m_bTouched, FIELD_BOOLEAN),
    DEFINE_FIELD(m_bPulsed, FIELD_BOOLEAN),

    // Functions
    DEFINE_ENTITYFUNC(Touch),
END_DATADESC();
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(MomStickybomb, DT_MomStickybomb)

BEGIN_NETWORK_TABLE(CMomStickybomb, DT_MomStickybomb)
#ifdef CLIENT_DLL
RecvPropVector(RECVINFO(m_vInitialVelocity)),
RecvPropInt(RECVINFO(m_bTouched)),
RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
RecvPropEHandle(RECVINFO(m_hLauncher)),
#else
SendPropVector(SENDINFO(m_vInitialVelocity), 20, 0, -3000, 3000),
SendPropExclude("DT_BaseEntity", "m_vecOrigin"),
SendPropBool(SENDINFO(m_bTouched)),
SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD_MP_INTEGRAL | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
SendPropEHandle(SENDINFO(m_hLauncher)),
#endif
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_stickybomb, CMomStickybomb);
PRECACHE_WEAPON_REGISTER(momentum_stickybomb);

#ifdef CLIENT_DLL
static MAKE_CONVAR(mom_sj_stickybomb_drawdelay, "0", FCVAR_ARCHIVE,
                   "Determines how long it takes for stickies to start being drawn upon spawning.\n", 0, 1);
#endif

CMomStickybomb::CMomStickybomb()
{
    m_vInitialVelocity.Init();
    m_bTouched = false;
    m_flChargeTime = 0.0f;
#ifdef CLIENT_DLL
    m_flSpawnTime = 0.0f;
#else
    m_flDamage = 0.0f;
    m_flCreationTime = 0.0f;
    m_bUseImpactNormal = false;
    m_vecImpactNormal.Init();
#endif
}

CMomStickybomb::~CMomStickybomb()
{
}

void CMomStickybomb::UpdateOnRemove()
{
    // Tell our launcher that we were removed
    CMomentumStickybombLauncher *pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(m_hLauncher.Get());

    if (pLauncher)
    {
        pLauncher->DeathNotice(this);
    }

    BaseClass::UpdateOnRemove();
}

void CMomStickybomb::Spawn()
{
    BaseClass::Spawn();
#ifdef CLIENT_DLL
    m_flSpawnTime = gpGlobals->curtime;
#else
    SetModel(MOM_STICKYBOMB_MODEL);

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
#endif
}

void CMomStickybomb::Precache()
{
    BaseClass::Precache();

    PrecacheModel(MOM_STICKYBOMB_MODEL);
}

#ifdef CLIENT_DLL

void CMomStickybomb::OnDataChanged(DataUpdateType_t type)
{
    BaseClass::OnDataChanged(type);

    if (type == DATA_UPDATE_CREATED)
    {
        m_flCreationTime = gpGlobals->curtime;

        const auto pWepScript = g_pWeaponDef->GetWeaponScript(WEAPON_STICKYLAUNCHER);

        const char *pParticle = pWepScript->pKVWeaponParticles->GetString("StickybombTrail_TF2");

        ParticleProp()->Create(pParticle, PATTACH_ABSORIGIN_FOLLOW);
        m_bPulsed = false;

        CMomentumStickybombLauncher *pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(m_hLauncher.Get());

        if (pLauncher)
        {
            pLauncher->AddStickybomb(this);
        }

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
    // Don't draw the stickybomb during the time defined by the cvar.
    if (gpGlobals->curtime - m_flSpawnTime < mom_sj_stickybomb_drawdelay.GetFloat())
    {
        return 0;
    }

    return BaseClass::DrawModel(flags);
}

void CMomStickybomb::Simulate()
{
    if (!m_bPulsed)
    {
        if ((gpGlobals->curtime - m_flCreationTime) >= 0.8f)
        {
            const auto pWepScript = g_pWeaponDef->GetWeaponScript(WEAPON_STICKYLAUNCHER);

            const char *pParticle = pWepScript->pKVWeaponParticles->GetString("StickybombPulse_TF2");

            ParticleProp()->Create(pParticle, PATTACH_ABSORIGIN_FOLLOW);
            m_bPulsed = true;
        }
    }
    BaseClass::Simulate();
}

#else

void CMomStickybomb::SetupInitialTransmittedGrenadeVelocity(const Vector &velocity) { m_vInitialVelocity = velocity; }

CMomStickybomb *CMomStickybomb::Create(const Vector &position, const QAngle &angles, const Vector &velocity,
                                       const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner)
{
    CMomStickybomb *pStickybomb =
        static_cast<CMomStickybomb *>(CBaseEntity::CreateNoSpawn("momentum_stickybomb", position, angles, pOwner));

    if (pStickybomb)
    {
        DispatchSpawn(pStickybomb);
        pStickybomb->InitStickybomb(velocity, angVelocity);
        pStickybomb->ApplyLocalAngularVelocityImpulse(angVelocity);
        pStickybomb->SetAbsVelocity(velocity);
        pStickybomb->SetAbsAngles(angles);

        pStickybomb->SetDamage(120.0f);
        pStickybomb->SetRadius(146.0f);
    }

    return pStickybomb;
}

void CMomStickybomb::InitStickybomb(const Vector &velocity, const AngularImpulse &angVelocity)
{
    SetupInitialTransmittedGrenadeVelocity(velocity);

    IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
    if (pPhysicsObject)
    {
        pPhysicsObject->AddVelocity(&velocity, &angVelocity);
    }
}

void CMomStickybomb::RemoveStickybomb(bool bNoGrenadeZone)
{
    // Kill it
    SetThink(&BaseClass::SUB_Remove);
    SetNextThink(gpGlobals->curtime);
    SetTouch(nullptr);
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

    SetThink(nullptr);

    vecSpot = GetAbsOrigin() + Vector(0, 0, 8);
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

    Explode(&tr, tr.m_pEnt);
}

void CMomStickybomb::Explode(trace_t *pTrace, CBaseEntity *pOther)
{
    if (CNoGrenadesZone::IsInsideNoGrenadesZone(this))
    {
        RemoveStickybomb(true);
        return;
    }

    if (m_bFizzle)
    {
        g_pEffects->Sparks(GetAbsOrigin());
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

    CBaseEntity *pOwner = GetOwnerEntity();

    float flDamage = GetDamage();
    float flRadius = GetRadius();

    // Explosion effect on client
    Vector vecOrigin = GetAbsOrigin();
    CPVSFilter filter(vecOrigin);

    const Vector vecNormal = m_bUseImpactNormal ? m_vecImpactNormal : pTrace->plane.normal;
    TE_TFExplosion(filter, vecOrigin, vecNormal, WEAPON_STICKYLAUNCHER);

    Vector vecReported = pOwner ? pOwner->GetAbsOrigin() : vec3_origin;

    // Damage
    CTakeDamageInfo info(this, pOwner, vec3_origin, vecOrigin, flDamage, GetDamageType(), 0, &vecReported);
    RadiusDamage(info, vecOrigin, flRadius, CLASS_NONE, nullptr);

    if (pOther && !pOther->IsPlayer())
    {
        UTIL_DecalTrace(pTrace, "Scorch");
    }

    SetThink(&CMomStickybomb::SUB_Remove);
    SetTouch(nullptr);

    UTIL_Remove(this);
}

void CMomStickybomb::Fizzle() { m_bFizzle = true; }

void CMomStickybomb::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
    BaseClass::VPhysicsCollision(index, pEvent);

    int otherIndex = !index;
    CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

    if (!pHitEntity)
        return;

    // Handle hitting skybox (bounce off).
    surfacedata_t *pprops = physprops->GetSurfaceData(pEvent->surfaceProps[otherIndex]);
    if (pprops->game.material == 'X')
    {
        return;
    }

    bool bIsDynamicProp = (nullptr != dynamic_cast<CDynamicProp *>(pHitEntity));

    // Stickybombs stick to the world when they touch it
    if (pHitEntity && (pHitEntity->IsWorld() || bIsDynamicProp) && gpGlobals->curtime > 0)
    {
        m_bTouched = true;

        g_PostSimulationQueue.QueueCall(VPhysicsGetObject(), &IPhysicsObject::EnableMotion, false);

        // Save impact data for explosions.
        m_bUseImpactNormal = true;
        pEvent->pInternalData->GetSurfaceNormal(m_vecImpactNormal);
        m_vecImpactNormal.Negate();
    }
}
#endif