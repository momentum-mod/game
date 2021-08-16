#include "cbase.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_df_lightninggun.h"
#include "movevars_shared.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumDFLightningGun, DT_MomentumDFLightningGun);

BEGIN_NETWORK_TABLE(CMomentumDFLightningGun, DT_MomentumDFLightningGun)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumDFLightningGun)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_df_lightninggun, CMomentumDFLightningGun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_df_lightninggun);

CMomentumDFLightningGun::CMomentumDFLightningGun()
{
    m_flIdleInterval = 20.0f;
    m_flTimeToIdleAfterFire = 0.1f;
}

void CMomentumDFLightningGun::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_df_rocket");
#endif
}

void CMomentumDFLightningGun::PrimaryAttack()
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
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + (0.05f / 1.3);
    }
    else
    {
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.05f;
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
    Vector dest;
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
    VectorMA(muzzle, LIGHTNING_RANGE + sv_df_weapon_scan.GetFloat(), vForward, dest);

    UTIL_TraceLine(muzzle, dest, MASK_RADIUS_DAMAGE, pPlayer, COLLISION_GROUP_NONE, &trace);
    if (trace.fraction < 0.99)
    {
        VectorCopy(trace.endpos, muzzle);
        VectorAngles(vForward, angForward);
        CMomDFRocket *rocket = CMomDFRocket::EmitRocket(muzzle, angForward, pPlayer, DF_LIGHTNING, damageFactor);
        rocket->Explode(&trace, trace.m_pEnt);
        DecalPacket rocketPacket = DecalPacket::Rocket(muzzle, angForward);
        g_pMomentumGhostClient->SendDecalPacket(&rocketPacket);
    }
#endif
}
