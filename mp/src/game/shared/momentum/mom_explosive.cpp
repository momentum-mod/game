#include "cbase.h"

#include "mom_explosive.h"

#ifdef GAME_DLL
#include "momentum/mom_timer.h"
#include "momentum/mom_player.h"
#include "Sprite.h"
#include "momentum/mom_triggers.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomExplosive, DT_MomExplosive);

BEGIN_NETWORK_TABLE(CMomExplosive, DT_MomExplosive)
#ifdef CLIENT_DLL
    RecvPropVector(RECVINFO(m_vInitialVelocity)),
    RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
    RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),
#else
    SendPropVector(SENDINFO(m_vInitialVelocity), 20, 0, -3000, 3000),

    SendPropExclude("DT_BaseEntity", "m_vecOrigin"),
    SendPropExclude("DT_BaseEntity", "m_angRotation"),
    SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD_MP_INTEGRAL | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
    SendPropQAngles(SENDINFO(m_angRotation), 6, SPROP_CHANGES_OFTEN, SendProxy_Angles),
#endif
END_NETWORK_TABLE();


CMomExplosive::CMomExplosive()
{
    m_vInitialVelocity.Init();

#ifdef CLIENT_DLL
    m_flSpawnTime = 0.0f;
#else
    m_fDamage = 0.0f;
#endif
}


void CMomExplosive::Spawn()
{
    BaseClass::Spawn();
#ifdef CLIENT_DLL
    m_flSpawnTime = gpGlobals->curtime;
#else
    UseClientSideAnimation();
    SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
    SetSolidFlags(FSOLID_NOT_STANDABLE);
    SetSolid(SOLID_BBOX);
    AddEffects(EF_NOSHADOW);
    AddFlag(FL_GRENADE);
    m_takedamage = DAMAGE_NO;
#endif
}

#ifdef CLIENT_DLL

int CMomExplosive::DrawModel(int flags)
{
    if (gpGlobals->curtime - m_flSpawnTime < GetDrawDelayTime())
    {
        return 0;
    }

    return BaseClass::DrawModel(flags);
}

void CMomExplosive::OnDataChanged(DataUpdateType_t updateType)
{
    BaseClass::OnDataChanged(updateType);

    if (updateType == DATA_UPDATE_CREATED)
    {
        InitializeInterpolationVelocity();

        CreateTrailParticles();
    }
}

void CMomExplosive::InitializeInterpolationVelocity()
{
    CInterpolatedVar<Vector> &interpolator = GetOriginInterpolator();
    interpolator.ClearHistory();

    CInterpolatedVar<QAngle> &rotInterpolator = GetRotationInterpolator();
    rotInterpolator.ClearHistory();

    const float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

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

#else

void CMomExplosive::InitExplosive(CBaseEntity *pOwner, const Vector &velocity, const QAngle &angles)
{
    m_vInitialVelocity = velocity;
    SetAbsVelocity(velocity);
    SetAbsAngles(angles);

    if (pOwner->IsPlayer())
    {
        const auto pPlayer = static_cast<CMomentumPlayer *>(pOwner);
        m_fDamage = (pPlayer->m_bHasPracticeMode && g_pMomentumTimer->IsRunning()) ? 0.0f : GetDamageAmount();
    }
    else
    {
        m_fDamage = 0.0f;
    }
}

void CMomExplosive::Destroy(bool bShowFizzleSprite)
{
    SetThink(&BaseClass::SUB_Remove);
    SetNextThink(gpGlobals->curtime);
    SetTouch(nullptr);
    AddEffects(EF_NODRAW);

    if (bShowFizzleSprite)
    {
        const auto pGlowSprite = CSprite::SpriteCreate(NOGRENADE_SPRITE, GetAbsOrigin(), false);
        if (pGlowSprite)
        {
            pGlowSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxFadeFast);
            pGlowSprite->SetThink(&CSprite::SUB_Remove);
            pGlowSprite->SetNextThink(gpGlobals->curtime + 1.0f);
        }
    }
}

#endif