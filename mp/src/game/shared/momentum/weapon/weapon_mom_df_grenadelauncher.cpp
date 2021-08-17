#include "cbase.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_df_grenadelauncher.h"
#include "movevars_shared.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumDFGrenadeLauncher, DT_MomentumDFGrenadeLauncher);

BEGIN_NETWORK_TABLE(CMomentumDFGrenadeLauncher, DT_MomentumDFGrenadeLauncher)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumDFGrenadeLauncher)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_df_grenadelauncher, CMomentumDFGrenadeLauncher);
PRECACHE_WEAPON_REGISTER(weapon_momentum_df_grenadelauncher);

CMomentumDFGrenadeLauncher::CMomentumDFGrenadeLauncher()
{
    m_flIdleInterval = 20.0f;
    m_flTimeToIdleAfterFire = 0.8f;
}

void CMomentumDFGrenadeLauncher::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_df_grenade");
#endif
}

void CMomentumDFGrenadeLauncher::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    int ammo = pPlayer->m_iMomAmmo.Get(GetWeaponID());
    if (ammo == 0)
    {
        return;
    }
    else if (ammo != -1)
    {
        pPlayer->m_iMomAmmo.Set(GetWeaponID(), ammo - 1);
    }

    if (pPlayer->m_flRemainingHaste < 0 || pPlayer->m_flRemainingHaste > gpGlobals->curtime)
    {
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + (0.8f / 1.3);
    }
    else
    {
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.8f;
    }

    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();

    WeaponSound(GetWeaponSound("single_shot"));

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifdef GAME_DLL
    Vector vForward, vRight, vUp;
    QAngle angForward;
    Vector muzzle;
    Vector start;
    trace_t trace;
    const int MASK_RADIUS_DAMAGE = MASK_SHOT & (~CONTENTS_HITBOX);
    float damageFactor = 1;

    if (pPlayer->m_flRemainingDamageBoost > gpGlobals->curtime || pPlayer->m_flRemainingDamageBoost < 0)
    {
        damageFactor = 3;
    }


    pPlayer->EyeVectors(&vForward, &vRight, &vUp);

    VectorCopy(pPlayer->GetAbsOrigin(), muzzle);
    muzzle[2] += pPlayer->GetViewOffset()[2];
    VectorCopy(muzzle, start);
    VectorMA(muzzle, sv_df_weapon_scan.GetFloat(), vForward, muzzle);

    VectorAngles(vForward, angForward);

    AngularImpulse angImpulse(600, random->RandomInt(-1200, 1200), 0);
    
    VectorNormalize(vForward);
    vForward[2] += 0.2f;
    VectorNormalize(vForward);
    
    VectorScale(vForward, 700, vForward);

    UTIL_TraceLine(start, muzzle, MASK_RADIUS_DAMAGE, pPlayer, COLLISION_GROUP_NONE, &trace);
    
    CMomDFGrenade::Create(trace.endpos, angForward, vForward, angImpulse, pPlayer, damageFactor);

    DecalPacket rocket = DecalPacket::Rocket(muzzle, angForward);
    g_pMomentumGhostClient->SendDecalPacket(&rocket);
#endif
}
