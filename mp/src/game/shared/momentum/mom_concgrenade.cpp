#include "cbase.h"

#include "mom_concgrenade.h"

#include "IEffects.h"
#include "weapon/weapon_def.h"

#include "util/mom_util.h"

#ifdef GAME_DLL
#include "momentum/mom_player.h"
#include "momentum/mom_triggers.h"
#include "fx_mom_shared.h"
#else
#include "functionproxy.h"
#include "c_te_effect_dispatch.h"
#include "cmodel.h"
#include "clienteffectprecachesystem.h"
#include "model_types.h"
#include "hud_concgrenade.h"
#endif

#include "tier0/memdbgon.h"

#define CONC_LATERAL_POWER 2.74f
#define CONC_VERTICAL_POWER 4.10f

#define CONC_WATER_SINK_RATE 64.0f
#define CONC_WATER_VEL_DEC 0.5f
#define CONC_WATER_REDUCE_THINK 0.2f

static MAKE_TOGGLE_CONVAR(mom_conc_debug_show_radius, "0", FCVAR_REPLICATED, "Show conc explosion radius (for debugging).\n");

static MAKE_TOGGLE_CONVAR(mom_conc_particle_trail_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggle whether the conc grenade has a trail particle or not. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_conc_sound_bounce_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggle whether the conc makes a sound when it bounces. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_conc_sound_fizzle_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggle whether the conc makes a sound when it fizzles. 0 = OFF, 1 = ON\n");

#ifdef GAME_DLL
BEGIN_DATADESC(CMomConcProjectile)
    DEFINE_THINKFUNC(GrenadeThink),
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(MomConcProjectile, DT_MomConcProjectile)

BEGIN_NETWORK_TABLE(CMomConcProjectile, DT_MomConcProjectile)
#ifdef CLIENT_DLL
    RecvPropFloat(RECVINFO(m_flDetonateTime)),
#else
    SendPropFloat(SENDINFO(m_flDetonateTime)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(momentum_concgrenade, CMomConcProjectile)
PRECACHE_WEAPON_REGISTER(momentum_concgrenade);

void CMomConcProjectile::Spawn()
{
    BaseClass::Spawn();
#ifdef CLIENT_DLL
    m_flNextBounceSoundTime = gpGlobals->curtime;

    m_pEntPanel = new CHudConcEntPanel();
    m_pEntPanel->Init(this);

    SetNextClientThink(gpGlobals->curtime);
#else
    SetModel(g_pWeaponDef->GetWeaponModel(WEAPON_CONCGRENADE, "world"));
    SetSize(Vector(-4.051f, -3.714f, -4.333f), Vector(3.822f, 3.962f, 3.02f));
    SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);

    // Rotate from the go
    SetLocalAngularVelocity(RandomAngle(-400, 400));

    SetGravity(GetGrenadeGravity());
    SetElasticity(GetGrenadeElasticity());
    SetFriction(GetGrenadeFriction());

    m_bHitwater = false;

    SetThink(&CMomConcProjectile::GrenadeThink);
    SetNextThink(gpGlobals->curtime);
#endif
}

CMomConcProjectile::~CMomConcProjectile()
{
#ifdef CLIENT_DLL
    if (m_pEntPanel)
        m_pEntPanel->DeletePanel();

    m_pEntPanel = nullptr;
#endif
}

void CMomConcProjectile::Precache()
{
    BaseClass::Precache();
}

void CMomConcProjectile::BounceSound()
{
    if (!mom_conc_sound_bounce_enable.GetBool())
        return;

    if (gpGlobals->curtime > m_flNextBounceSoundTime && GetAbsVelocity().LengthSqr() > 1)
    {
        EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_CONCGRENADE, "bounce"));

        m_flNextBounceSoundTime = gpGlobals->curtime + 0.1f;
    }
}

#ifdef CLIENT_DLL

class CConcGrenadeDetonationProgressProxy : public CResultProxy
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

        m_pResult->SetFloatValue(clamp((gpGlobals->curtime - pEntity->m_flSpawnTime) / CONC_MAX_TIME, 0.0f, 1.0f));
    }
};

EXPOSE_INTERFACE(CConcGrenadeDetonationProgressProxy, IMaterialProxy, "ConcDetProg" IMATERIAL_PROXY_INTERFACE_VERSION);

void CMomConcProjectile::CreateTrailParticles()
{
    if (!mom_conc_particle_trail_enable.GetBool())
        return;

    ParticleProp()->Create(g_pWeaponDef->GetWeaponParticle(WEAPON_CONCGRENADE, "Trail"), PATTACH_ABSORIGIN_FOLLOW);
}

void CMomConcProjectile::ClientThink()
{
    if (!IsInWorld())
    {
        return;
    }

    if (GetGroundEntity() && (GetAbsVelocity() == vec3_origin))
    {
        if (GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH)
            SetMoveType(MOVETYPE_NONE);
    }

    BaseClass::ClientThink();
    SetNextClientThink(gpGlobals->curtime);
}

#else

CMomConcProjectile* CMomConcProjectile::Create(float fTimer, const Vector &position, const Vector &velocity, CBaseEntity *pOwner)
{
    const auto pConcGrenade = dynamic_cast<CMomConcProjectile *>(CreateNoSpawn("momentum_concgrenade", position, vec3_angle, pOwner));

    if (pConcGrenade)
    {
        DispatchSpawn(pConcGrenade);
        pConcGrenade->InitExplosive(pOwner, velocity, vec3_angle);

        if (fTimer >= CONC_MAX_TIME)
        {
            pConcGrenade->SetHandheld(true);
            pConcGrenade->SetDetonateTimerLength(0);
        }
        else
        {
            pConcGrenade->SetDetonateTimerLength(CONC_MAX_TIME - fTimer);
        }
    }

    return pConcGrenade;
}

void CMomConcProjectile::PlayFizzleSound()
{
    if (mom_conc_sound_fizzle_enable.GetBool())
    {
        EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_CONCGRENADE, "fizzle"));
    }
}

void CMomConcProjectile::Destroy()
{
    BaseClass::Destroy();

    SetModelName(NULL_STRING);
    AddSolidFlags(FSOLID_NOT_SOLID);
    SetAbsVelocity(vec3_origin);
}

void CMomConcProjectile::Explode()
{
    const auto& vecOrigin = GetAbsOrigin();

    g_pEffects->EnergySplash(vecOrigin, Vector(0, 0, 1.0f), true);

    // Prevent practice mode or ghost concs from affecting us
    if (GetDamage() > 0.0f)
        AffectEntitiesInRadius();

    const Vector vecSpot = vecOrigin + Vector(0, 0, 8);
    trace_t tr;
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

    // Pull out a bit
    if (!CloseEnough(tr.fraction, 1.0f))
    {
        SetAbsOrigin(tr.endpos + (tr.plane.normal * 1.0f));
    }

    CPVSFilter filter(vecOrigin);
    TE_TFExplosion(filter, vecOrigin, tr.plane.normal, WEAPON_CONCGRENADE, m_bIsHandheld);

    if (mom_conc_debug_show_radius.GetBool())
    {
        DrawRadius();
    }

    Destroy();
}

void CMomConcProjectile::AffectEntitiesInRadius()
{
    CBaseEntity *pEntity;

    for (CEntitySphereQuery sphere(GetAbsOrigin(), GetGrenadeRadius()); (pEntity = sphere.GetCurrentEntity()) != nullptr; sphere.NextEntity())
    {
        if (!pEntity || !pEntity->IsPlayer() || !pEntity->IsAlive())
            continue;

        // Make handheld concs not push other players
        if (pEntity != GetOwnerEntity() && m_bIsHandheld)
            continue;

        Vector vecDistance = pEntity->WorldSpaceCenter() - GetAbsOrigin();
        float flDistance = vecDistance.Length();
        Vector vecResult;

        // TFC considers a distance < 16units to be a handheld
        // However in FF sometimes the distance can be more with a handheld
        // But we don't want to lose the trait of a handheld-like jump with a drop conc so an extra flag here helps out.
        // Remember that m_bIsHandheld only affects the grenade owner

        if ((pEntity == GetOwnerEntity() && m_bIsHandheld) || flDistance < 16.0f)
        {
            // These values are close (~within 0.01) of TFC
            float flLateralPower = CONC_LATERAL_POWER;
            float flVerticalPower = CONC_VERTICAL_POWER;

            Vector vecVelocity = pEntity->GetAbsVelocity();
            float flHorizontalSpeed = vecVelocity.Length2D();

            vecResult = Vector(vecVelocity.x * flLateralPower, vecVelocity.y * flLateralPower, vecVelocity.z * flVerticalPower);
            DevMsg("[Handheld conc] not on the ground or too close (%f)\n", flHorizontalSpeed);

            DevMsg(2, "[Handheld conc] flDistance = %f\n", flDistance);
        }
        else
        {
            float verticalDistance = vecDistance.z;
            vecDistance.z = 0;
            float horizontalDistance = vecDistance.Length();

            // Normalize the lateral direction of this
            vecDistance /= horizontalDistance;

            // This is the equation I've calculated for drop concs
            // It's accurate to about ~0.001 of TFC so pretty sure this is the
            // damn thing they use.
            vecDistance *= horizontalDistance * (8.4f - 0.015f * flDistance);
            vecDistance.z = verticalDistance * (12.6f - 0.0225f * flDistance);

            vecResult = vecDistance;
        }

        pEntity->SetAbsVelocity(vecResult);

        IGameEvent *pEvent = gameeventmanager->CreateEvent("player_explosive_hit");
        if (pEvent)
        {
            pEvent->SetFloat("speed", vecResult.Length());
            gameeventmanager->FireEvent(pEvent);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose: This is messy and probably shouldn't be done
//-----------------------------------------------------------------------------
void CMomConcProjectile::ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity)
{
    // Assume all surfaces have the same elasticity
    float flSurfaceElasticity = 1.0;

    // Don't bounce off of players with perfect elasticity
    if (trace.m_pEnt && trace.m_pEnt->IsPlayer())
    {
        flSurfaceElasticity = 0.3f;
    }

    // if its breakable glass and we kill it, don't bounce.
    // give some damage to the glass, and if it breaks, pass
    // through it.
    bool breakthrough = false;

    if (trace.m_pEnt && FClassnameIs(trace.m_pEnt, "func_breakable"))
        breakthrough = true;

    if (trace.m_pEnt && FClassnameIs(trace.m_pEnt, "func_breakable_surf"))
        breakthrough = true;

    if (breakthrough)
    {
        CTakeDamageInfo info(this, GetOwnerEntity(), 10, DMG_CLUB);
        trace.m_pEnt->DispatchTraceAttack(info, GetAbsVelocity(), &trace);

        ApplyMultiDamage();

        if (trace.m_pEnt->m_iHealth <= 0)
        {
            // slow our flight a little bit
            Vector vel = GetAbsVelocity();

            vel *= 0.4f;

            SetAbsVelocity(vel);
            return;
        }
    }

    float backoff = 1.0f + GetElasticity();

    Vector vecAbsVelocity;

    PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, backoff);

    if (trace.plane.normal.z > 0.7f)
    {
        vecAbsVelocity *= 1.0f - GetFriction();
    }

    // Get the total velocity (player + conveyors, etc.)
    VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);

    // stop if on ground
    if (trace.plane.normal[2] > 0.7f)
    {
        float speed = DotProduct(vecVelocity, vecVelocity);

        if (speed < (30 * 30))
        {
            SetGroundEntity(trace.m_pEnt);
            SetAbsVelocity(vec3_origin);
            SetLocalAngularVelocity(vec3_angle);
        }
        else
        {
            SetAbsVelocity(vecVelocity);
        }
    }
    else
    {
        SetAbsVelocity(vecVelocity);
    }

    BounceSound();
}

void CMomConcProjectile::SetDetonateTimerLength(float timer)
{
    m_flDetonateTime = gpGlobals->curtime + timer;
}

// Override CBaseEntity::IsInWorld to ignore speed
bool CMomConcProjectile::IsInWorld() const
{
    if (!edict())
        return true;

    if (GetAbsOrigin().x >= MAX_COORD_INTEGER)
        return false;
    if (GetAbsOrigin().y >= MAX_COORD_INTEGER)
        return false;
    if (GetAbsOrigin().z >= MAX_COORD_INTEGER)
        return false;
    if (GetAbsOrigin().x <= MIN_COORD_INTEGER)
        return false;
    if (GetAbsOrigin().y <= MIN_COORD_INTEGER)
        return false;
    if (GetAbsOrigin().z <= MIN_COORD_INTEGER)
        return false;

    return true;
}

void CMomConcProjectile::DrawRadius()
{
    const float flRadius = GetGrenadeRadius();
    Vector pos = GetAbsOrigin();
    int r = 255;
    int g = 255, b = 255;
    float flLifetime = 10.0f;
    bool bDepthTest = true;

    Vector edge, lastEdge;
    NDebugOverlay::Line(pos, pos + Vector(0, 0, 50), r, g, b, !bDepthTest, flLifetime);

    lastEdge = Vector(flRadius + pos.x, pos.y, pos.z);
    float angle;
    for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
    {
        edge.x = flRadius * cosf(DEG2RAD(angle)) + pos.x;
        edge.y = pos.y;
        edge.z = flRadius * sinf(DEG2RAD(angle)) + pos.z;

        NDebugOverlay::Line(edge, lastEdge, r, g, b, !bDepthTest, flLifetime);

        lastEdge = edge;
    }

    lastEdge = Vector(pos.x, flRadius + pos.y, pos.z);
    for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
    {
        edge.x = pos.x;
        edge.y = flRadius * cosf(DEG2RAD(angle)) + pos.y;
        edge.z = flRadius * sinf(DEG2RAD(angle)) + pos.z;

        NDebugOverlay::Line(edge, lastEdge, r, g, b, !bDepthTest, flLifetime);

        lastEdge = edge;
    }

    lastEdge = Vector(pos.x, flRadius + pos.y, pos.z);
    for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
    {
        edge.x = flRadius * cosf(DEG2RAD(angle)) + pos.x;
        edge.y = flRadius * sinf(DEG2RAD(angle)) + pos.y;
        edge.z = pos.z;

        NDebugOverlay::Line(edge, lastEdge, r, g, b, !bDepthTest, flLifetime);

        lastEdge = edge;
    }
}

void CMomConcProjectile::GrenadeThink()
{
    if (!IsInWorld())
    {
        Fizzle();
        return;
    }

    if (gpGlobals->curtime > m_flDetonateTime)
    {
        Explode();
        return;
    }

    if (GetGroundEntity() && (GetAbsVelocity() == vec3_origin))
    {
        if (GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH)
            SetMoveType(MOVETYPE_NONE);
    }

    SetNextThink(gpGlobals->curtime);
    WaterCheck();
}

void CMomConcProjectile::WaterCheck()
{
    if (GetWaterLevel() == 0)
        return;

    if (!m_bHitwater)
    {
        SetAbsVelocity(GetAbsVelocity() * CONC_WATER_VEL_DEC);
        m_bHitwater = true;

        m_flHitwaterTimer = gpGlobals->curtime;
    }

    if ((m_flHitwaterTimer + CONC_WATER_REDUCE_THINK) < gpGlobals->curtime)
    {
        SetAbsVelocity(GetAbsVelocity() * CONC_WATER_VEL_DEC);
        m_flHitwaterTimer = gpGlobals->curtime;
    }
}
#endif