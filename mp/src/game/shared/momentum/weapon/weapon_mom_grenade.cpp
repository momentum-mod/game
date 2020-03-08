#include "cbase.h"

#include "weapon_mom_grenade.h"

#include "datacache/imdlcache.h"
#include "in_buttons.h"
#include "mom_player_shared.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_grenade_projectile.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumGrenade, DT_MomentumGrenade);

BEGIN_NETWORK_TABLE(CMomentumGrenade, DT_MomentumGrenade)
#ifdef GAME_DLL
    SendPropBool(SENDINFO(m_bRedraw)),
    SendPropBool(SENDINFO(m_bPinPulled)),
    SendPropFloat(SENDINFO(m_fThrowTime), 0, SPROP_NOSCALE),
#else
    RecvPropBool(RECVINFO(m_bRedraw)),
    RecvPropBool(RECVINFO(m_bPinPulled)),
    RecvPropFloat(RECVINFO(m_fThrowTime)),
#endif
END_NETWORK_TABLE();

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CMomentumGrenade) 
DEFINE_PRED_FIELD(m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bPinPulled, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_fThrowTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_momentum_grenade, CMomentumGrenade);
PRECACHE_WEAPON_REGISTER(weapon_momentum_grenade);

CMomentumGrenade::CMomentumGrenade()
{
    m_bRedraw = false;
    m_bPinPulled = false;
    m_fThrowTime = 0.0f;
    m_iPrimaryAmmoType = AMMO_TYPE_GRENADE;
}

bool CMomentumGrenade::Deploy()
{
    m_bRedraw = false;
    m_bPinPulled = false;
    m_fThrowTime = 0.0f;

#ifndef CLIENT_DLL
    // if we're officially out of grenades, ditch this weapon
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
    {
        pPlayer->Weapon_Drop(this, nullptr, nullptr);
        UTIL_Remove(this);
        return false;
    }
#endif

    return BaseClass::Deploy();
}

bool CMomentumGrenade::Holster(CBaseCombatWeapon *pSwitchingTo)
{
    m_bRedraw = false;
    m_bPinPulled = false; // when this is holstered make sure the pin isn’t pulled.
    m_fThrowTime = 0;

#ifndef CLIENT_DLL
    // If they attempt to switch weapons before the throw animation is done,
    // allow it, but kill the weapon if we have to.
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
    {
        pPlayer->Weapon_Drop(this, nullptr, nullptr);
        UTIL_Remove(this);
    }
#endif

    return BaseClass::Holster(pSwitchingTo);
}

void CMomentumGrenade::PrimaryAttack()
{
    if (m_bRedraw || m_bPinPulled || m_fThrowTime > 0.0f)
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer || pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
        return;

    // The pull pin animation has to finish, then we wait until they aren't holding the primary
    // attack button, then throw the grenade.
    SendWeaponAnim(ACT_VM_PULLPIN);
    m_bPinPulled = true;

    // Don't let weapon idle interfere in the middle of a throw!
    MDLCACHE_CRITICAL_SECTION();
    SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration());

    m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CMomentumGrenade::SecondaryAttack()
{
    if (m_bRedraw)
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (pPlayer->GetFlags() & FL_DUCKING)
    {
        SendWeaponAnim(ACT_VM_SECONDARYATTACK);
    }
    else
    {
        SendWeaponAnim(ACT_VM_HAULBACK);
    }

    m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

bool CMomentumGrenade::Reload()
{
    if ((m_bRedraw) && (m_flNextPrimaryAttack <= gpGlobals->curtime) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
    {
        // Redraw the weapon
        SendWeaponAnim(ACT_VM_DRAW);

        // Update our times
        m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
        m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

        SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration());

        // Mark this as done
        //	m_bRedraw = false;
    }

    return true;
}

void CMomentumGrenade::ItemPostFrame()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    CBaseViewModel *vm = pPlayer->GetViewModel(m_nViewModelIndex);
    if (!vm)
        return;

    // If they let go of the fire button, they want to throw the grenade.
    if (m_bPinPulled && !(pPlayer->m_nButtons & IN_ATTACK))
    {
        // pPlayer->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );

        StartGrenadeThrow();

        MDLCACHE_CRITICAL_SECTION();
        m_bPinPulled = false;
        SendWeaponAnim(ACT_VM_THROW);
        SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration());

        // we're still throwing, so reset our next primary attack
        m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration(); 
    }
    else if ((m_fThrowTime > 0.0f) && (m_fThrowTime < gpGlobals->curtime))
    {
        // only decrement our ammo when we actually create the projectile
        DecrementAmmo(pPlayer);

        ThrowGrenade();
    }
    else if (m_bRedraw)
    {
        // Has the throw animation finished playing
        if (m_flTimeWeaponIdle < gpGlobals->curtime)
        {
#ifdef GAME_DLL
            // if we're officially out of grenades, ditch this weapon
            if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
            {
                pPlayer->Weapon_Drop(this, nullptr, nullptr);
                UTIL_Remove(this);
            }
            else
            {
                pPlayer->SwitchToNextBestWeapon(this);
            }
#endif
            // don't animate this grenade any more!
        }
    }
    else
    {
        BaseClass::ItemPostFrame();
    }
}

void CMomentumGrenade::StartGrenadeThrow() { m_fThrowTime = gpGlobals->curtime + 0.1f; }

void CMomentumGrenade::DecrementAmmo(CBaseCombatCharacter *pOwner)
{
#ifdef GAME_DLL
    pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
#endif
}

void CMomentumGrenade::ThrowGrenade()
{
#ifdef GAME_DLL
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
    {
        Assert(false);
        return;
    }

    QAngle angThrow = pPlayer->LocalEyeAngles();

    Vector vForward, vRight, vUp;

    if (angThrow.x < 0.0f)
        angThrow.x += 360.0f;

    if (angThrow.x < 90.0f)
        angThrow.x = -10.0f + angThrow.x * (100.0f / 90.0f);
    else
    {
        angThrow.x = 360.0f - angThrow.x;
        angThrow.x = -10.0f + angThrow.x * -(80.0f / 90.0f);
    }

    float flVel = (90.0f - angThrow.x) * 6.0f;

    if (flVel > 750.0f)
        flVel = 750.0f;

    AngleVectors(angThrow, &vForward, &vRight, &vUp);

    Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

    // We want to throw the grenade from 16 units out.  But that can cause problems if we're facing
    // a thin wall.  Do a hull trace to be safe.
    trace_t trace;
    Vector mins(-2, -2, -2);
    Vector maxs(2, 2, 2);
    UTIL_TraceHull(vecSrc, vecSrc + vForward * 16.0f, mins, maxs, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace);

    Vector vecThrow = vForward * flVel + pPlayer->GetAbsVelocity();

    EmitGrenade(trace.endpos, vec3_angle, vecThrow, pPlayer);
#endif

    m_bRedraw = true;
    m_fThrowTime = 0.0f;

    // if( pPlayer )
    //	pPlayer->Radio( "Radio.FireInTheHole",   "#Cstrike_TitlesTXT_Fire_in_the_hole" );
}

#ifdef GAME_DLL
void CMomentumGrenade::EmitGrenade(const Vector &vecSrc, const QAngle &vecAngles, const Vector &vecVel, CBaseEntity *pOwner)
{
    AngularImpulse angImpulse(600, random->RandomInt(-1200, 1200), 0);

    // Online uses angles, but we're packing 3 floats so whatever
    QAngle vecThrowOnline(vecVel.x, vecVel.y, vecVel.z);
    DecalPacket packet = DecalPacket::Bullet(vecSrc, vecThrowOnline, AMMO_TYPE_GRENADE, angImpulse.y, 0, 0.0f);
    g_pMomentumGhostClient->SendDecalPacket(&packet);

    CMomGrenadeProjectile::Create(vecSrc, vecAngles, vecVel, angImpulse, pOwner, GetWorldModel());
}
#endif