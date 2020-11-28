#include "cbase.h"

#include "weapon_mom_concgrenade.h"
#include "mom_player_shared.h"
#include "in_buttons.h"

#include "momentum/mom_concgrenade.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#endif

#include "tier0/memdbgon.h"

// Minimum time the grenade needs to be cooked for before it can be thrown
#define CONC_THROW_DELAY 0.5f
#define CONC_THROWSPEED 660.0f
#define CONC_SPAWN_ANG_X 18.5f

#define CONC_SOUND_TIMER "timer"

static MAKE_TOGGLE_CONVAR(mom_conc_sound_timer_enable, "1", FCVAR_ARCHIVE, "Toggles the conc timer sound. 0 = OFF, 1 = ON\n");

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumConcGrenade, DT_MomentumConcGrenade);

BEGIN_NETWORK_TABLE(CMomentumConcGrenade, DT_MomentumConcGrenade)
#ifdef GAME_DLL
    SendPropBool(SENDINFO(m_bPrimed)),
    SendPropFloat(SENDINFO(m_flThrowTime), 0, SPROP_NOSCALE),
    SendPropFloat(SENDINFO(m_flTimer), 0, SPROP_NOSCALE),
#else
    RecvPropBool(RECVINFO(m_bPrimed)),
    RecvPropFloat(RECVINFO(m_flThrowTime)),
    RecvPropFloat(RECVINFO(m_flTimer)),
#endif
END_NETWORK_TABLE();

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CMomentumConcGrenade)
    DEFINE_PRED_FIELD(m_bPrimed, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
    DEFINE_PRED_FIELD(m_flThrowTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
    DEFINE_PRED_FIELD(m_flTimer, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_momentum_concgrenade, CMomentumConcGrenade);
PRECACHE_WEAPON_REGISTER(weapon_momentum_concgrenade);

CMomentumConcGrenade::CMomentumConcGrenade()
{
    m_bPrimed = false;
    m_flThrowTime = 0.0f;
    m_flTimer = 0.0f;
    m_iPrimaryAmmoType = AMMO_TYPE_GRENADE;
    m_bNeedsRepress = false;
}

bool CMomentumConcGrenade::Deploy()
{
    m_bPrimed = false;
    m_flThrowTime = 0.0f;
    m_flTimer = 0.0f;
    m_bNeedsRepress = false;

    return BaseClass::Deploy();
}

bool CMomentumConcGrenade::Holster(CBaseCombatWeapon *pSwitchingTo)
{
    m_bPrimed = false;
    m_flThrowTime = 0.0f;
    m_flTimer = 0.0f;
    m_bNeedsRepress = false;

    return BaseClass::Holster(pSwitchingTo);
}

void CMomentumConcGrenade::PrimaryAttack()
{
    if (m_bPrimed || m_flThrowTime > 0.0f || m_bNeedsRepress)
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    if (mom_conc_sound_timer_enable.GetBool())
        WeaponSound(GetWeaponSound(CONC_SOUND_TIMER));

    SendWeaponAnim(ACT_VM_PULLBACK);
    m_bPrimed = true;
    m_bNeedsRepress = true;

    if (m_flTimer <= 0)
    {
        m_flTimer = gpGlobals->curtime;
    }
}

void CMomentumConcGrenade::SecondaryAttack()
{
#ifdef GAME_DLL
    const auto pOwner = GetPlayerOwner();
    if (pOwner && pOwner->IsAlive())
    {
        pOwner->DestroyExplosives();
        StopWeaponSound(GetWeaponSound(CONC_SOUND_TIMER));
    }
#endif
}

void CMomentumConcGrenade::ItemPostFrame()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    // If they let go of the fire button, they want to throw the grenade.
    if (m_bPrimed && !(pPlayer->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
    {
        StartGrenadeThrow();
        m_bPrimed = false;
        m_bNeedsRepress = false;

        if (m_flTimer > 0.0f && gpGlobals->curtime - m_flTimer > CONC_THROW_DELAY) // throw conc with remaining fuse timer
        {
            ThrowGrenade(gpGlobals->curtime - m_flTimer);
        }
    }

    if (m_flThrowTime > 0.0f && m_flThrowTime < gpGlobals->curtime) // throw delay of 0.5s if player released attack right after priming conc
    {
        ThrowGrenade(CONC_THROW_DELAY);
    }
    else if (m_flTimer > 0.0f && gpGlobals->curtime - m_flTimer >= CONC_MAX_TIME) // explode in hand if grenade is still held when reaching max timer
    {
        ThrowGrenade(CONC_MAX_TIME);
        m_bPrimed = false;
    }
    else
    {
        if (!(pPlayer->m_nButtons & IN_ATTACK)) // Require player to release and repress +attack before another grenade can be thrown
        {
            m_bNeedsRepress = false;
        }

        BaseClass::ItemPostFrame();
    }
}

void CMomentumConcGrenade::StartGrenadeThrow()
{
    m_flThrowTime = gpGlobals->curtime + (0.5f - (gpGlobals->curtime - m_flTimer));
}

void CMomentumConcGrenade::ThrowGrenade(float flTimer)
{
#ifdef GAME_DLL
    CMomentumPlayer *pOwner = GetPlayerOwner();
    if (!pOwner)
        return;

    Vector vecVelocity;
    if (flTimer > 0.0f)
    {
        Vector vecForward;
        QAngle angAngles;

        pOwner->EyeVectors(&vecForward);
        VectorAngles(vecForward, angAngles);
        angAngles.x -= CONC_SPAWN_ANG_X;

        AngleVectors(angAngles, &vecVelocity);
        VectorNormalize(vecVelocity);

        vecVelocity *= CONC_THROWSPEED;
    }
    else
    {
        vecVelocity.Init();
    }

    const auto &vecSrc = pOwner->WorldSpaceCenter(); // Thrown from the waist

    // Online uses angles, but we're packing 3 floats so whatever
    QAngle vecThrowOnline(vecVelocity.x, vecVelocity.y, vecVelocity.z);
    DecalPacket packet = DecalPacket::ConcThrow(vecSrc, vecThrowOnline, flTimer);
    g_pMomentumGhostClient->SendDecalPacket(&packet);

    CMomConcProjectile::Create(flTimer, vecSrc, vecVelocity, pOwner);
#endif

    m_flThrowTime = 0.0f;
    m_flTimer = 0.0f;

    SendWeaponAnim(ACT_VM_THROW);

    m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
}