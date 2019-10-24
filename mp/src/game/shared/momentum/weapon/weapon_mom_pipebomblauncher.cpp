#include "cbase.h"

#include "mom_pipebomb.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_pipebomblauncher.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumPipebombLauncher, DT_MomentumPipebombLauncher)

BEGIN_NETWORK_TABLE(CMomentumPipebombLauncher, DT_MomentumPipebombLauncher)
#ifdef CLIENT_DLL
RecvPropInt(RECVINFO(m_iPipebombCount)),
#else
SendPropInt(SENDINFO(m_iPipebombCount), 5, SPROP_UNSIGNED),
#endif
    END_NETWORK_TABLE()
#ifdef CLIENT_DLL
        BEGIN_PREDICTION_DATA(CMomentumPipebombLauncher) DEFINE_FIELD(m_flChargeBeginTime, FIELD_FLOAT)
            END_PREDICTION_DATA()
#endif

                LINK_ENTITY_TO_CLASS(weapon_momentum_pipebomblauncher, CMomentumPipebombLauncher);
PRECACHE_WEAPON_REGISTER(weapon_momentum_pipebomblauncher);

#define MOM_PIPEBOMB_MIN_CHARGE_VEL 900;
#define MOM_PIPEBOMB_MAX_CHARGE_VEL 2400;
#define MOM_PIPEBOMB_MAX_CHARGE_TIME 4.0f;
#define MOM_WEAPON_PIPEBOMB_COUNT 8;

#ifdef GAME_DLL
// static MAKE_TOGGLE_CONVAR(mom_rj_center_fire, "0", FCVAR_ARCHIVE,
//                          "If enabled, all rockets will be fired from the center of the screen. 0 = OFF, 1 = ON\n");
#endif

CMomentumPipebombLauncher::CMomentumPipebombLauncher()
{
    m_flTimeToIdleAfterFire = 0.7f;
    m_flIdleInterval = 20.0f;
    m_flLastDenySoundTime = 0.0f;
    m_flChargeBeginTime = 0.0f;
}

void CMomentumPipebombLauncher::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_pipebomb");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we holster
//-----------------------------------------------------------------------------
bool CMomentumPipebombLauncher::Holster(CBaseCombatWeapon *pSwitchingTo)
{
    m_flChargeBeginTime = 0;

    return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we deploy
//-----------------------------------------------------------------------------
bool CMomentumPipebombLauncher::Deploy(void)
{
    m_flChargeBeginTime = 0;

    return BaseClass::Deploy();
}

void CMomentumPipebombLauncher::WeaponIdle(void)
{
    if (m_flChargeBeginTime > 0)
    {
        // LaunchGrenade();
    }
    else
    {
        BaseClass::WeaponIdle();
    }
}

void CMomentumPipebombLauncher::ItemBusyFrame(void)
{
#ifdef GAME_DLL
    CBasePlayer *pOwner = ToBasePlayer(GetOwner());
    if (pOwner && pOwner->m_nButtons & (1 << 11)) // IN_ATTACK2
    {
        // We need to do this to catch the case of player trying to detonate
        // pipebombs while in the middle of reloading.
        PipebombLauncherDetonate();
    }
#endif

    BaseClass::ItemBusyFrame();
}

void CMomentumPipebombLauncher::PipebombLauncherFire()
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
    }
    else
    {
        float flTotalChargeTime = gpGlobals->curtime - m_flChargeBeginTime;

        WeaponSound(SPECIAL3);

        if (flTotalChargeTime >= 4.0f)
        {
            //FireProjectile(pPlayer);
        }
    }

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.7f;
    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();
    WeaponSound(SINGLE);

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    // player "shoot" animation
    pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifdef GAME_DLL

	/*
    Vector vForward, vRight, vUp;

    pPlayer->EyeVectors(&vForward, &vRight, &vUp);

    // Offset values from
    // https://github.com/NicknineTheEagle/TF2-Base/blob/fcc7d838c289e0ec0e0dd1a189f51750cf21beb8/src/game/shared/tf/tf_weaponbase_gun.cpp#L396
    Vector vecOffset(16.0f, 8.0f, -6.0f);
    if (pPlayer->GetFlags() & FL_DUCKING)
    {
        vecOffset.z = 8.0f;
    }
    Vector vecSrc;
    QAngle angForward;
    GetProjectileFireSetup(pPlayer, vecOffset, &vecSrc, &angForward);

    trace_t trace;
    CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);
    UTIL_TraceLine(pPlayer->EyePosition(), vecSrc, MASK_SOLID_BRUSHONLY, &traceFilter, &trace);
	*/

    FireProjectile(pPlayer);
#endif
}

void CMomentumPipebombLauncher::PipebombLauncherDetonate()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (m_iPipebombCount)
    {
        if (!pPlayer)
            return;

        if (DetonateRemotePipebombs(false))
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
void CMomentumPipebombLauncher::GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc,
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

    // Vector vecVelocity = (vecForward * GetProjectileSpeed()) + (vecUp * 200.0f) +
    //                     (random->RandomFloat(-10.0f, 10.0f) * vecRight) + (random->RandomFloat(-10.0f, 10.0f) *
    //                     vecUp);

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

float CMomentumPipebombLauncher::GetProjectileSpeed(void)
{
    float flForwardSpeed = RemapValClamped((gpGlobals->curtime - m_flChargeBeginTime), 0.0f, 4.0f, 900, 2400);

    return flForwardSpeed;
}

CBaseEntity *CMomentumPipebombLauncher::FireProjectile(CMomentumPlayer *pPlayer)
{
    CBaseEntity *pProjectile = FirePipebomb(pPlayer);
    if (pProjectile)
    {
#ifdef GAME_DLL
        // If we've gone over the max pipebomb count, detonate the oldest
        if (m_Pipebombs.Count() >= 8)
        {
            CMomPipebomb *pTemp = m_Pipebombs[0];
            if (pTemp)
            {
                pTemp->Detonate(); // explode NOW
            }

            m_Pipebombs.Remove(0);
        }

        CMomPipebomb *pPipebomb = (CMomPipebomb *)pProjectile;

        PipebombHandle hHandle;
        hHandle = pPipebomb;
        m_Pipebombs.AddToTail(hHandle);

        m_iPipebombCount = m_Pipebombs.Count();
#endif
    }
    return pProjectile;
}

CBaseEntity *CMomentumPipebombLauncher::FirePipebomb(CMomentumPlayer *pPlayer)
{
#ifdef GAME_DLL

    Vector vecForward, vecRight, vecUp;
    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    // Create grenades here!!
    Vector vecSrc = pPlayer->Weapon_ShootPosition();
    vecSrc += vecForward * 16.0f + vecRight * 8.0f + vecUp * -6.0f;

    Vector vecVelocity = (vecForward * 900.0f) + (vecUp * 200.0f) +
                         (random->RandomFloat(-10.0f, 10.0f) * vecRight) + (random->RandomFloat(-10.0f, 10.0f) * vecUp);

    CMomPipebomb *pProjectile = CMomPipebomb::Create(
        vecSrc, pPlayer->EyeAngles(), vecVelocity, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer);

    return pProjectile;
#endif

    return NULL;
}

void CMomentumPipebombLauncher::AddPipeBomb(CMomPipebomb *pBomb)
{
    PipebombHandle hHandle;
    hHandle = pBomb;
    m_Pipebombs.AddToTail(hHandle);
}

//-----------------------------------------------------------------------------
// Purpose: If a pipebomb has been removed, remove it from our list
//-----------------------------------------------------------------------------
void CMomentumPipebombLauncher::DeathNotice(CBaseEntity *pVictim)
{
    Assert(dynamic_cast<CMomPipebomb *>(pVictim));

    PipebombHandle hHandle;
    hHandle = (CMomPipebomb *)pVictim;
    m_Pipebombs.FindAndRemove(hHandle);

    m_iPipebombCount = m_Pipebombs.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Remove *with* explosions
//-----------------------------------------------------------------------------
bool CMomentumPipebombLauncher::DetonateRemotePipebombs(bool bFizzle)
{
    bool bFailedToDetonate = false;

    int count = m_Pipebombs.Count();

    for (int i = 0; i < count; i++)
    {
        CMomPipebomb *pTemp = m_Pipebombs[i];
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
#else
            if (bFizzle == false)
            {
                if ((gpGlobals->curtime - pTemp->m_flCreationTime) < 0.8f) // Pipebomb arm time
                {
                    bFailedToDetonate = true;
                    continue;
                }
            }
#endif
#ifdef GAME_DLL

            pTemp->Detonate();
#endif
        }
    }

    return bFailedToDetonate;
}

float CMomentumPipebombLauncher::GetChargeMaxTime(void) { return MOM_PIPEBOMB_MAX_CHARGE_TIME; }

void CMomentumPipebombLauncher::PrimaryAttack() { PipebombLauncherFire(); }

void CMomentumPipebombLauncher::SecondaryAttack() { PipebombLauncherDetonate(); }

bool CMomentumPipebombLauncher::CanDeploy()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
        return false;

    return BaseClass::CanDeploy();
}