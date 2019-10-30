#include "cbase.h"

#include "in_buttons.h"
#include "mom_player_shared.h"
#include "mom_stickybomb.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_stickybomblauncher.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumStickybombLauncher, DT_MomentumStickybombLauncher)

BEGIN_NETWORK_TABLE(CMomentumStickybombLauncher, DT_MomentumStickybombLauncher)
#ifdef CLIENT_DLL
RecvPropInt(RECVINFO(m_iStickybombCount)),
#else
SendPropInt(SENDINFO(m_iStickybombCount), 5, SPROP_UNSIGNED),
#endif
    END_NETWORK_TABLE()
#ifdef CLIENT_DLL
        BEGIN_PREDICTION_DATA(CMomentumStickybombLauncher) DEFINE_FIELD(m_flChargeBeginTime, FIELD_FLOAT)
            END_PREDICTION_DATA()
#endif

                LINK_ENTITY_TO_CLASS(weapon_momentum_stickylauncher, CMomentumStickybombLauncher);
PRECACHE_WEAPON_REGISTER(weapon_momentum_stickylauncher);

#define MOM_STICKYBOMB_MIN_CHARGE_VEL 900;
#define MOM_STICKYBOMB_MAX_CHARGE_VEL 2400;
#define MOM_STICKYBOMB_MAX_CHARGE_TIME 4.0f;
#define MOM_WEAPON_STICKYBOMB_COUNT 8;

#ifdef GAME_DLL
// static MAKE_TOGGLE_CONVAR(mom_rj_center_fire, "0", FCVAR_ARCHIVE,
//                          "If enabled, all rockets will be fired from the center of the screen. 0 = OFF, 1 = ON\n");
#endif

CMomentumStickybombLauncher::CMomentumStickybombLauncher()
{
    m_flTimeToIdleAfterFire = 0.7f;
    m_flIdleInterval = 20.0f;
    m_flLastDenySoundTime = 0.0f;
    m_flChargeBeginTime = 0.0f;
    m_iStickybombCount = 0;
}

void CMomentumStickybombLauncher::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_stickybomb");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we holster
//-----------------------------------------------------------------------------
bool CMomentumStickybombLauncher::Holster(CBaseCombatWeapon *pSwitchingTo)
{
    m_flChargeBeginTime = 0;

    return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we deploy
//-----------------------------------------------------------------------------
bool CMomentumStickybombLauncher::Deploy()
{
    m_flChargeBeginTime = 0;

    return BaseClass::Deploy();
}

void CMomentumStickybombLauncher::WeaponIdle()
{
    if (m_flChargeBeginTime > 0)
    {
        LaunchGrenade();
    }
    else
    {
        BaseClass::WeaponIdle();
    }
}

void CMomentumStickybombLauncher::LaunchGrenade() 
{
    // Get the player owning the weapon.
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifdef GAME_DLL
    CMomStickybomb *pProjectile = static_cast<CMomStickybomb*>(FireProjectile(pPlayer));
    if (pProjectile)
    {
        // Save the charge time to scale the detonation timer.
        pProjectile->SetChargeTime(gpGlobals->curtime - m_flChargeBeginTime);
    }
#endif

    // Set next attack times.
    m_flNextPrimaryAttack = gpGlobals->curtime + 0.7f;
    m_flLastDenySoundTime = gpGlobals->curtime;
    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

	DoFireEffects();
    WeaponSound(SINGLE);

    m_flChargeBeginTime = 0;
}

void CMomentumStickybombLauncher::ItemBusyFrame()
{
#ifdef GAME_DLL
    CBasePlayer *pOwner = ToBasePlayer(GetOwner());
    if (pOwner && pOwner->m_nButtons & (IN_ATTACK2)) // IN_ATTACK2
    {
        // We need to do this to catch the case of player trying to detonate
        // stickybombs while in the middle of reloading.
        StickybombLauncherDetonate();
    }
#endif

    BaseClass::ItemBusyFrame();
}

void CMomentumStickybombLauncher::StickybombLauncherFire()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    // Are we capable of firing again?
    if (m_flNextPrimaryAttack > gpGlobals->curtime)
        return;

    if (m_flChargeBeginTime <= 0)
    {
        // save that we had the attack button down
        m_flChargeBeginTime = gpGlobals->curtime;

        SendWeaponAnim(ACT_VM_PULLBACK);
        WeaponSound(SPECIAL3);
    }
    else
    {
        float flTotalChargeTime = gpGlobals->curtime - m_flChargeBeginTime;

        //WeaponSound(SPECIAL3);

        if (flTotalChargeTime >= 4.0f)
        {
            LaunchGrenade();
        }
    }
}

void CMomentumStickybombLauncher::StickybombLauncherDetonate()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (m_iStickybombCount)
    {
        if (!pPlayer)
            return;

        if (DetonateRemoteStickybombs(false))
        {
            if (m_flLastDenySoundTime <= gpGlobals->curtime)
            {
                // Deny!
                m_flLastDenySoundTime = gpGlobals->curtime + 1;
                WeaponSound(SPECIAL2);
                return;
            }
        }
    }
    else
    {
        WeaponSound(SPECIAL1);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Return the origin & angles for a projectile fired from the player's gun
//-----------------------------------------------------------------------------
void CMomentumStickybombLauncher::GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc,
                                                         QAngle *angForward)
{
#ifdef GAME_DLL
    static ConVarRef cl_righthand("cl_righthand");
#else
    extern ConVar cl_righthand;
#endif
    if (!cl_righthand.GetBool())
    {
        vecOffset.y *= -1.0f;
    }

    Vector vecForward, vecRight, vecUp;
    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    Vector vecShootPos = pPlayer->Weapon_ShootPosition();

    // Estimate end point
    Vector endPos = vecShootPos + vecForward * 2000.0f;

    // Trace forward and find what's in front of us, and aim at that
    trace_t tr;

    CTraceFilterSimple filter(pPlayer, COLLISION_GROUP_NONE);
    UTIL_TraceLine(vecShootPos, endPos, MASK_SOLID, &filter, &tr);

    // Offset actual start point
    *vecSrc = vecShootPos + (vecForward * vecOffset.x) + (vecRight * vecOffset.y) + (vecUp * vecOffset.z);

    // Find angles that will get us to our desired end point
    // Only use the trace end if it wasn't too close, which results
    // in visually bizarre forward angles
    if (tr.fraction > 0.1f)
    {
        VectorAngles(tr.endpos - *vecSrc, *angForward);
    }
    else
    {
        VectorAngles(endPos - *vecSrc, *angForward);
    }
}

float CMomentumStickybombLauncher::GetProjectileSpeed()
{
    return RemapValClamped((gpGlobals->curtime - m_flChargeBeginTime), 0.0f, 4.0f, 900, 2400);
}

CBaseEntity *CMomentumStickybombLauncher::FireProjectile(CMomentumPlayer *pPlayer)
{
    CBaseEntity *pProjectile = FireStickybomb(pPlayer);
    if (pProjectile)
    {
#ifdef GAME_DLL
        // If we've gone over the max stickybomb count, detonate the oldest
        if (m_Stickybombs.Count() >= 8)
        {
            CMomStickybomb *pTemp = m_Stickybombs[0];
            if (pTemp)
            {
                pTemp->Detonate(); // explode NOW
            }

            m_Stickybombs.Remove(0);
        }

        CMomStickybomb *pStickybomb = (CMomStickybomb *)pProjectile;
        StickybombHandle hHandle;
        hHandle = pStickybomb;
        m_Stickybombs.AddToTail(hHandle);

        m_iStickybombCount = m_Stickybombs.Count();
#endif
    }
    return pProjectile;
}

CBaseEntity *CMomentumStickybombLauncher::FireStickybomb(CMomentumPlayer *pPlayer)
{
#ifdef GAME_DLL
    static ConVarRef cl_righthand("cl_righthand");
#else
    extern ConVar cl_righthand;
#endif
    Vector vecForward, vecRight, vecUp;

    if (!cl_righthand.GetBool())
    {
        vecRight.y *= -1.0f;
    }

#ifdef GAME_DLL

    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    // Create grenades here!!
    Vector vecSrc = pPlayer->Weapon_ShootPosition();
    vecSrc += vecForward * 16.0f + vecRight * 8.0f + vecUp * -6.0f;

    Vector vecVelocity = (vecForward * GetProjectileSpeed()) + (vecUp * 200.0f) + (random->RandomFloat(-10.0f, 10.0f) * vecRight) +
                         (random->RandomFloat(-10.0f, 10.0f) * vecUp);

    CMomStickybomb *pProjectile = CMomStickybomb::Create(
        vecSrc, pPlayer->EyeAngles(), vecVelocity, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer);

    return pProjectile;
#endif

    return NULL;
}

void CMomentumStickybombLauncher::AddStickybomb(CMomStickybomb *pBomb)
{
    StickybombHandle hHandle;
    hHandle = pBomb;
    m_Stickybombs.AddToTail(hHandle);
}

//-----------------------------------------------------------------------------
// Purpose: If a stickybomb has been removed, remove it from our list
//-----------------------------------------------------------------------------
void CMomentumStickybombLauncher::DeathNotice(CBaseEntity *pVictim)
{
    Assert(dynamic_cast<CMomStickybomb *>(pVictim));

    StickybombHandle hHandle;
    hHandle = (CMomStickybomb *)pVictim;
    m_Stickybombs.FindAndRemove(hHandle);

    m_iStickybombCount = m_Stickybombs.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Remove *with* explosions
//-----------------------------------------------------------------------------
bool CMomentumStickybombLauncher::DetonateRemoteStickybombs(bool bFizzle)
{
    bool bFailedToDetonate = false;

    int count = m_Stickybombs.Count();

    for (int i = 0; i < count; i++)
    {
        CMomStickybomb *pTemp = m_Stickybombs[i];
        if (pTemp)
        {
            // This guy will die soon enough.
            if (pTemp->IsEffectActive(EF_NODRAW))
                continue;
#ifdef GAME_DLL
            if (bFizzle)
            {
                pTemp->Fizzle();
            }
#endif
            if (bFizzle == false)
            {
                if ((gpGlobals->curtime - pTemp->GetCreationTime()) < 0.8f) // Stickybomb arm time
                {
                    bFailedToDetonate = true;
                    continue;
                }
            }
#ifdef GAME_DLL
            pTemp->Detonate();
#endif
        }
    }

    return bFailedToDetonate;
}

float CMomentumStickybombLauncher::GetChargeMaxTime() { return MOM_STICKYBOMB_MAX_CHARGE_TIME; }

void CMomentumStickybombLauncher::PrimaryAttack() { StickybombLauncherFire(); }

void CMomentumStickybombLauncher::SecondaryAttack() { StickybombLauncherDetonate(); }

bool CMomentumStickybombLauncher::CanDeploy()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_SJ))
        return false;

    return BaseClass::CanDeploy();
}