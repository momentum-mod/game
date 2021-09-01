#include "cbase.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_df_plasmagun.h"
#include "movevars_shared.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumDFPlasmaGun, DT_MomentumDFPlasmaGun);

BEGIN_NETWORK_TABLE(CMomentumDFPlasmaGun, DT_MomentumDFPlasmaGun)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumDFPlasmaGun)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_df_plasmagun, CMomentumDFPlasmaGun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_df_plasmagun);

CMomentumDFPlasmaGun::CMomentumDFPlasmaGun()
{
    m_flIdleInterval = 20.0f;
    m_flTimeToIdleAfterFire = 0.1f;
    shotsFired = 0;
    tickWait = 0;
}

void CMomentumDFPlasmaGun::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_df_rocket");
#endif
}

void CMomentumDFPlasmaGun::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    if (tickWait > gpGlobals->tickcount)
    {
        return;
    }

    int ammo = pPlayer->m_iMomAmmo.Get(GetWeaponID());
    if (ammo == 0)
    {
        return;
    }
    else if (ammo != -1)
    {
        pPlayer->m_iMomAmmo.Set(GetWeaponID(), ammo - 1);
    }

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = 0;
    if (pPlayer->m_flRemainingHaste < 0 || pPlayer->m_flRemainingHaste > gpGlobals->curtime)
    {
        tickWait = gpGlobals->tickcount + (shotsFired % 2 == 0 ? 9 : 10);
    }
    else
    {
        tickWait = gpGlobals->tickcount + (shotsFired % 2 == 0 ? 12 : 13);
    }

    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();

    WeaponSound(GetWeaponSound("single_shot"));

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

    shotsFired += 1;

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
    VectorAngles(vForward, angForward);

    CalculateMuzzlePoint(trace, speed[DF_PLASMA], muzzle);

    CMomDFRocket *rocket = CMomDFRocket::EmitRocket(muzzle, angForward, pPlayer, DF_PLASMA, damageFactor);
    if (trace.fraction < 0.99)
    {
        rocket->Explode(&trace, trace.m_pEnt);
    }

    DecalPacket rocketPacket = DecalPacket::Rocket(muzzle, angForward);
    g_pMomentumGhostClient->SendDecalPacket(&rocketPacket);
#endif
}
