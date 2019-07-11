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
    m_flTimeToIdleAfterFire = 0.8f;
    m_flIdleInterval = 20.0f;
}

void CMomentumRocketLauncher::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_rocket");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Return the origin & angles for a projectile fired from the player's gun
//-----------------------------------------------------------------------------
void CMomentumRocketLauncher::GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward)
{
    Vector vecForward, vecRight, vecUp;
    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    Vector vecShootPos = pPlayer->Weapon_ShootPosition();

    // Estimate end point
    Vector endPos = vecShootPos + vecForward * 2000;

    // Trace forward and find what's in front of us, and aim at that
    trace_t tr;

    CTraceFilterSimple filter(pPlayer, COLLISION_GROUP_NONE);
    UTIL_TraceLine(vecShootPos, endPos, MASK_SOLID, &filter, &tr);

    // Offset actual start point
    *vecSrc = vecShootPos + (vecForward * vecOffset.x) + (vecRight * vecOffset.y) + (vecUp * vecOffset.z);

    // Find angles that will get us to our desired end point
    // Only use the trace end if it wasn't too close, which results
    // in visually bizarre forward angles
    if (tr.fraction > 0.1)
    {
        VectorAngles(tr.endpos - *vecSrc, *angForward);
    }
    else
    {
        VectorAngles(endPos - *vecSrc, *angForward);
    }
}

void CMomentumRocketLauncher::RocketLauncherFire()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.8f;
    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();
    WeaponSound(SINGLE);

    // MOM_FIXME:
    // This will cause an assertion error.
    // Prevents us from using BaseGunFire() as well.
    //SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    // player "shoot" animation
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

    CMomRocket::EmitRocket(vecSrc, angForward, pPlayer);
#endif
}

void CMomentumRocketLauncher::PrimaryAttack()
{
    RocketLauncherFire();
}

bool CMomentumRocketLauncher::CanDeploy()
{
    ConVarRef gm("mom_gamemode");
    if (gm.GetInt() != GAMEMODE_RJ)
        return false;

    return BaseClass::CanDeploy();
}