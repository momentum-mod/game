#include "cbase.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_df_grenadelauncher.h"

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

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.8f;
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
    trace_t trace;
    const int MASK_RADIUS_DAMAGE = MASK_SHOT & (~CONTENTS_HITBOX);

    pPlayer->EyeVectors(&vForward, &vRight, &vUp);

    VectorCopy(pPlayer->GetAbsOrigin(), muzzle);
    muzzle[2] += pPlayer->GetViewOffset()[2];

    VectorAngles(vForward, angForward);

    AngularImpulse angImpulse(600, random->RandomInt(-1200, 1200), 0);
    
    VectorNormalize(vForward);
    vForward[2] += 0.2f;
    VectorNormalize(vForward);
    
    VectorScale(vForward, 700, vForward);
    
    CMomDFGrenade::Create(muzzle, angForward, vForward, angImpulse, pPlayer);

    DecalPacket rocket = DecalPacket::Rocket(muzzle, angForward);
    g_pMomentumGhostClient->SendDecalPacket(&rocket);
#endif
}
