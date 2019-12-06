#include "cbase.h"
#include "weapon_mom_grenade.h"
#include "basegrenade_shared.h"
#include "datacache/imdlcache.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "mom_player_shared.h"
#include "weapon_base.h"

#ifdef GAME_DLL
#include "items.h"
#include "momentum/ghost_client.h"
#include <momentum/mom_grenade_projectile.h>
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumGrenade, DT_MomentumGrenade)

BEGIN_NETWORK_TABLE(CMomentumGrenade, DT_MomentumGrenade)
#ifndef CLIENT_DLL
SendPropBool(SENDINFO(m_bRedraw)), SendPropBool(SENDINFO(m_bPinPulled)),
SendPropFloat(SENDINFO(m_fThrowTime), 0, SPROP_NOSCALE),
#else
RecvPropBool(RECVINFO(m_bRedraw)), RecvPropBool(RECVINFO(m_bPinPulled)), RecvPropFloat(RECVINFO(m_fThrowTime)),
#endif
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA(CMomentumGrenade) 
DEFINE_PRED_FIELD(m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_momentum_grenade, CMomentumGrenade);
PRECACHE_WEAPON_REGISTER(weapon_momentum_grenade);

#ifndef CLIENT_DLL

void CMomentumGrenade::EmitGrenade(const Vector &vecSrc, const QAngle &vecAngles, const Vector &vecVel,
                                   AngularImpulse angImpulse, CBaseEntity *pOwner)
{
    CMomGrenadeProjectile::Create(vecSrc, vecAngles, vecVel, angImpulse, pOwner, GetWorldModel());
}

#endif

CMomentumGrenade::CMomentumGrenade()
{
    m_bRedraw = false;
    m_bPinPulled = false;
    m_fThrowTime = 0;
    m_iPrimaryAmmoType = AMMO_TYPE_GRENADE;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMomentumGrenade::Precache() { BaseClass::Precache(); }

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMomentumGrenade::Deploy()
{
    m_bRedraw = false;
    m_bPinPulled = false;
    m_fThrowTime = 0;

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

//-----------------------------------------------------------------------------
// Purpose:
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CMomentumGrenade::Holster(CBaseCombatWeapon *pSwitchingTo)
{
    m_bRedraw = false;
    m_bPinPulled = false; // when this is holstered make sure the pin isn�t pulled.
    m_fThrowTime = 0;

#ifndef CLIENT_DLL
    // If they attempt to switch weapons before the throw animation is done,
    // allow it, but kill the weapon if we have to.
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
    {
        CBaseCombatCharacter *pOwner = (CBaseCombatCharacter *)pPlayer;
        pOwner->Weapon_Drop(this);
        UTIL_Remove(this);
    }
#endif

    return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMomentumGrenade::SecondaryAttack()
{
    if (m_bRedraw)
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (pPlayer == nullptr)
        return;

    // See if we're ducking
    if (pPlayer->GetFlags() & FL_DUCKING)
    {
        // Send the weapon animation
        SendWeaponAnim(ACT_VM_SECONDARYATTACK);
    }
    else
    {
        // Send the weapon animation
        SendWeaponAnim(ACT_VM_HAULBACK);
    }

    // Don't let weapon idle interfere in the middle of a throw!
    SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration());

    m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose:
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
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
    else if ((m_fThrowTime > 0) && (m_fThrowTime < gpGlobals->curtime))
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

#ifdef CLIENT_DLL

void CMomentumGrenade::DecrementAmmo(CBaseCombatCharacter *pOwner) {}

void CMomentumGrenade::DropGrenade()
{
    m_bRedraw = true;
    m_fThrowTime = 0.0f;
}

void CMomentumGrenade::ThrowGrenade()
{
    m_bRedraw = true;
    m_fThrowTime = 0.0f;
}

void CMomentumGrenade::StartGrenadeThrow() { m_fThrowTime = gpGlobals->curtime + 0.1f; }

#else

BEGIN_DATADESC(CMomentumGrenade)
DEFINE_FIELD(m_bRedraw, FIELD_BOOLEAN), 
END_DATADESC()

int CMomentumGrenade::CapabilitiesGet()
{
    return bits_CAP_WEAPON_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pOwner -
//-----------------------------------------------------------------------------
void CMomentumGrenade::DecrementAmmo(CBaseCombatCharacter *pOwner) { pOwner->RemoveAmmo(1, m_iPrimaryAmmoType); }

void CMomentumGrenade::StartGrenadeThrow() { m_fThrowTime = gpGlobals->curtime + 0.1f; }

void CMomentumGrenade::ThrowGrenade()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
    {
        Assert(false);
        return;
    }

    QAngle angThrow = pPlayer->LocalEyeAngles();

    Vector vForward, vRight, vUp;

    if (angThrow.x < 0)
        angThrow.x += 360;

    if (angThrow.x < 90)
        angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);
    else
    {
        angThrow.x = 360.0f - angThrow.x;
        angThrow.x = -10 + angThrow.x * -((90 - 10) / 90.0);
    }

    float flVel = (90 - angThrow.x) * 6;

    if (flVel > 750)
        flVel = 750;

    AngleVectors(angThrow, &vForward, &vRight, &vUp);

    Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

    // We want to throw the grenade from 16 units out.  But that can cause problems if we're facing
    // a thin wall.  Do a hull trace to be safe.
    trace_t trace;
    Vector mins(-2, -2, -2);
    Vector maxs(2, 2, 2);
    UTIL_TraceHull(vecSrc, vecSrc + vForward * 16, mins, maxs, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace);
    vecSrc = trace.endpos;

    Vector vecThrow = vForward * flVel + pPlayer->GetAbsVelocity();
    int impulseInt = random->RandomInt(-1200, 1200);

#ifdef GAME_DLL
    // Online uses angles, but we're packing 3 floats so whatever
    QAngle vecThrowOnline(vecThrow.x, vecThrow.y, vecThrow.z);
    DecalPacket packet = DecalPacket::Bullet(vecSrc, vecThrowOnline, AMMO_TYPE_GRENADE, impulseInt, 0, 0.0f);
    g_pMomentumGhostClient->SendDecalPacket(&packet);
#endif

    EmitGrenade(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, impulseInt, 0), pPlayer);

    m_bRedraw = true;
    m_fThrowTime = 0.0f;

    // if( pPlayer )
    //	pPlayer->Radio( "Radio.FireInTheHole",   "#Cstrike_TitlesTXT_Fire_in_the_hole" );
}

void CMomentumGrenade::DropGrenade()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
    {
        Assert(false);
        return;
    }

    Vector vForward;
    pPlayer->EyeVectors(&vForward);
    Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset() + vForward * 16;

    Vector vecVel = pPlayer->GetAbsVelocity();

    EmitGrenade(vecSrc, vec3_angle, vecVel, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer);

    m_bRedraw = true;
    m_fThrowTime = 0.0f;
}

bool CMomentumGrenade::AllowsAutoSwitchFrom(void) const { return !m_bPinPulled; }

#endif
