#include "cbase.h"

#include "weapon_mom_stickybomblauncher.h"

#include "in_buttons.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"

#ifdef CLIENT_DLL
#include "prediction.h"
#else
#include "momentum/mom_triggers.h"
#endif

#include "tier0/memdbgon.h"

#define MOM_STICKYBOMB_MIN_CHARGE_VEL 900
#define MOM_STICKYBOMB_MAX_CHARGE_VEL 2400
#define MOM_STICKYBOMB_MAX_CHARGE_TIME 4.0f
#define MOM_WEAPON_STICKYBOMB_COUNT 3

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumStickybombLauncher, DT_MomentumStickybombLauncher)

BEGIN_NETWORK_TABLE(CMomentumStickybombLauncher, DT_MomentumStickybombLauncher)
#ifdef CLIENT_DLL
    RecvPropInt(RECVINFO(m_iStickybombCount)),
    RecvPropFloat(RECVINFO(m_flChargeBeginTime)),
    RecvPropBool(RECVINFO(m_bIsChargeEnabled)),
#else
    SendPropInt(SENDINFO(m_iStickybombCount), 5, SPROP_UNSIGNED),
    SendPropFloat(SENDINFO(m_flChargeBeginTime), 5, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),
    SendPropBool(SENDINFO(m_bIsChargeEnabled)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CMomentumStickybombLauncher) 
    DEFINE_FIELD(m_flChargeBeginTime, FIELD_FLOAT)
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_momentum_stickylauncher, CMomentumStickybombLauncher);
PRECACHE_WEAPON_REGISTER(weapon_momentum_stickylauncher);

#ifdef GAME_DLL
static MAKE_TOGGLE_CONVAR_CV(mom_sj_charge_enable, "1", FCVAR_ARCHIVE,
    "Toggles sticky launcher firing mode. 0 = Stickies are fired instantly when holding primary fire,\n1 = Stickies are charged when holding primary fire.\n", nullptr,
    [](IConVar *pVar, const char *pNewVal)
    {
        const auto pPlayer = CMomentumPlayer::GetLocalPlayer();

        if (pPlayer)
        {
            const auto pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(pPlayer->GetActiveWeapon());

            if (pLauncher && pLauncher->GetChargeBeginTime() > 0)
            {
                Warning("Cannot disable charge while charging!\n");
                return false;
            }
        }

        return true;
    }
);
#endif

CMomentumStickybombLauncher::CMomentumStickybombLauncher()
{
    m_flTimeToIdleAfterFire = 0.6f;
    m_flLastDenySoundTime = 0.0f;
    m_flChargeBeginTime = 0.0f;
    m_bIsChargeEnabled.Set(true);
    m_iStickybombCount.Set(0);
}

CMomentumStickybombLauncher::~CMomentumStickybombLauncher()
{
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
#ifdef CLIENT_DLL
    static ConVarRef mom_sj_charge_enable("mom_sj_charge_enable");
#endif

    if (m_flChargeBeginTime > 0 && mom_sj_charge_enable.GetBool())
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
#ifdef CLIENT_DLL
    static ConVarRef mom_sj_charge_enable("mom_sj_charge_enable");
#endif

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

    const auto pProjectile = FireProjectile(pPlayer);
    if (pProjectile && mom_sj_charge_enable.GetBool())
    {
        // Save the charge time to scale the detonation timer.
        pProjectile->SetChargeTime(gpGlobals->curtime - m_flChargeBeginTime);
    }

    // Set next attack times.
    m_flNextPrimaryAttack = gpGlobals->curtime + 0.6f;
    m_flLastDenySoundTime = gpGlobals->curtime;

    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();
    WeaponSound(GetWeaponSound("single_shot"));

    m_flChargeBeginTime = 0;
}

void CMomentumStickybombLauncher::ItemBusyFrame()
{
#ifdef GAME_DLL
    CMomentumPlayer *pOwner = GetPlayerOwner();
    if (pOwner && pOwner->m_nButtons & (IN_ATTACK2))
    {
        SecondaryAttack();
    }
#endif

    BaseClass::ItemBusyFrame();
}

void CMomentumStickybombLauncher::ItemPostFrame()
{
#ifdef CLIENT_DLL
    static ConVarRef mom_sj_charge_enable("mom_sj_charge_enable");
#endif

    BaseClass::ItemPostFrame();

    // Allow player to fire and detonate at the same time.
    CMomentumPlayer *pOwner = GetPlayerOwner();
    if (pOwner && !(pOwner->m_nButtons & IN_ATTACK))
    {
        if (m_flChargeBeginTime > 0 && mom_sj_charge_enable.GetBool())
        {
            LaunchGrenade();
        }
    }
}

void CMomentumStickybombLauncher::PrimaryAttack()
{
#ifdef CLIENT_DLL
    static ConVarRef mom_sj_charge_enable("mom_sj_charge_enable");
#endif

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    // Are we capable of firing again?
    if (m_flNextPrimaryAttack > gpGlobals->curtime)
        return;

    if (m_flChargeBeginTime <= 0 && mom_sj_charge_enable.GetBool())
    {
        // save that we had the attack button down
        m_flChargeBeginTime = gpGlobals->curtime;

        if (m_bIsChargeEnabled.Get())
        {
            SendWeaponAnim(ACT_VM_PULLBACK);
            WeaponSound(GetWeaponSound("charge"));
        }
    }
    else
    {
        float flTotalChargeTime = gpGlobals->curtime - m_flChargeBeginTime;

        if (flTotalChargeTime >= MOM_STICKYBOMB_MAX_CHARGE_TIME)
        {
            LaunchGrenade();
        }
    }
}

void CMomentumStickybombLauncher::SecondaryAttack()
{
    if (m_iStickybombCount == 0)
        return;

    const auto pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    if (DetonateRemoteStickybombs(false))
    {
        if (m_flLastDenySoundTime <= gpGlobals->curtime)
        {
            // Deny!
            m_flLastDenySoundTime = gpGlobals->curtime + 1;
            WeaponSound(GetWeaponSound("deny"));
        }
    }
    else
    {
        // Play a detonate sound.
        WeaponSound(GetWeaponSound("detonate"));
    }
}

float CMomentumStickybombLauncher::CalculateProjectileSpeed(float flProgress)
{
    return RemapValClamped(flProgress, 0.0f, MOM_STICKYBOMB_MAX_CHARGE_TIME,
                           MOM_STICKYBOMB_MIN_CHARGE_VEL, MOM_STICKYBOMB_MAX_CHARGE_VEL);
}

float CMomentumStickybombLauncher::GetProjectileSpeed()
{
#ifdef CLIENT_DLL
    static ConVarRef mom_sj_charge_enable("mom_sj_charge_enable");
#endif

    if (!mom_sj_charge_enable.GetBool())
        return 900.0f;

    return CalculateProjectileSpeed(gpGlobals->curtime - m_flChargeBeginTime);
}


//-----------------------------------------------------------------------------
// Purpose: Add stickybombs to our list as they're fired
//-----------------------------------------------------------------------------
CMomStickybomb *CMomentumStickybombLauncher::FireProjectile(CMomentumPlayer *pPlayer)
{
    const auto pProjectile = FireStickybomb(pPlayer);
#ifdef GAME_DLL
    if (pProjectile)
    {
        // If we've gone over the max stickybomb count, fizzle the oldest
        if (m_Stickybombs.Count() >= MOM_WEAPON_STICKYBOMB_COUNT)
        {
            CMomStickybomb *pTemp = m_Stickybombs[0];
            if (pTemp)
            {
                pTemp->Fizzle();
                pTemp->RemoveStickybomb(true);
            }

            m_Stickybombs.Remove(0);
        }

        CMomStickybomb *pStickybomb = pProjectile;
        pStickybomb->SetLauncher(this);

        AddStickybomb(pStickybomb);

        m_iStickybombCount = m_Stickybombs.Count();
    }
#endif
    return pProjectile;
}

CMomStickybomb *CMomentumStickybombLauncher::FireStickybomb(CMomentumPlayer *pPlayer)
{
#ifdef GAME_DLL
    static ConVarRef cl_righthand("cl_righthand");

    // Clamp forwards velocity to 900 if inside start zone or mom_sj_charge_enable is 0
    float vel = GetProjectileSpeed();
    CTriggerZone *pZone = pPlayer->GetCurrentZoneTrigger();

    if (pZone && pZone->GetZoneType() == ZONE_TYPE_START)
    {
        if (pZone->IsTouching(pPlayer))
        {
            vel = 900.0f;
        }
        else
        {
            vel = GetProjectileSpeed();
        }
    }

    Vector vecForward, vecRight, vecUp;
    float yOffset = 8.0f;

    if (!cl_righthand.GetBool())
    {
        yOffset *= -1.0f;
    }

    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    Vector vecSrc = pPlayer->Weapon_ShootPosition();

    vecSrc += vecForward * 16.0f + vecRight * yOffset + vecUp * -6.0f;
    Vector vecVelocity = (vecForward * vel) + (vecUp * 200.0f);

    return CMomStickybomb::Create(vecSrc, pPlayer->EyeAngles(), vecVelocity, AngularImpulse(600, 0, 0), pPlayer);
#endif
    return nullptr;
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
    const auto pSticky = dynamic_cast<CMomStickybomb *>(pVictim);
    Assert(pSticky);

    StickybombHandle hHandle;
    hHandle = pSticky;
    m_Stickybombs.FindAndRemove(hHandle);

    m_iStickybombCount = m_Stickybombs.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Remove *with* explosions
//-----------------------------------------------------------------------------
bool CMomentumStickybombLauncher::DetonateRemoteStickybombs(bool bFizzle)
{
    bool bFailedToDetonate = false;

    FOR_EACH_VEC_BACK(m_Stickybombs, i)
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
            if (!bFizzle)
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

float CMomentumStickybombLauncher::GetChargeMaxTime()
{
    return MOM_STICKYBOMB_MAX_CHARGE_TIME;
}

bool CMomentumStickybombLauncher::CanDeploy()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_SJ))
        return false;

    return BaseClass::CanDeploy();
}