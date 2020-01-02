#include "cbase.h"

#include "in_buttons.h"
#include "mom_player_shared.h"
#include "mom_stickybomb.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_stickybomblauncher.h"

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

BEGIN_NETWORK_TABLE_NOBASE(CMomentumStickybombLauncher, DT_MomentumStickybombLauncherLocalData)
#ifdef CLIENT_DLL
    RecvPropInt(RECVINFO(m_iStickybombCount)),
    RecvPropFloat(RECVINFO(m_flChargeBeginTime)),
#else
    SendPropInt(SENDINFO(m_iStickybombCount), 5, SPROP_UNSIGNED),
    SendPropFloat(SENDINFO(m_flChargeBeginTime), 5, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),
#endif
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE(CMomentumStickybombLauncher, DT_MomentumStickybombLauncher)
#ifdef CLIENT_DLL
    RecvPropDataTable("MomentumStickybombLauncherLocalData", 0, 0, &REFERENCE_RECV_TABLE(DT_MomentumStickybombLauncherLocalData)),
#else
    SendPropDataTable("MomentumStickybombLauncherLocalData", 0, &REFERENCE_SEND_TABLE(DT_MomentumStickybombLauncherLocalData), SendProxy_SendLocalWeaponDataTable),
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
    "Toggle whether holding down primary fire charges stickies or fires them immediately.\n", nullptr,
    [](IConVar *pVar, const char *pNewVal)
    {
        const auto pPlayer = CMomentumPlayer::GetLocalPlayer();

        if (pPlayer)
        {
            const auto pLauncher = dynamic_cast<CMomentumStickybombLauncher *>(pPlayer->GetActiveWeapon());

            CTriggerZone *pZone = pPlayer->GetCurrentZoneTrigger();

            if (pLauncher && pLauncher->GetChargeBeginTime() > 0)
            {
                Warning("Cannot disable charge while charging!\n");
                return false;
            }
            if (pZone && pZone->GetZoneType() == ZONE_TYPE_START)
            {
                Warning("Cannot use charge while in a start zone!\n");
                return false;
            }
        }
 
        return true;
    }
);
#endif

#ifndef CLIENT_DLL
BEGIN_DATADESC(CMomentumStickybombLauncher)
END_DATADESC()
#endif

CMomentumStickybombLauncher::CMomentumStickybombLauncher()
{
    m_flTimeToIdleAfterFire = 0.6f;
    m_flLastDenySoundTime = 0.0f;
    m_flChargeBeginTime = 0.0f;
    #ifdef CLIENT_DLL
    m_iStickybombCount = 0;
    #else
    m_iStickybombCount.Set(0);
    #endif
}

CMomentumStickybombLauncher::~CMomentumStickybombLauncher()
{
}

void CMomentumStickybombLauncher::Precache()
{
    BaseClass::Precache();
    PrecacheScriptSound("StickybombLauncher.Single");
    PrecacheScriptSound("StickybombLauncher.Detonate");
    PrecacheScriptSound("StickybombLauncher.Deny");
    PrecacheScriptSound("StickybombLauncher.Charge");

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_stickybomb");
#endif
}

void CMomentumStickybombLauncher::Spawn()
{
    BaseClass::Spawn();
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

    // Get the player owning the weapon.
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

    CMomStickybomb *pProjectile = static_cast<CMomStickybomb*>(FireProjectile(pPlayer));
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
    WeaponSound(SINGLE);

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

        SendWeaponAnim(ACT_VM_PULLBACK);
        WeaponSound(SPECIAL3);
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
        else
        {
            // Play a detonate sound.
            WeaponSound(SPECIAL1);
        }
    }
}

float CMomentumStickybombLauncher::GetProjectileSpeed()
{
#ifdef CLIENT_DLL
    static ConVarRef mom_sj_charge_enable("mom_sj_charge_enable");
#endif

    float flForwardSpeed = RemapValClamped((gpGlobals->curtime - m_flChargeBeginTime), 
                           0.0f, 
                           MOM_STICKYBOMB_MAX_CHARGE_TIME, 
                           MOM_STICKYBOMB_MIN_CHARGE_VEL, 
                           MOM_STICKYBOMB_MAX_CHARGE_VEL);

    return mom_sj_charge_enable.GetBool() ? flForwardSpeed : 900.0f;
}


//-----------------------------------------------------------------------------
// Purpose: Add stickybombs to our list as they're fired
//-----------------------------------------------------------------------------
CBaseEntity *CMomentumStickybombLauncher::FireProjectile(CMomentumPlayer *pPlayer)
{
    CBaseEntity *pProjectile = FireStickybomb(pPlayer);
    if (pProjectile)
    {
#ifdef GAME_DLL
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

        CMomStickybomb *pStickybomb = (CMomStickybomb *)pProjectile;
        pStickybomb->SetLauncher(this);

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

#ifdef GAME_DLL
    Vector vecForward, vecRight, vecUp;
    float yOffset = 8.0f;

    if (!cl_righthand.GetBool())
    {
        yOffset *= -1.0f;
    }

    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    Vector vecSrc = pPlayer->Weapon_ShootPosition();

    vecSrc += vecForward * 16.0f + vecRight * yOffset + vecUp * -6.0f;
    Vector vecVelocity = (vecForward * GetProjectileSpeed()) + (vecUp * 200.0f);
    
	CMomStickybomb *pProjectile = CMomStickybomb::Create(
        vecSrc, pPlayer->EyeAngles(), vecVelocity, AngularImpulse(600, 0, 0), pPlayer);

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