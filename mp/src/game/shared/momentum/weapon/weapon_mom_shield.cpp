#include "cbase.h"
#include "weapon_mom_shield.h"
#include "fx_mom_shared.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumShield, DT_MomentumShield);

BEGIN_NETWORK_TABLE(CMomentumShield, DT_MomentumShield)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumShield)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_shield, CMomentumShield);
PRECACHE_WEAPON_REGISTER(weapon_momentum_shield);

CMomentumShield::CMomentumShield()
{
}

void CMomentumShield::PrimaryAttack()
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

    pPlayer->m_flChargeTime = gpGlobals->curtime + 1.5;

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);

    m_flNextPrimaryAttack = gpGlobals->curtime + PrimaryFireTime();
    m_flNextSecondaryAttack = gpGlobals->curtime + PrimaryFireTime();

    SetWeaponIdleTime(gpGlobals->curtime + IdleTime());
}

void CMomentumShield::WeaponIdle()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (m_flTimeWeaponIdle < gpGlobals->curtime)
    {
        SendWeaponAnim(ACT_VM_IDLE);
    }
}