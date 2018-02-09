//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "fx_mom_shared.h"
#include "mom_player_shared.h"
#include "weapon_csbasegun.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponCSBaseGun, DT_WeaponCSBaseGun)

BEGIN_NETWORK_TABLE(CWeaponCSBaseGun, DT_WeaponCSBaseGun)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponCSBaseGun)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_csbase_gun, CWeaponCSBaseGun);

CWeaponCSBaseGun::CWeaponCSBaseGun()
{
    m_flTimeToIdleAfterFire = 2.0f;
    m_flIdleInterval = 20.0f;
    m_zoomFullyActiveTime = -1.0f;
}

void CWeaponCSBaseGun::Spawn()
{
    m_flAccuracy = 0.2;
    m_bDelayFire = false;
    m_zoomFullyActiveTime = -1.0f;

    BaseClass::Spawn();
}

bool CWeaponCSBaseGun::Deploy()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    m_flAccuracy = 0.2;
    pPlayer->m_SrvData.m_iShotsFired = 0;
    m_bDelayFire = false;
    m_zoomFullyActiveTime = -1.0f;

    return BaseClass::Deploy();
}

void CWeaponCSBaseGun::ItemPostFrame()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    if ((m_flNextPrimaryAttack <= gpGlobals->curtime) && (pPlayer->m_SrvData.m_bResumeZoom))
    {
#ifndef CLIENT_DLL
        pPlayer->SetFOV(pPlayer, pPlayer->m_SrvData.m_iLastZoom, 0.05f);
        m_zoomFullyActiveTime =
            gpGlobals->curtime +
            0.05f; // Make sure we think that we are zooming on the server so we don't get instant acc bonus

        if (pPlayer->GetFOV() == pPlayer->m_SrvData.m_iLastZoom)
        {
            // return the fade level in zoom.
            pPlayer->m_SrvData.m_bResumeZoom = false;
        }
#endif
    }

    BaseClass::ItemPostFrame();
}

void CWeaponCSBaseGun::PrimaryAttack()
{
    // Derived classes should implement this and call CSBaseGunFire.
    Assert(false);
}

bool CWeaponCSBaseGun::CSBaseGunFire(float flSpread, float flCycleTime, bool bPrimaryMode)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    m_bDelayFire = true;
    pPlayer->m_SrvData.m_iShotsFired++;

// Out of ammo?
#ifdef WEAPONS_USE_AMMO
    if (m_iClip1 <= 0)
    {
        if (m_bFireOnEmpty)
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
        }

        return false;
    }

    m_iClip1--;
#endif

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);

    FX_FireBullets(pPlayer->entindex(), pPlayer->Weapon_ShootPosition(),
                   pPlayer->EyeAngles() + 2.0f * pPlayer->GetPunchAngle(), GetWeaponID(),
                   bPrimaryMode ? Primary_Mode : Secondary_Mode, CBaseEntity::GetPredictionRandomSeed() & 255,
                   flSpread);

    DoFireEffects();

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + flCycleTime;

#ifdef WEAPONS_USE_AMMO
    if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
    {
        // HEV suit - indicate out of ammo condition
        pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
    }
#endif

    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    return true;
}

void CWeaponCSBaseGun::DoFireEffects()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (pPlayer)
        pPlayer->DoMuzzleFlash();
}

#ifdef WEAPONS_USE_AMMO
bool CWeaponCSBaseGun::Reload()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
        return false;

    int iResult = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
    if (!iResult)
        return false;

    pPlayer->SetAnimation(PLAYER_RELOAD);

#ifndef CLIENT_DLL
    if ((iResult) && (pPlayer->GetFOV() != pPlayer->GetDefaultFOV()))
    {
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV());
    }
#endif

    m_flAccuracy = 0.2;
    pPlayer->m_SrvData.m_iShotsFired = 0;
    m_bDelayFire = false;

    return true;
}
#endif

void CWeaponCSBaseGun::WeaponIdle()
{
    if (m_flTimeWeaponIdle > gpGlobals->curtime)
        return;

    // only idle if the slid isn't back
#ifdef WEAPONS_USE_AMMO
    if (m_iClip1 != 0)
#endif
    {
        SetWeaponIdleTime(gpGlobals->curtime + m_flIdleInterval);
        SendWeaponAnim(ACT_VM_IDLE);
    }
}
