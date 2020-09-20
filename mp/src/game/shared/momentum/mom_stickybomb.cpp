#include "cbase.h"

#include "mom_stickybomb.h"

#include "weapon/weapon_mom_stickybomblauncher.h"
#include "mom_shareddefs.h"
#include "weapon/weapon_def.h"

#ifndef CLIENT_DLL
#include "vcollide_parse.h"
#include "momentum/mom_triggers.h"
#include "IEffects.h"
#include "fx_mom_shared.h"
#include "physics_collisionevent.h"
#include "momentum/mom_stickybomb_verts.h"
#else
#include "functionproxy.h"
#endif

#include "tier0/memdbgon.h"

#define MOM_STICKYBOMB_RADIUS 146.0f
#define MOM_STICKYBOMB_INITIAL_SPEED 900.0f
#define MOM_STICKYBOMB_MAX_SPEED 2400.0f
#define MOM_STICKYBOMB_GRAVITY 0.5f
#define MOM_STICKYBOMB_FRICTION 0.8f
#define MOM_STICKYBOMB_ELASTICITY 0.45f
#define MOM_STICKYBOMB_ARMTIME 0.8f // Takes 0.8 seconds to arm (pulse)

IMPLEMENT_NETWORKCLASS_ALIASED(MomStickybomb, DT_MomStickybomb)

BEGIN_NETWORK_TABLE(CMomStickybomb, DT_MomStickybomb)
#ifdef CLIENT_DLL
  RecvPropInt(RECVINFO(m_fFlags)),
  RecvPropBool(RECVINFO(m_bCanExplode)),
#else
  SendPropInt(SENDINFO(m_fFlags), -1, SPROP_UNSIGNED),
  SendPropBool(SENDINFO(m_bCanExplode)),
#endif
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_stickybomb, CMomStickybomb);
PRECACHE_WEAPON_REGISTER(momentum_stickybomb);

#ifdef CLIENT_DLL
static MAKE_CONVAR(mom_sj_stickybomb_drawdelay, "0", FCVAR_ARCHIVE,
                   "Determines how long it takes for stickies to start being drawn upon spawning.\n", 0, 1);
static MAKE_TOGGLE_CONVAR(mom_sj_particle_trail_enable, "1", FCVAR_ARCHIVE,
                          "Toggles the sticky trail particle. 0 = OFF, 1 = ON\n");
#else
static MAKE_TOGGLE_CONVAR(mom_sj_decals_enable, "1", FCVAR_ARCHIVE, "Toggles creating decals on sticky explosion. 0 = OFF, 1 = ON\n");
#endif

CMomStickybomb::CMomStickybomb()
{
    m_flChargeTime = 0.0f;
    m_bCanExplode = true;

#ifdef GAME_DLL
    m_bFizzle = false;
    m_flCreationTime = 0.0f;
    m_bDidHitWorld = false;
    m_vecImpactNormal.Init();
#else
    m_bPulsed = false;
#endif
}

CMomStickybomb::~CMomStickybomb()
{
}

void CMomStickybomb::UpdateOnRemove()
{
    const auto pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(GetOriginalLauncher());
    if (pLauncher)
    {
        pLauncher->DeathNotice(this);
    }

    BaseClass::UpdateOnRemove();
}

void CMomStickybomb::Spawn()
{
    BaseClass::Spawn();

    SetRenderMode(kRenderTransColor);

#ifdef GAME_DLL
    SetModel(g_pWeaponDef->GetWeaponModel(WEAPON_STICKYLAUNCHER, "sticky"));
    
    SetSize(Vector(-2, -2, -2), Vector(2, 2, 2));

    objectparams_t params;
    params.massCenterOverride = nullptr;
    params.mass = 5.0f;
    params.inertia = 1.0f;
    params.damping = 0.0f;
    params.rotdamping = 0.0f;
    params.rotInertiaLimit = 0.05f;
    params.pName = "stickybombmod";
    params.pGameData = this;
    params.volume = 336.820007f;
    params.dragCoefficient = 1.0f;
    params.enableCollisions = true;

    const auto pNewObject = physenv->CreatePolyObject(GetStickybombCollide(), 0, GetAbsOrigin(), GetAbsAngles(), &params);

    pNewObject->EnableCollisions(true);
    pNewObject->EnableGravity(true);
    pNewObject->EnableDrag(true);
    pNewObject->EnableMotion(true);

    VPhysicsDestroyObject();

    SetSolid(SOLID_BBOX);

    VPhysicsSetObject(pNewObject);
    SetMoveType(MOVETYPE_VPHYSICS);
    pNewObject->Wake();

    m_flCreationTime = gpGlobals->curtime;
    SetGravity(MOM_STICKYBOMB_GRAVITY);
    SetFriction(MOM_STICKYBOMB_FRICTION);
    SetElasticity(MOM_STICKYBOMB_ELASTICITY);
#endif
}

bool CMomStickybomb::IsArmed() const
{
#ifdef CLIENT_DLL
    return (gpGlobals->curtime - m_flSpawnTime) >= MOM_STICKYBOMB_ARMTIME;
#else
    return (gpGlobals->curtime - m_flCreationTime) >= MOM_STICKYBOMB_ARMTIME;
#endif
}

#ifdef CLIENT_DLL

float CMomStickybomb::GetDrawDelayTime()
{
    return mom_sj_stickybomb_drawdelay.GetFloat();
}

void CMomStickybomb::CreateTrailParticles()
{
    if (!mom_sj_particle_trail_enable.GetBool())
        return;

    ParticleProp()->Create(g_pWeaponDef->GetWeaponParticle(WEAPON_STICKYLAUNCHER, "StickybombTrail"), PATTACH_ABSORIGIN_FOLLOW);
}

void CMomStickybomb::OnDataChanged(DataUpdateType_t type)
{
    BaseClass::OnDataChanged(type);

    if (type == DATA_UPDATE_CREATED)
    {
        m_bPulsed = false;

        const auto pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(GetOriginalLauncher());
        if (pLauncher)
        {
            pLauncher->AddStickybomb(this);
        }
    }
}

void CMomStickybomb::Simulate()
{
    if (!m_bPulsed && (gpGlobals->curtime - m_flSpawnTime) > (MOM_STICKYBOMB_ARMTIME - 0.25f))
    {
        ParticleProp()->Create(g_pWeaponDef->GetWeaponParticle(WEAPON_STICKYLAUNCHER, "StickybombPulse"), PATTACH_ABSORIGIN_FOLLOW);

        m_bPulsed = true;
    }

    BaseClass::Simulate();
}

class CStickybombArmTimeProxy : public CResultProxy
{
public:
    void OnBind(void *pC_BaseEntity) override
    {
        Assert(m_pResult);

        if (!pC_BaseEntity)
            return;

        const auto pEntity = BindArgToEntity(pC_BaseEntity);
        if (!pEntity)
            return;

        m_pResult->SetFloatValue(RemapValClamped(gpGlobals->curtime - pEntity->m_flSpawnTime, MOM_STICKYBOMB_ARMTIME - 0.15f, MOM_STICKYBOMB_ARMTIME, 0.0f, 1.0f));
    }
};

EXPOSE_INTERFACE(CStickybombArmTimeProxy, IMaterialProxy, "ArmTime" IMATERIAL_PROXY_INTERFACE_VERSION);

#else

CMomStickybomb *CMomStickybomb::Create(const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner)
{
    const auto pStickybomb = dynamic_cast<CMomStickybomb *>(CreateNoSpawn("momentum_stickybomb", position, angles, pOwner));

    if (pStickybomb)
    {
        DispatchSpawn(pStickybomb);
        pStickybomb->InitExplosive(pOwner, velocity, angles);
    }

    return pStickybomb;
}

void CMomStickybomb::InitExplosive(CBaseEntity *pOwner, const Vector &velocity, const QAngle &angles)
{
    BaseClass::InitExplosive(pOwner, velocity, angles);

    const AngularImpulse angVelocity(600, 0, 0);
    ApplyLocalAngularVelocityImpulse(angVelocity);

    const auto pPhysicsObject = VPhysicsGetObject();
    if (pPhysicsObject)
    {
        pPhysicsObject->AddVelocity(&velocity, &angVelocity);
    }
}

void CMomStickybomb::Destroy(bool bShowFizzleSprite)
{
    if (bShowFizzleSprite)
    {
        m_bFizzle = true;
        Dissolve(nullptr, gpGlobals->curtime, false, ENTITY_DISSOLVE_CORE);
    }
    else
    {
        BaseClass::Destroy(bShowFizzleSprite);
    }
}

void CMomStickybomb::Detonate()
{
    SetThink(nullptr);

    const Vector vecSpot = GetAbsOrigin() + Vector(0, 0, 8);
    trace_t tr;
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

    Explode(&tr, tr.m_pEnt);
}

void CMomStickybomb::Explode(trace_t *pTrace, CBaseEntity *pOther)
{
    if (CNoGrenadesZone::IsInsideNoGrenadesZone(this) || m_bFizzle)
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

    // Explosion effect on client
    Vector vecOrigin = GetAbsOrigin();
    CPVSFilter filter(vecOrigin);

    const Vector vecNormal = m_bDidHitWorld ? m_vecImpactNormal : pTrace->plane.normal;
    TE_TFExplosion(filter, vecOrigin, vecNormal, WEAPON_STICKYLAUNCHER);

    const auto pOwner = GetOwnerEntity();
    Vector vecReported = pOwner ? pOwner->GetAbsOrigin() : vec3_origin;

    // Damage
    CTakeDamageInfo info(this, pOwner, vec3_origin, vecOrigin, GetDamage(), GetDamageType(), 0, &vecReported);
    RadiusDamage(info, vecOrigin, MOM_EXPLOSIVE_RADIUS, CLASS_NONE, nullptr);

    if (mom_sj_decals_enable.GetBool() && pOther && !pOther->IsPlayer())
    {
        UTIL_DecalTrace(pTrace, "StickyScorch");
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
    if (pHitEntity && (pHitEntity->IsWorld() || bIsDynamicProp))
    {
        g_PostSimulationQueue.QueueCall(VPhysicsGetObject(), &IPhysicsObject::EnableMotion, false);

        // Save impact data for explosions.
        m_bDidHitWorld = true;
        pEvent->pInternalData->GetSurfaceNormal(m_vecImpactNormal);
        m_vecImpactNormal.Negate();
    }
}

#endif