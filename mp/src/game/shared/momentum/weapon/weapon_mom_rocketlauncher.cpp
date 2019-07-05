#include "cbase.h"

#include "mom_player_shared.h"
#include "weapon_mom_rocketlauncher.h"

#ifndef CLIENT_DLL
#include "explode.h"
#endif

#include "tier0/memdbgon.h"

//=============================================================================
// Rocket
//=============================================================================

#ifndef CLIENT_DLL
#define ROCKET_SPEED 1100

BEGIN_DATADESC(CMomentumRocket)
    // Fields
    DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
    DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
    DEFINE_FIELD(m_flDamage, FIELD_FLOAT),

    // Functions
    DEFINE_FUNCTION(Touch),
END_DATADESC()

LINK_ENTITY_TO_CLASS(momentum_rocket, CMomentumRocket);

class CMomentumRocketLauncher;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CMomentumRocket::CMomentumRocket() { m_hRocketTrail = NULL; }

CMomentumRocket::~CMomentumRocket() {}

//-----------------------------------------------------------------------------
// Purpose:
//
//
//-----------------------------------------------------------------------------
void CMomentumRocket::Precache()
{
    // MOM_TODO:
    // Replace HL2 missile model
    PrecacheModel("models/weapons/w_missile.mdl");
    PrecacheScriptSound("Missile.Ignite");
}

//-----------------------------------------------------------------------------
// Purpose:
//
//
//-----------------------------------------------------------------------------
void CMomentumRocket::Spawn()
{
    Precache();

    SetSolid(SOLID_BBOX);
    SetMoveType(MOVETYPE_FLY);
    SetModel("models/weapons/w_missile.mdl");
    EmitSound("Missile.Ignite");

    Vector vecForward;
    AngleVectors(GetLocalAngles(), &vecForward);
    SetAbsVelocity(vecForward * ROCKET_SPEED);

    SetDamage(90.0f);

    SetTouch(&CMomentumRocket::Touch);
    SetThink(NULL);

    CreateSmokeTrail();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
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
    float flRadius = 146.0f;

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

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pOther -
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
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
// Purpose:
//
// Input  : &vecOrigin -
//			&vecAngles -
//			NULL -
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

//=============================================================================
// Rocket Launcher
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumRocketLauncher, DT_MomentumRocketLauncher)

BEGIN_NETWORK_TABLE(CMomentumRocketLauncher, DT_MomentumRocketLauncher)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumRocketLauncher)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_rocketlauncher, CMomentumRocketLauncher);
PRECACHE_WEAPON_REGISTER(weapon_momentum_rocketlauncher);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CMomentumRocketLauncher::CMomentumRocketLauncher()
{
    // m_flTimeToIdleAfterFire = 1.9f;
    // m_flIdleInterval = 20.0f;
}

void CMomentumRocketLauncher::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_rocket");
#endif
}

void CMomentumRocketLauncher::RocketLauncherFire()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.8f;

#ifdef GAME_DLL
    Vector vForward, vRight, vUp;

    pPlayer->EyeVectors(&vForward, &vRight, &vUp);

    // Offset values from
    // https://github.com/NicknineTheEagle/TF2-Base/blob/master/src/game/shared/tf/tf_weaponbase_gun.cpp#L334
    Vector vecOffset(23.5f, 12.0f, -3.0f);
    if (pPlayer->GetFlags() & FL_DUCKING)
    {
        vecOffset.z = 8.0f;
    }

    Vector muzzlePoint =
        pPlayer->Weapon_ShootPosition() + (vForward * vecOffset.x) + (vRight * vecOffset.y) + (vUp * vecOffset.z);

    QAngle vecAngles;
    VectorAngles(vForward, vecAngles);

    CMomentumRocket::EmitRocket(muzzlePoint, vecAngles, pPlayer);
#endif

    WeaponSound(SINGLE);

    // MOM_FIXME:
    // This will cause an assertion error.
    // SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CMomentumRocketLauncher::PrimaryAttack() { RocketLauncherFire(); }