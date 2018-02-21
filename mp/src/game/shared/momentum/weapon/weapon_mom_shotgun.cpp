#include "cbase.h"
#include "weapon_mom_shotgun.h"
#include "fx_mom_shared.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumShotgun, DT_MomentumShotgun);

BEGIN_NETWORK_TABLE(CMomentumShotgun, DT_MomentumShotgun)
#ifdef CLIENT_DLL
RecvPropInt(RECVINFO(m_fInSpecialReload))
#else
SendPropInt(SENDINFO(m_fInSpecialReload), 2, SPROP_UNSIGNED)
#endif
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumShotgun)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_shotgun, CMomentumShotgun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_shotgun);

CMomentumShotgun::CMomentumShotgun()
{
    m_flPumpTime = 0;
}

void CMomentumShotgun::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    // don't fire underwater
    if (pPlayer->GetWaterLevel() == 3)
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
        return;
    }

#ifdef WEAPONS_USE_AMMO
    if (m_iClip1 <= 0)
    {
        Reload();

        if (m_iClip1 == 0)
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
        }

        return;
    }
    m_iClip1--;
#endif

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->DoMuzzleFlash();

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);

    // Dispatch the FX right away with full accuracy.
    FX_FireBullets(pPlayer->entindex(), pPlayer->Weapon_ShootPosition(),
                   pPlayer->EyeAngles() + 2.0f * pPlayer->GetPunchAngle(), GetWeaponID(), Primary_Mode,
                   GetPredictionRandomSeed() &
                       255, // wrap it for network traffic so it's the same between client and server
                   0.0725   // flSpread
                   );

#ifdef WEAPONS_USE_AMMO
    if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
    {
        // HEV suit - indicate out of ammo condition
        pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
    }

    if (m_iClip1 != 0)
#endif
        m_flPumpTime = gpGlobals->curtime + 0.5;

    m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
    m_flNextSecondaryAttack = gpGlobals->curtime + 0.25;
#ifdef WEAPONS_USE_AMMO
    if (m_iClip1 != 0)
        SetWeaponIdleTime(gpGlobals->curtime + 2.5);
    else
        SetWeaponIdleTime(gpGlobals->curtime + 0.25);
#else
    SetWeaponIdleTime(gpGlobals->curtime + 2.5f);
#endif

    m_fInSpecialReload = 0;

    // Update punch angles.
    QAngle angle = pPlayer->GetPunchAngle();

    if (pPlayer->GetFlags() & FL_ONGROUND)
    {
        angle.x -= SharedRandomInt("XM1014PunchAngleGround", 3, 5);
    }
    else
    {
        angle.x -= SharedRandomInt("XM1014PunchAngleAir", 7, 10);
    }

    pPlayer->SetPunchAngle(angle);
}

#ifdef WEAPONS_USE_AMMO
bool CMomentumShotgun::Reload()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 || m_iClip1 == GetMaxClip1())
        return true;

    // don't reload until recoil is done
    if (m_flNextPrimaryAttack > gpGlobals->curtime)
        return true;

    // MIKETODO: shotgun reloading (wait until we get content)

    // check to see if we're ready to reload
    if (m_fInSpecialReload == 0)
    {
        pPlayer->SetAnimation(PLAYER_RELOAD);

        SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);
        m_fInSpecialReload = 1;
        pPlayer->m_flNextAttack = gpGlobals->curtime + 0.5;
        SetWeaponIdleTime(gpGlobals->curtime + 0.5);
        m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
        m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;

#ifdef GAME_DLL
// pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_START );
#endif

        return true;
    }
    else if (m_fInSpecialReload == 1)
    {
        if (m_flTimeWeaponIdle > gpGlobals->curtime)
            return true;
        // was waiting for gun to move to side
        m_fInSpecialReload = 2;

        SendWeaponAnim(ACT_VM_RELOAD);
        SetWeaponIdleTime(gpGlobals->curtime + 0.5);
#ifdef GAME_DLL
        if (m_iClip1 == 6)
        {
            // pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_END );
        }
        else
        {
            // pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_LOOP );
        }
#endif
    }
    else
    {
        // Add them to the clip
        m_iClip1 += 1;

#ifdef GAME_DLL
        SendReloadEvents();
#endif

        if (pPlayer)
            pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

        m_fInSpecialReload = 1;
    }

    return true;
}
#endif

void CMomentumShotgun::WeaponIdle()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (m_flPumpTime && m_flPumpTime < gpGlobals->curtime)
    {
        // play pumping sound
        m_flPumpTime = 0;
    }

    if (m_flTimeWeaponIdle < gpGlobals->curtime)
    {
#ifdef WEAPONS_USE_AMMO
        if (m_iClip1 == 0 && m_fInSpecialReload == 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType))
        {
            Reload();
        }
        else if (m_fInSpecialReload != 0)
        {
            if (m_iClip1 != 7 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType))
            {
                Reload();
            }
            else
            {
                // reload debounce has timed out
                // MIKETODO: shotgun anims
                SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

                // play cocking sound
                m_fInSpecialReload = 0;
                SetWeaponIdleTime(gpGlobals->curtime + 1.5);
            }
        }
        else
#endif
        {
            SendWeaponAnim(ACT_VM_IDLE);
        }
    }
}