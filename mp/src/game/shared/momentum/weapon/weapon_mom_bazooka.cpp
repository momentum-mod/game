#include "cbase.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_bazooka.h"
#include "in_buttons.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumBazooka, DT_MomentumBazooka);

BEGIN_NETWORK_TABLE(CMomentumBazooka, DT_MomentumBazooka)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumBazooka)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_bazooka, CMomentumBazooka);
PRECACHE_WEAPON_REGISTER(weapon_momentum_bazooka);

MAKE_TOGGLE_CONVAR(mom_bb_sound_shoot_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggles the rocket shooting sound on or off. 0 = OFF, 1 = ON\n");

#ifdef GAME_DLL
static MAKE_TOGGLE_CONVAR_CV(mom_bb_center_fire, "1", FCVAR_ARCHIVE, "If enabled, all rockets will be fired from the center of the screen. 0 = OFF, 1 = ON\n", nullptr,
    [](IConVar *pVar, const char *pNewVal)
    {
        if (g_pMomentumTimer->IsRunning())
        {
            Warning("Cannot change rocket firing mode while in a run! Stop your timer to be able to change it.\n");
            return false;
        }

        return true;
    }
);
#endif

CMomentumBazooka::CMomentumBazooka()
{
    m_flTimeToIdleAfterFire = 0.8f;
    m_flIdleInterval = 20.0f;
    m_iLoaded = 0;
    m_bSpewing = false;
    m_flFirstAttackTime = 0;
}

void CMomentumBazooka::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_rocket");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Return the origin & angles for a projectile fired from the player's gun
//-----------------------------------------------------------------------------
void CMomentumBazooka::GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward)
{
#ifdef GAME_DLL
    static ConVarRef cl_righthand("cl_righthand");
#else
    extern ConVar_Validated cl_righthand;
    static ConVarRef mom_bb_center_fire("mom_bb_center_fire");
#endif

    if (mom_bb_center_fire.GetBool())
    {
        vecOffset.y = 0.0f;
    }
    else if (!cl_righthand.GetBool())
    {
        vecOffset.y *= -1.0f;
    }

    Vector vecForward, vecRight, vecUp;
    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    Vector vecShootPos = pPlayer->Weapon_ShootPosition();

    // Estimate end point
    Vector endPos = vecShootPos + vecForward * 2000.0f;

    // Trace forward and find what's in front of us, and aim at that
    trace_t tr;

    CTraceFilterSimple filter(pPlayer, COLLISION_GROUP_NONE);
    UTIL_TraceLine(vecShootPos, endPos, MASK_SOLID_BRUSHONLY, &filter, &tr);

    // Offset actual start point
    *vecSrc = vecShootPos + (vecForward * vecOffset.x) + (vecRight * vecOffset.y) + (vecUp * vecOffset.z);

    // Find angles that will get us to our desired end point
    // Only use the trace end if it wasn't too close, which results
    // in visually bizarre forward angles
    if (tr.fraction > 0.1f)
    {
        VectorAngles(tr.endpos - *vecSrc, *angForward);
    }
    else
    {
        VectorAngles(endPos - *vecSrc, *angForward);
    }
}

void CMomentumBazooka::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    if (m_bSpewing)
    {
        WeaponIdle();
        return;
    }

    if (m_bFirstLoad)
    {
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 1.196f;
        m_flFirstAttackTime = gpGlobals->curtime + 0.7f;
        m_bFirstLoad = false;
    }
    else
    {
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 1.04f;
    }

    if (mom_bb_sound_shoot_enable.GetBool())
    {
        WeaponSound(GetWeaponSound("load"));
    }

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

    m_iLoaded++;
#ifdef GAME_DLL
    if (m_iLoaded > 3)
    {
        CMomRocket *pRocket = EmitRocket();
        trace_t trace;
        CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);
        UTIL_TraceLine(pPlayer->EyePosition(), pRocket->GetAbsOrigin(), MASK_SOLID_BRUSHONLY, &traceFilter, &trace);
        pRocket->Explode(&trace, pPlayer);
    }
#endif
    m_iLoaded = m_iLoaded % 6;
}

void CMomentumBazooka::WeaponIdle()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    if (!(pPlayer->m_nButtons & IN_ATTACK))
    {
        m_bFirstLoad = true;
    }

    if (m_iLoaded > 0 && !(pPlayer->m_nButtons & IN_ATTACK) && !m_bSpewing)
    {
        m_bSpewing = true;
        if (m_iLoaded > 3)
        {
            m_iLoaded = 6 - m_iLoaded;
        }
        if (gpGlobals->curtime < m_flFirstAttackTime)
        {
            m_flNextPrimaryAttack = m_flFirstAttackTime;
        }
        else
        {
            m_flNextPrimaryAttack = gpGlobals->curtime;
        }
    }

    if (m_bSpewing && gpGlobals->curtime >= m_flNextPrimaryAttack)
    {
        if (m_iLoaded == 0)
        {
            m_bSpewing = false;
            return;
        }
        EmitRocket();
        m_flNextPrimaryAttack = gpGlobals->curtime + 0.24f;
        m_iLoaded--;
    }
}

CMomRocket *CMomentumBazooka::EmitRocket()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return nullptr;

    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();

    if (mom_bb_sound_shoot_enable.GetBool())
    {
        WeaponSound(GetWeaponSound("single_shot"));
    }

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

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
    Vector vecSrc;
    QAngle angForward;
    GetProjectileFireSetup(pPlayer, vecOffset, &vecSrc, &angForward);

    trace_t trace;
    CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);
    UTIL_TraceLine(pPlayer->EyePosition(), vecSrc, MASK_SOLID_BRUSHONLY, &traceFilter, &trace);

    CMomRocket *pRocket = CMomRocket::EmitRocket(trace.endpos, angForward, pPlayer);
    pRocket->m_bBazookaRocket = true;

    DecalPacket rocket = DecalPacket::Rocket(trace.endpos, angForward);
    g_pMomentumGhostClient->SendDecalPacket(&rocket);
    return pRocket;
#else
    return nullptr;
#endif
}