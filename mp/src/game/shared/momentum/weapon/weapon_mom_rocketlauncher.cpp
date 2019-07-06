#include "cbase.h"

#include "mom_player_shared.h"
#include "weapon_mom_rocketlauncher.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumRocketLauncher, DT_MomentumRocketLauncher)

BEGIN_NETWORK_TABLE(CMomentumRocketLauncher, DT_MomentumRocketLauncher)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumRocketLauncher)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_rocketlauncher, CMomentumRocketLauncher);
PRECACHE_WEAPON_REGISTER(weapon_momentum_rocketlauncher);

CMomentumRocketLauncher::CMomentumRocketLauncher()
{
    m_flTimeToIdleAfterFire = 1.9f;
    m_flIdleInterval = 20.0f;
}

void CMomentumRocketLauncher::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_rocket");
#endif
}

void CMomentumRocketLauncher::RocketLauncherFire()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.8f;

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

    Vector muzzlePoint = pPlayer->Weapon_ShootPosition() + (vForward * vecOffset.x) + (vRight * vecOffset.y) + (vUp * vecOffset.z);

    QAngle vecAngles;
    VectorAngles(vForward, vecAngles);

    CMomentumRocket::EmitRocket(muzzlePoint, vecAngles, pPlayer);
#endif

    WeaponSound(SINGLE);

    // MOM_FIXME:
    // This will cause an assertion error.
    // SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CMomentumRocketLauncher::PrimaryAttack()
{
    RocketLauncherFire();
}