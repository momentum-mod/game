#include "cbase.h"
#include "ai_activity.h"
#include "weapon_mom_pistol.h"
#include "fx_mom_shared.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"


IMPLEMENT_NETWORKCLASS_ALIASED(MomentumPistol, DT_MomentumPistol)

BEGIN_NETWORK_TABLE(CMomentumPistol, DT_MomentumPistol)
#ifdef CLIENT_DLL
    RecvPropBool(RECVINFO(m_bBurstMode))
#else
    SendPropBool(SENDINFO(m_bBurstMode))
#endif
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumPistol)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_pistol, CMomentumPistol);
PRECACHE_WEAPON_REGISTER(weapon_momentum_pistol);


CMomentumPistol::CMomentumPistol()
{
    m_iPistolShotsFired = 0;
    m_flPistolShoot = 0.0f;
    m_flAccuracy = 0.9f;
    m_flLastFire = gpGlobals->curtime;

    m_iPrimaryAmmoType = AMMO_TYPE_PISTOL;
}


void CMomentumPistol::Spawn()
{
    BaseClass::Spawn();
    m_bBurstMode = false;
    m_iPistolShotsFired = 0;
    m_flPistolShoot = 0.0f;
    m_flNextPrimaryAttack = 0.0f;
    m_flNextSecondaryAttack = 0.0f;
}

bool CMomentumPistol::Deploy()
{
    m_iPistolShotsFired = 0;
    m_flPistolShoot = 0.0f;
    return BaseClass::Deploy();
}

Activity CMomentumPistol::GetDrawActivity()
{
    return m_bBurstMode ? ACT_VM_DRAW_SPECIAL : BaseClass::GetDrawActivity();
}

void CMomentumPistol::SecondaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_SJ))
    {
        const auto pWeapon = pPlayer->GetWeapon(WEAPON_STICKYLAUNCHER);
        if (pWeapon)
        {
            pWeapon->SecondaryAttack();
        }
    }
    else
    {
        ToggleBurst(pPlayer);
    }
}

void CMomentumPistol::FireRemaining(int &shotsFired, float &shootTime) const
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
    {
        Error("%s !pPlayer\n", __FUNCTION__);
    }

#ifdef WEAPONS_USE_AMMO
    m_iClip1--;
    if (m_iClip1 < 0)
    {
        m_iClip1 = 0;
        shotsFired = 3;
        shootTime = 0.0f;
        return;
    }
#endif

    // Dispatch the FX right away with full accuracy.
    FX_FireBullets(
        pPlayer->entindex(),
        pPlayer->Weapon_ShootPosition(),
        pPlayer->EyeAngles() + 2.0f * pPlayer->GetPunchAngle(),
        m_iPrimaryAmmoType,
        true,
        GetPredictionRandomSeed() & 255, // wrap it for network traffic so it's the same between client and server
        0.0f);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

    shotsFired++;

    if (shotsFired != 3)
        shootTime = gpGlobals->curtime + 0.1f;
    else
        shootTime = 0.0;
}


void CMomentumPistol::ItemPostFrame()
{
    if (m_flPistolShoot != 0.0)
        FireRemaining(m_iPistolShotsFired, m_flPistolShoot);

    BaseClass::ItemPostFrame();
}

void CMomentumPistol::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    float flCycleTime = 0.15f;

    if (m_bBurstMode)
    {
        m_iPistolShotsFired = 0;
        flCycleTime = 0.5f;
    }
    else
    {
        ++pPlayer->m_iShotsFired;

        if (pPlayer->m_iShotsFired > 1)
            return;
    }

    m_flLastFire = gpGlobals->curtime;

#ifdef WEAPONS_USE_AMMO
    if (m_iClip1 <= 0)
    {
        if (m_bFireOnEmpty)
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
        }

        return;
    }

    m_iClip1--;
#endif

    pPlayer->DoMuzzleFlash();

    Vector vecShootOrigin2;  //The origin of the shot 
    QAngle	angShootDir2;    //The angle of the shot

    //We need to figure out where to place the particle effect, so look up where the muzzle is
    // MOM_TODO: Make this a better effect
    /* VxPDX: I originally re-enabled this and planned creating a better smoke system, but using this makes the particle system
       throw an error when switching to a weapon without the muzzle attachment or, if it has one, attaches itself to next weapon's
       muzzle.
    */
    //DispatchParticleEffect("weapon_muzzle_smoke", PATTACH_POINT_FOLLOW, GetPlayerOwner()->GetViewModel(), LookupAttachment("muzzle"));

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);

    FX_FireBullets(
        pPlayer->entindex(),
        pPlayer->Weapon_ShootPosition(),
        pPlayer->EyeAngles() + 2.0f * pPlayer->GetPunchAngle(),
        m_iPrimaryAmmoType,
        false,
        GetPredictionRandomSeed() & 255, // wrap it for network traffic so it's the same between client and server
        0.0f);

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + flCycleTime;

#ifdef WEAPONS_USE_AMMO
    if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
    {
        // HEV suit - indicate out of ammo condition
        pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
    }
#endif

    SetWeaponIdleTime(gpGlobals->curtime + 2.5f);

    if (m_bBurstMode)
    {
        // Fire off the next two rounds
        m_flPistolShoot = gpGlobals->curtime + 0.1f;
        m_iPistolShotsFired++;

        SendWeaponAnim(ACT_VM_SECONDARYATTACK);
    }
    else
    {
        SendWeaponAnim(ACT_VM_PRIMARYATTACK);
    }
}

#ifdef WEAPONS_USE_AMMO
bool CMomentumPistol::Reload()
{
    if (m_flPistolShoot != 0)
        return true;

    if (!DefaultPistolReload())
        return false;

    m_flAccuracy = 0.9;
    return true;
}
#endif

void CMomentumPistol::WeaponIdle()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (m_flTimeWeaponIdle > gpGlobals->curtime)
        return;
    
    SendWeaponAnim(m_bBurstMode ? ACT_VM_IDLE_LOWERED : ACT_VM_IDLE);

#ifdef WEAPONS_USE_AMMO
    // only idle if the slid isn't back
    if (m_iClip1 != 0)
#endif
}

void CMomentumPistol::ToggleBurst(CMomentumPlayer *pPlayer)
{
    // MOM_TODO: Create an effect (animation maybe, lights on the gun?) that shows switch, don't wanna use ugly text
    if (m_bBurstMode)
    {
        ClientPrint(pPlayer, HUD_PRINTCENTER, "#MOM_Weapon_SwitchToSemiAuto");
        m_bBurstMode = false;
        SendWeaponAnim(ACT_VM_LOWERED_TO_IDLE);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTCENTER, "#MOM_Weapon_SwitchToBurstFire");
        m_bBurstMode = true;
        SendWeaponAnim(ACT_VM_IDLE_TO_LOWERED);
    }

    m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.8;
}