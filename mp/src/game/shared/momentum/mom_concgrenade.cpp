#include "cbase.h"

#include "mom_concgrenade.h"

#include "IEffects.h"
#include "weapon/weapon_def.h"

#include "util/mom_util.h"

#ifdef GAME_DLL
#include "momentum/mom_player.h"
#include "momentum/mom_triggers.h"
#include "te_effect_dispatch.h"
#else
#include "iinput.h"
#include "c_te_effect_dispatch.h"
#include "cmodel.h"
#include "clienteffectprecachesystem.h"
#include "model_types.h"
#include "hud_concgrenade.h"
#endif

#include "tier0/memdbgon.h"

#define CONC_TRAIL "sprites/conc_trail.vmt"
#define CONC_GLOW_SPRITE "sprites/glow04_noz.vmt"
#define CONC_RINGS "sprites/lgtning.vmt"

#define CONC_LATERAL_POWER 2.74f
#define CONC_VERTICAL_POWER 4.10f

#define CONC_WATER_SINK_RATE 64.0f
#define CONC_WATER_VEL_DEC 0.5f
#define CONC_WATER_REDUCE_THINK 0.2f

#define CONC_EFFECT "MOM_ConcussionEffect"
#define CONC_EFFECT_HANDHELD "MOM_ConcussionEffectHandheld"
#define CONCBITS_EFFECT "MOM_ConcBitsEffect"

static MAKE_TOGGLE_CONVAR(mom_conc_debug_show_radius, "0", FCVAR_DEVELOPMENTONLY, "Show conc explosion radius (for debugging).\n");
static MAKE_TOGGLE_CONVAR(mom_conc_glow_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggle the conc glow sprite. 0 = OFF, 1 = ON\n");
static ConVar mom_conc_glow_color("mom_conc_glow_color", "FFFFC8C8", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Controls the color of the conc glow in RGBA hex.\n");
static MAKE_CONVAR(mom_conc_glow_size, "1.0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Controls the size of the conc glow effect.\n", 0.4f, 100.0f);

static MAKE_TOGGLE_CONVAR(mom_conc_trail_enable, "1", FCVAR_ARCHIVE, "Toggle whether the conc grenade has a trail or not. 0 = OFF, 1 = ON\n");
static ConVar mom_conc_trail_color("mom_conc_trail_color", "FFFFC8C8", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Controls the color of the conc trail in RGBA hex.\n");

static MAKE_TOGGLE_CONVAR(mom_conc_outline_enable, "1", FCVAR_ARCHIVE, "Toggle whether the conc grenade can be seen through walls with an outline. 0 = OFF, 1 = ON.\n");

#ifdef CLIENT_DLL
int g_iConcRingTexture = -1;

CLIENTEFFECT_REGISTER_BEGIN(PrecacheGrenadeSprite)
    CLIENTEFFECT_MATERIAL("sprites/conc_trail")
    CLIENTEFFECT_MATERIAL("sprites/conc_ring")
    CLIENTEFFECT_MATERIAL("sprites/conc_ring_blur")
    CLIENTEFFECT_MATERIAL("sprites/lgtning")
CLIENTEFFECT_REGISTER_END()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(MomConcGlow, DT_MomConcGlow)

BEGIN_NETWORK_TABLE(CMomConcGlow, DT_MomConcGlow)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(env_momconcglow, CMomConcGlow)

BEGIN_DATADESC(CMomConcGlow)
END_DATADESC()

#ifdef GAME_DLL
BEGIN_DATADESC(CMomConcProjectile)
    DEFINE_THINKFUNC(GrenadeThink),
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(MomConcProjectile, DT_MomConcProjectile)

BEGIN_NETWORK_TABLE(CMomConcProjectile, DT_MomConcProjectile)
#ifdef CLIENT_DLL
    RecvPropFloat(RECVINFO(m_flSpawnTime)),
    RecvPropVector(RECVINFO(m_vInitialVelocity)),
    RecvPropFloat(RECVINFO(m_flDetonateTime)),
    RecvPropBool(RECVINFO(m_bIsOn)),
#else
    SendPropFloat(SENDINFO(m_flSpawnTime)),
    SendPropVector(SENDINFO(m_vInitialVelocity), 20, 0, -3000, 3000),
    SendPropFloat(SENDINFO(m_flDetonateTime)),
    SendPropBool(SENDINFO(m_bIsOn)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(momentum_concgrenade, CMomConcProjectile)
PRECACHE_WEAPON_REGISTER(momentum_concgrenade);

void CMomConcProjectile::Spawn()
{
    BaseClass::Spawn();
#ifdef CLIENT_DLL
    m_flSpawnTime = gpGlobals->curtime;
    m_flNextBounceSoundTime = gpGlobals->curtime;
    m_pEntPanel = new CHudConcEntPanel();
    m_pEntPanel->Init(this);
    SetNextClientThink(gpGlobals->curtime);
#else
    SetModel(g_pWeaponDef->GetWeaponModel(WEAPON_CONCGRENADE, "world"));
    SetSize(Vector(-4.051f, -3.714f, -4.333f), Vector(3.822f, 3.962f, 3.02f));
    SetSolid(SOLID_BBOX);
    SetSolidFlags(FSOLID_NOT_STANDABLE);
    SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
    SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);

    // Rotate from the go
    SetLocalAngularVelocity(RandomAngle(-400, 400));

    SetGravity(GetGrenadeGravity());
    SetElasticity(GetGrenadeElasticity());
    SetFriction(GetGrenadeFriction());
    SetDamage(GetGrenadeDamage());
    SetDamageRadius(GetGrenadeRadius());

    m_takedamage = DAMAGE_NO;

    AddFlag(FL_GRENADE);

    m_bHitwater = false;
    m_bIsOn = false;

    CreateTrail();

    if(mom_conc_glow_enable.GetBool())
        CreateGlowSprite();

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
    PrecacheModel(CONC_TRAIL);
    PrecacheModel(CONC_GLOW_SPRITE);
#ifdef CLIENT_DLL
    g_iConcRingTexture = //modelinfo->GetModelIndex(CONC_RINGS);
#endif
    PrecacheModel(CONC_RINGS);

    BaseClass::Precache();
}

void CMomConcProjectile::BounceSound()
{
    if (gpGlobals->curtime > m_flNextBounceSoundTime && GetAbsVelocity().LengthSqr() > 1)
    {
        EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_CONCGRENADE, "bounce"));

        m_flNextBounceSoundTime = gpGlobals->curtime + 0.1;
    }
}

#ifdef CLIENT_DLL
void CMomConcProjectile::OnDataChanged(DataUpdateType_t type)
{
    BaseClass::OnDataChanged(type);

    // Do interpolation samples on a player's own projectiles when they are in first person
    if (type == DATA_UPDATE_CREATED && GetOwnerEntity() == CBasePlayer::GetLocalPlayer() && !input->CAM_IsThirdPerson())
    {
        // Now stick our initial velocity into the interpolation history
        CInterpolatedVar<Vector> &interpolator = GetOriginInterpolator();

        interpolator.ClearHistory();
        float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

        // Add a sample 2 seconds back.
        Vector vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 0.75f;
        interpolator.AddToHead(changeTime - 2.0f, &vecCurOrigin, false);

        // Add a sample 1 second back.
        vecCurOrigin = GetLocalOrigin() - (/*m_vInitialVelocity*/ GetAbsVelocity() * 0.5f);
        interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);

        // Add the current sample.
        vecCurOrigin = GetLocalOrigin();
        interpolator.AddToHead(changeTime, &vecCurOrigin, false);

        vecCurOrigin = GetLocalOrigin() + GetAbsVelocity() * 0.5f;
        interpolator.AddToHead(changeTime + 0.5f, &vecCurOrigin, false);

        vecCurOrigin = GetLocalOrigin() + GetAbsVelocity() * 1.0f;
        interpolator.AddToHead(changeTime + 1.0f, &vecCurOrigin, false);
    }
}

void CMomConcProjectile::ClientThink()
{
    if (!IsInWorld())
    {
        return;
    }

    if (gpGlobals->curtime > m_flDetonateTime)
    {
        Detonate();
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

void CMomConcProjectile::DoIdleEffect()
{
    DevMsg(2, "[concussion] idle\n");

    if (m_hGlowSprite)
    {
        m_hGlowSprite->SetBrightness(random->RandomInt(32, 48));
        m_hGlowSprite->SetScale(random->RandomFloat(2.5, 3.5));
    }
}

int CMomConcGlow::DrawModel(int flags)
{
    if (m_bReadyToDraw == false)
        return 0;

    if (modelinfo->GetModelType(GetModel()) != mod_sprite)
    {
        assert(0);
        return 0;
    }

    const auto *pConc = dynamic_cast<CMomConcProjectile*> (GetOwnerEntity());

    if (!pConc)
        return 0;

    // Because we're using a NOZ sprite, need to traceline to ensure this is really
    // visible. And we're using a NOZ sprite so it doesnt clip on stuff, okay!
    trace_t tr;
    UTIL_TraceLine(CBasePlayer::GetLocalPlayer()->EyePosition(), GetAbsOrigin(), MASK_SHOT, pConc, COLLISION_GROUP_NONE, &tr);

    if (tr.fraction < 1.0f && tr.m_pEnt != pConc)
        return 0;

    Color concGlowColor;
    if (!MomUtil::GetColorFromHex(mom_conc_glow_color.GetString(), concGlowColor))
        return 0;

    Vector vecForward, vecRight, vecUp, vecDir;
    pConc->GetVectors(&vecForward, &vecRight, &vecUp);
    vecDir = pConc->GetAbsOrigin() - CBasePlayer::GetLocalPlayer()->EyePosition();
    VectorNormalize(vecDir);

    float alpha = vecUp.Dot(vecDir);

    if (alpha < 0)
        alpha *= -1;

    alpha = 1.0f - alpha;
    alpha *= alpha;

    alpha = static_cast<float>(concGlowColor.a()) * (54.0f + 200.0f * alpha);

    int drawn = DrawSprite(
        this,
        GetModel(),
        GetAbsOrigin(),
        GetAbsAngles(),
        m_flFrame, 				// sprite frame to render
        m_hAttachedToEntity, 	// attach to
        m_nAttachment, 			// attachment point
        GetRenderMode(), 		// rendermode
        m_nRenderFX,
        static_cast<int>(alpha),
        concGlowColor.r(),
        concGlowColor.g(),
        concGlowColor.b(),
        mom_conc_glow_size.GetFloat() + random->RandomFloat(-0.04f, 0.04f)); // sprite scale

    return drawn;
}

void CMomConcGlow::OnDataChanged(DataUpdateType_t updateType)
{
    if (updateType == DATA_UPDATE_CREATED)
    {
        SetNextClientThink(CLIENT_THINK_ALWAYS);
    }
}
#else

void CMomConcProjectile::Remove()
{
    if (m_pTrail.Get())
    {
        m_pTrail->SetParent(nullptr);
        m_pTrail->Remove();
        m_pTrail = nullptr;
    }

    if (m_hGlowSprite.Get())
    {
        m_hGlowSprite->SetParent(nullptr);
        m_hGlowSprite->Remove();
        m_hGlowSprite = nullptr;
    }

    SetModelName(NULL_STRING);
    AddSolidFlags(FSOLID_NOT_SOLID);
    SetTouch(nullptr);
    SetThink(&CBaseGrenade::SUB_Remove);

    AddEffects(EF_NODRAW);
    SetAbsVelocity(vec3_origin);
    SetNextThink(gpGlobals->curtime);

    const auto pSprite = CSprite::SpriteCreate(NOGRENADE_SPRITE, GetAbsOrigin(), false);
    if (pSprite)
    {
        pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxFadeFast);
        pSprite->SetThink(&CSprite::SUB_Remove);
        pSprite->SetNextThink(gpGlobals->curtime + 1.0f);
    }
}

// Do not move the explosion 32 units above the ground! That behaviour does not occur with conc grenades
void CMomConcProjectile::Explode(trace_t *pTrace, int bitsDamageType)
{
    CEffectData data;
    data.m_vOrigin = GetAbsOrigin();
    data.m_flScale = 1.0f;
    data.m_flRadius = GetGrenadeRadius();

    if (m_bIsHandheld)
    {
        DispatchEffect(CONC_EFFECT_HANDHELD, data);
    }
    else
    {
        DispatchEffect(CONC_EFFECT, data);
    }

    g_pEffects->EnergySplash(GetAbsOrigin(), Vector(0, 0, 1.0f), true);

    CBaseEntity *pEntity;

    for (CEntitySphereQuery sphere(GetAbsOrigin(), GetGrenadeRadius()); (pEntity = sphere.GetCurrentEntity()) != nullptr; sphere.NextEntity())
    {
        if (!pEntity || !pEntity->IsPlayer() || !pEntity->IsAlive())
            continue;

        // Make handheld concs not push other players
        if (pEntity != GetThrower() && m_bIsHandheld)
            continue;

        Vector vecDistance = pEntity->GetAbsOrigin() - GetAbsOrigin();
        float flDistance = vecDistance.Length();
        Vector vecResult;

        // TFC considers a distance < 16units to be a handheld
        // However in FF sometimes the distance can be more with a handheld
        // But we don't want to lose the trait of a handheld-like jump with a drop conc so an extra flag here helps out.
        // Remember that m_bIsHandheld only affects the grenade owner

        if (pEntity == GetThrower() && m_bIsHandheld || flDistance < 16.0f)
        {
            // These values are close (~within 0.01) of TFC
            float flLateralPower = CONC_LATERAL_POWER;
            float flVerticalPower = CONC_VERTICAL_POWER;

            Vector vecVelocity = pEntity->GetAbsVelocity();
            Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);
            float flHorizontalSpeed = vecLatVelocity.Length();

            vecResult = Vector(vecVelocity.x * flLateralPower, vecVelocity.y * flLateralPower, vecVelocity.z * flVerticalPower);

            DevMsg(2, "[Handheld conc] not on the ground or too fast (%f)\n", flHorizontalSpeed);
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

            //pEntity->SetAbsVelocity(vecDisplacement);
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

    EmitSound(g_pWeaponDef->GetWeaponSound(WEAPON_CONCGRENADE, "explode"));

    if (mom_conc_debug_show_radius.GetBool())
    {
        DrawRadius(GetGrenadeRadius());
    }

    Remove();
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
        flSurfaceElasticity = 0.3;
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
        CTakeDamageInfo info(this, GetThrower(), 10, DMG_CLUB);
        trace.m_pEnt->DispatchTraceAttack(info, GetAbsVelocity(), &trace);

        ApplyMultiDamage();

        if (trace.m_pEnt->m_iHealth <= 0)
        {
            // slow our flight a little bit
            Vector vel = GetAbsVelocity();

            vel *= 0.4;

            SetAbsVelocity(vel);
            return;
        }
    }

    float backoff = 1.0 + GetElasticity();

    Vector vecAbsVelocity;

    PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, backoff);

    if (trace.plane.normal.z > 0.7f)
    {
        vecAbsVelocity *= 1.0f - GetFriction();
    }

    // Get the total velocity (player + conveyors, etc.)
    VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);

    // stop if on ground
    if (trace.plane.normal[2] > 0.7)
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

void CMomConcProjectile::SetupInitialTransmittedVelocity(const Vector &velocity)
{
    m_vInitialVelocity = velocity;
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

void CMomConcProjectile::DrawRadius(float flRadius)
{
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
        edge.x = flRadius * cos(DEG2RAD(angle)) + pos.x;
        edge.y = pos.y;
        edge.z = flRadius * sin(DEG2RAD(angle)) + pos.z;

        NDebugOverlay::Line(edge, lastEdge, r, g, b, !bDepthTest, flLifetime);

        lastEdge = edge;
    }

    lastEdge = Vector(pos.x, flRadius + pos.y, pos.z);
    for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
    {
        edge.x = pos.x;
        edge.y = flRadius * cos(DEG2RAD(angle)) + pos.y;
        edge.z = flRadius * sin(DEG2RAD(angle)) + pos.z;

        NDebugOverlay::Line(edge, lastEdge, r, g, b, !bDepthTest, flLifetime);

        lastEdge = edge;
    }

    lastEdge = Vector(pos.x, flRadius + pos.y, pos.z);
    for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
    {
        edge.x = flRadius * cos(DEG2RAD(angle)) + pos.x;
        edge.y = flRadius * sin(DEG2RAD(angle)) + pos.y;
        edge.z = pos.z;

        NDebugOverlay::Line(edge, lastEdge, r, g, b, !bDepthTest, flLifetime);

        lastEdge = edge;
    }
}

void CMomConcProjectile::CreateTrail()
{
    if (!mom_conc_trail_enable.GetBool())
        return;

    Color trailColor;
    if (!MomUtil::GetColorFromHex(mom_conc_trail_color.GetString(), trailColor))
        return;

    m_pTrail = CSpriteTrail::SpriteTrailCreate(CONC_TRAIL, GetLocalOrigin(), false);

    if (!m_pTrail)
        return;

    m_pTrail->FollowEntity(this);
    m_pTrail->SetTransparency(kRenderTransAdd, trailColor.r(), trailColor.g(), trailColor.b(), trailColor.a(), kRenderFxNone);
    m_pTrail->SetStartWidth(10.0f);
    m_pTrail->SetEndWidth(5.0f);
    m_pTrail->SetLifeTime(0.5f);
}

void CMomConcProjectile::CreateGlowSprite()
{
    m_hGlowSprite = CMomConcGlow::Create(GetAbsOrigin(), this);
    m_hGlowSprite->SetAttachment(this, LookupAttachment("glowsprite"));
    m_hGlowSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 128, kRenderFxNone);
    m_hGlowSprite->SetBrightness(255, 0.2f);
    m_hGlowSprite->SetScale(1.0f, 0.2f);
}


void CMomConcProjectile::GrenadeThink()
{
    if (!IsInWorld())
    {
        Remove();
        return;
    }

    if (gpGlobals->curtime > m_flDetonateTime)
    {
        Detonate();
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
    if (GetWaterLevel() != 0)
    {
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
}

CMomConcGlow *CMomConcGlow::Create(const Vector &origin, CBaseEntity *pOwner)
{
    auto *pConcGlow = dynamic_cast<CMomConcGlow*>(CBaseEntity::Create("env_momconcglow", origin, QAngle(0, 0, 0)));

    if (!pConcGlow)
        return nullptr;

    pConcGlow->SetRenderMode(kRenderWorldGlow);

    pConcGlow->SetMoveType(MOVETYPE_NONE);
    pConcGlow->AddSolidFlags(FSOLID_NOT_SOLID);
    pConcGlow->AddEffects(EF_NOSHADOW);
    UTIL_SetSize(pConcGlow, vec3_origin, vec3_origin);

    pConcGlow->SetOwnerEntity(pOwner);

    pConcGlow->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

    pConcGlow->SpriteInit(CONC_GLOW_SPRITE, origin);
    pConcGlow->SetName(AllocPooledString("glowsprite"));
    pConcGlow->SetTransparency(kRenderWorldGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
    pConcGlow->SetScale(0.25f);
    pConcGlow->SetOwnerEntity(pOwner);
    pConcGlow->SetSimulatedEveryTick(true);

    return pConcGlow;
}
#endif