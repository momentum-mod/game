#include "cbase.h"

#include "weapon_mom_smg.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumMachinegun, DT_MomentumMachinegun);

BEGIN_NETWORK_TABLE(CMomentumMachinegun, DT_MomentumMachinegun)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumMachinegun)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_machinegun, CMomentumMachinegun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_machinegun);


CMomentumMachinegun::CMomentumMachinegun()
{
    m_flIdleInterval = 20.0f;
    m_flTimeToIdleAfterFire = 2.0f;

    m_iPrimaryAmmoType = AMMO_TYPE_MACHINEGUN;
}

void CMomentumMachinegun::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (!BaseGunFire(0.0f, 0.07f, true))
        return;

    //MOM_TODO: Do we want this kickback? Should it be convar'd?

    // Kick the gun based on the state of the player.
    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        pPlayer->KickBack(1.0, 0.55, 0.40, 0.04, 6.0, 4.0, 4);
    else if (pPlayer->GetAbsVelocity().Length2D() > 5)
        pPlayer->KickBack(0.30, 0.20, 0.10, 0.0260, 3.8, 2.15, 5.8);
    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        pPlayer->KickBack(0.275, 0.2, 0.125, 0.02, 3, 1, 9);
    else
        pPlayer->KickBack(0.3, 0.225, 0.125, 0.02, 3.25, 1.25, 8);
}
