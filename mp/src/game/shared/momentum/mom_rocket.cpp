#include "cbase.h"
#include "mom_rocket.h"
#include "engine/IEngineSound.h"
#include "mom_shareddefs.h"
#include "weapon/mom_weapon_parse.h"

#ifndef CLIENT_DLL
#include "Sprite.h"
#include "explode.h"
#include "momentum/fx_mom_shared.h"
#include "momentum/mom_triggers.h"
#include "mom_player_shared.h"
#endif

#include "tier0/memdbgon.h"

#define MOM_ROCKET_RADIUS 146.0f
#define MOM_ROCKET_SPEED 1100.0f
#define MOM_ROCKET_MODEL "models/weapons/w_missile.mdl"
#define TF_ROCKET_MODEL "models/weapons/w_models/w_rocket.mdl"

#ifndef CLIENT_DLL

BEGIN_DATADESC(CMomRocket)
    // Fields
    DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
    DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
    DEFINE_FIELD(m_flDamage, FIELD_FLOAT),

    // Functions
    DEFINE_ENTITYFUNC(Touch),
END_DATADESC();
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
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(momentum_rocket, CMomRocket);
PRECACHE_WEAPON_REGISTER(momentum_rocket);

#ifdef GAME_DLL
static MAKE_TOGGLE_CONVAR(mom_rj_use_tf_rocketmodel, "0", FCVAR_ARCHIVE,
                          "Toggles between the TF2 rocket model and the Momentum one. 0 = Momentum, 1 = TF2\n");
static MAKE_CONVAR(mom_rj_particles, "1", FCVAR_ARCHIVE,
                   "Toggles between the TF2 particles for explosions and the Momentum ones. 0 = None, 1 = Momentum, 2 = TF2\n", 0, 2);
static MAKE_CONVAR(mom_rj_trail, "1", FCVAR_ARCHIVE,
                   "Toggles between the TF2 rocket trail and the Momentum one. 0 = None, 1 = Momentum, 2 = TF2\n", 0, 2);
#endif

CMomRocket::CMomRocket()
{
    m_vInitialVelocity.Init();

#ifdef CLIENT_DLL
    m_flSpawnTime = 0.0f;
#else
    m_flDamage = 0.0f;
    m_hOwner = nullptr;
    m_hRocketTrail = nullptr;
#endif
}

CMomRocket::~CMomRocket()
{
#ifdef CLIENT_DLL
    ParticleProp()->StopEmission();
#endif
}

bool CMomRocket::UseTFTrail()
{
#ifdef CLIENT_DLL
    static ConVarRef mom_rj_trail("mom_rj_trail");
#endif
    return mom_rj_trail.GetInt() == 2;
}

#ifdef CLIENT_DLL

void CMomRocket::PostDataUpdate(DataUpdateType_t type)
{
    BaseClass::PostDataUpdate(type);

    if (type == DATA_UPDATE_CREATED)
    {
        if (UseTFTrail())
        {
            static ConVarRef mom_rj_use_tf_rocketmodel("mom_rj_use_tf_rocketmodel");
            bool bIsMomModel = !mom_rj_use_tf_rocketmodel.GetBool();
            // If the Momentum rocket model is used, the attachment point for particle systems is called "0" instead of "trail"
            const char *pAttachmentName = bIsMomModel ? "0" : "trail";

            if (enginetrace->GetPointContents(GetAbsOrigin()) & MASK_WATER)
            {
                ParticleProp()->Create("rockettrail_underwater", PATTACH_POINT_FOLLOW, pAttachmentName);
            }
            else
            {
                ParticleProp()->Create("rockettrail", PATTACH_POINT_FOLLOW, pAttachmentName);
            }
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

int CMomRocket::DrawModel(int flags)
{
    // Don't draw rocket during the first 0.2 seconds.
    if (gpGlobals->curtime - m_flSpawnTime < 0.2f)
    {
        return 0;
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

    UseClientSideAnimation();
    SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
    SetSolidFlags(FSOLID_NOT_STANDABLE);
    SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
    SetSolid(SOLID_BBOX);
    AddEFlags(EFL_NO_WATER_VELOCITY_CHANGE);
    AddEffects(EF_NOSHADOW);
    SetSize(Vector(0, 0, 0), Vector(0, 0, 0));
    AddFlag(FL_GRENADE);

    m_takedamage = DAMAGE_NO;
    SetGravity(0.0f);

    SetTouch(&CMomRocket::RocketTouch);
    SetNextThink(gpGlobals->curtime);
}

void CMomRocket::Precache()
{
    BaseClass::Precache();
    PrecacheModel(MOM_ROCKET_MODEL);
    PrecacheModel(TF_ROCKET_MODEL);
    PrecacheScriptSound("Missile.Ignite");
    PrecacheScriptSound("BaseExplosionEffect.Sound");
    PrecacheScriptSound("BaseExplosionEffect.SoundTF2");
}

void CMomRocket::SetupInitialTransmittedGrenadeVelocity(const Vector &velocity) { m_vInitialVelocity = velocity; }

void CMomRocket::Destroy(bool bNoGrenadeZone)
{
    SetThink(&BaseClass::SUB_Remove);
    SetNextThink(gpGlobals->curtime);
    SetTouch(NULL);
    AddEffects(EF_NODRAW);
    StopSound("Missile.Ignite");
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

void CMomRocket::DestroyTrail()
{
    if (UseTFTrail())
        return;

    if (m_hRocketTrail)
    {
        m_hRocketTrail->SetLifetime(0.1f);
        m_hRocketTrail->SetParent(nullptr);
        m_hRocketTrail = nullptr;
    }
}

void CMomRocket::CreateRocketExplosionEffect(trace_t *pTrace, CBaseEntity *pOther)
{
    static ConVarRef mom_rj_sounds("mom_rj_sounds");
    int iEntIndex = pOther->entindex();
    CWeaponID m_hWeaponID = WEAPON_ROCKETLAUNCHER;
    Vector vecOrigin = GetAbsOrigin();
    CBaseEntity *pOwner = GetOwnerEntity();
    float flDamage = GetDamage();
    float flRadius = GetRadius();
    CPVSFilter filter(vecOrigin);

    switch (mom_rj_particles.GetInt())
    {
    case 1:
        if (mom_rj_sounds.GetInt() == 0)
        {
            // Silent explosion
            ExplosionCreate(vecOrigin, GetAbsAngles(), pOwner, flDamage, flRadius, false, 0.0f, false, true);
        }
        else if (mom_rj_sounds.GetInt() == 1)
        {
            ExplosionCreate(vecOrigin, GetAbsAngles(), pOwner, flDamage, flRadius, false);
        }
        else if (mom_rj_sounds.GetInt() == 2)
        {
            // Small hack: If TF2 sounds are selected but Momentum particles, then use silent standard explosion effect and play TF2 explosion sound
            ExplosionCreate(vecOrigin, GetAbsAngles(), pOwner, flDamage, flRadius, false, 0.0f, false, true);
            CBaseEntity::EmitSound(filter, SOUND_FROM_WORLD, "BaseExplosionEffect.SoundTF2", &vecOrigin);
        }
        break;
    case 0:
    case 2:
        TE_TFExplosion(filter, 0.0f, vecOrigin, pTrace->plane.normal, m_hWeaponID, iEntIndex);
        break;
    }
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

    Vector vecOrigin = GetAbsOrigin();
    CBaseEntity *pOwner = GetOwnerEntity();

    float flDamage = GetDamage();
    float flRadius = GetRadius();

    // Create explosion effect with no damage depending on the particle cvar setting, see method declaration
    CreateRocketExplosionEffect(pTrace, pOther);

    // Damage
    CTakeDamageInfo info(this, pOwner, vec3_origin, vecOrigin, flDamage, GetDamageType());
    RadiusDamage(info, vecOrigin, flRadius, CLASS_NONE, nullptr);

    DestroyTrail();

    m_hOwner = nullptr;

    StopSound("Missile.Ignite");

    if (!pOther->IsPlayer())
    {
        UTIL_DecalTrace(pTrace, "Scorch");
    }

    // Remove the rocket
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
        DestroyTrail();

        m_hOwner = nullptr;
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
CMomRocket *CMomRocket::EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner)
{
    CMomRocket *pRocket = static_cast<CMomRocket *>(CreateNoSpawn("momentum_rocket", vecOrigin, vecAngles, pOwner));

    if (!mom_rj_use_tf_rocketmodel.GetBool())
    {
        pRocket->SetModel(MOM_ROCKET_MODEL);
    }
    else
    {
        pRocket->SetModel(TF_ROCKET_MODEL);
    }

    DispatchSpawn(pRocket);

    Vector vecForward;
    AngleVectors(vecAngles, &vecForward);

    const Vector velocity = vecForward * MOM_ROCKET_SPEED;
    pRocket->SetAbsVelocity(velocity);
    pRocket->SetupInitialTransmittedGrenadeVelocity(velocity);
    pRocket->SetThrower(pOwner);

    QAngle angles;
    VectorAngles(velocity, angles);
    pRocket->SetAbsAngles(angles);

    if (pOwner->IsPlayer())
    {
        const auto pPlayer = static_cast<CMomentumPlayer*>(pOwner);
        pRocket->SetDamage(pPlayer->m_bHasPracticeMode ? 0.0f : 90.0f);
    }
    else
    {
        pRocket->SetDamage(0.0f);
    }

    // NOTE: Rocket explosion radius is 146.0f in TF2, but 121.0f is used for self damage (see RadiusDamage in mom_gamerules)
    pRocket->SetRadius(146.0f);

    // Momentum trail
    if (mom_rj_trail.GetInt() == 1)
    {
        pRocket->CreateSmokeTrail();
    }

    static ConVarRef mom_rj_sounds("mom_rj_sounds");
    const bool bMomSounds = !mom_rj_use_tf_rocketmodel.GetBool() && mom_rj_sounds.GetInt() == 1;
    const bool bTF2Sounds = !mom_rj_use_tf_rocketmodel.GetBool() && mom_rj_sounds.GetInt() == 2;
    if (bMomSounds || bTF2Sounds)
    {
        pRocket->EmitSound("Missile.Ignite");
    }

    return pRocket;
}
#endif