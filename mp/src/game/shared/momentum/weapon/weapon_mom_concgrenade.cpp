#include "cbase.h"

#include "weapon_mom_concgrenade.h"
#include "mom_player_shared.h"
#include "in_buttons.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_concgrenade.h"
#endif

#include "tier0/memdbgon.h"

// Minimum time the grenade needs to be cooked for before it can be thrown
#define CONC_THROW_DELAY 0.5f
#define CONC_THROWSPEED 660.0f
#define CONC_SPAWN_ANG_X 18.5f

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
}

bool CMomentumConcGrenade::Deploy()
{
    m_bPrimed = false;
    m_flThrowTime = 0.0f;
    m_flTimer = 0.0f;

    return BaseClass::Deploy();
}

bool CMomentumConcGrenade::Holster(CBaseCombatWeapon *pSwitchingTo)
{
    m_bPrimed = false;
    m_flThrowTime = 0.0f;
    m_flTimer = 0.0f;

    return BaseClass::Holster(pSwitchingTo);
}

void CMomentumConcGrenade::PrimaryAttack()
{
    if (m_bPrimed || m_flThrowTime > 0.0f)
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    WeaponSound(GetWeaponSound("timer"));
    m_bPrimed = true;

    if (m_flTimer <= 0)
    {
        m_flTimer = gpGlobals->curtime;
    }

    m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
}

void CMomentumConcGrenade::SecondaryAttack() {}

void CMomentumConcGrenade::ItemPostFrame()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    CBaseViewModel *vm = pPlayer->GetViewModel(m_nViewModelIndex);

    if (!vm)
        return;

    // If they let go of the fire button, they want to throw the grenade.
    if (m_bPrimed && !(pPlayer->m_nButtons & IN_ATTACK))
    {
        StartGrenadeThrow();
        m_bPrimed = false;

        if (m_flTimer > 0.0f && gpGlobals->curtime - m_flTimer > 0.5f) // throw conc with remaining fuse timer
        {
            ThrowGrenade(gpGlobals->curtime - m_flTimer);
        }
    }
    if (m_flThrowTime > 0.0f && m_flThrowTime < gpGlobals->curtime) // throw delay of 0.5s if player released attack right after priming conc
    {
        ThrowGrenade(0.5f);
    }
    else if (m_flTimer > 0.0f && gpGlobals->curtime - m_flTimer >= GetMaxTimer()) // explode in hand if grenade is still held when reaching max timer
    {
        ThrowGrenade(GetMaxTimer());
        m_bPrimed = false;
    }
    else
    {
        BaseClass::ItemPostFrame();
    }
}

void CMomentumConcGrenade::StartGrenadeThrow()
{
    m_flThrowTime = gpGlobals->curtime + (0.5f - (gpGlobals->curtime - m_flTimer));
}

void CMomentumConcGrenade::ThrowGrenade(float flTimer, float flSpeed)
{
#ifdef GAME_DLL
    CMomentumPlayer *pOwner = GetPlayerOwner();

    Vector vecForward, vecSrc, vecVelocity;
    QAngle angAngles;
    float playerPitch;

    pOwner->EyeVectors(&vecForward);
    vecSrc = pOwner->GetAbsOrigin() + Vector(0, 0, 36); // Thrown from the waist
    playerPitch = pOwner->GetLocalAngles().x;

    // Online uses angles, but we're packing 3 floats so whatever
    //QAngle vecThrowOnline(vecVelocity.x, vecVelocity.y, vecVelocity.z);
    //DecalPacket packet = DecalPacket::Bullet(vecSrc, vecThrowOnline, AMMO_TYPE_GRENADE, angImpulse.y, 0, 0.0f);
    //g_pMomentumGhostClient->SendDecalPacket(&packet);

    const auto pGrenade = dynamic_cast<CMomConcProjectile*>(CreateEntityByName("momentum_concgrenade"));

    VectorAngles(vecForward, angAngles);

    angAngles.x -= CONC_SPAWN_ANG_X;

    UTIL_SetOrigin(pGrenade, vecSrc);

    if (flTimer != 0)
    {
        AngleVectors(angAngles, &vecVelocity);
        VectorNormalize(vecVelocity);
        if (flSpeed > 0)
        {
            vecVelocity *= CONC_THROWSPEED;
        }
        else
        {
            vecVelocity *= flSpeed;
        }
    }
    else
    {
        vecVelocity = Vector(0, 0, 0);
    }

    if (flTimer >= GetMaxTimer())
    {
        pGrenade->SetHandheld(true);
        pGrenade->SetDetonateTimerLength(0);
    }
    else
    {
        pGrenade->SetDetonateTimerLength(GetMaxTimer() - flTimer);
    }

    DispatchSpawn(pGrenade);
    pGrenade->SetAbsVelocity(vecVelocity);
    pGrenade->SetupInitialTransmittedVelocity(vecVelocity);
    pGrenade->SetThrower(pOwner);
    pGrenade->SetGravity(pGrenade->GetGrenadeGravity());
    pGrenade->SetFriction(pGrenade->GetGrenadeFriction());
    pGrenade->SetElasticity(pGrenade->GetGrenadeElasticity());
    pGrenade->SetDamage(0.0f);
    //pGrenade->m_flSpawnTime = gpGlobals->curtime - (3.0f - flTimer); // This shold be done in a neater way!!

    pGrenade->SetThink(&CMomConcProjectile::GrenadeThink);
    pGrenade->SetNextThink(gpGlobals->curtime);
#endif

    m_flThrowTime = 0.0f;
    m_flTimer = 0.0f;
}