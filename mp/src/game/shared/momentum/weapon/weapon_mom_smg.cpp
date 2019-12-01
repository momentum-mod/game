#include "cbase.h"

#include "weapon_mom_smg.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumSMG, DT_MomentumSMG);

BEGIN_NETWORK_TABLE(CMomentumSMG, DT_MomentumSMG)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumSMG)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_smg, CMomentumSMG);
PRECACHE_WEAPON_REGISTER(weapon_momentum_smg);


CMomentumSMG::CMomentumSMG()
{
    m_flIdleInterval = 20.0f;
    m_flTimeToIdleAfterFire = 2.0f;

    m_iPrimaryAmmoType = AMMO_TYPE_SMG;
}


void CMomentumSMG::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (!BaseGunFire(0.0f, 0.07f, true))
        return;

    //MOM_TODO: Do we want this kickback? Should it be convar'd?

    // Kick the gun based on the state of the player.
    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        pPlayer->KickBack(0.9, 0.45, 0.35, 0.04, 5.25, 3.5, 4);
    else if (pPlayer->GetAbsVelocity().Length2D() > 5)
        pPlayer->KickBack(0.45, 0.3, 0.2, 0.0275, 4, 2.25, 7);
    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        pPlayer->KickBack(0.275, 0.2, 0.125, 0.02, 3, 1, 9);
    else
        pPlayer->KickBack(0.3, 0.225, 0.125, 0.02, 3.25, 1.25, 8);
}