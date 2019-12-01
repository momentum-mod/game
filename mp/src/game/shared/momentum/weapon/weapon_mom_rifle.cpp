#include "cbase.h"

#include "mom_player_shared.h"
#include "weapon_mom_rifle.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumRifle, DT_MomentumRifle)

BEGIN_NETWORK_TABLE(CMomentumRifle, DT_MomentumRifle)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumRifle)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_rifle, CMomentumRifle);
PRECACHE_WEAPON_REGISTER(weapon_momentum_rifle);

CMomentumRifle::CMomentumRifle()
{
    m_flTimeToIdleAfterFire = 1.9f;
    m_flIdleInterval = 20.0f;

    m_iPrimaryAmmoType = AMMO_TYPE_RIFLE;
}

void CMomentumRifle::PrimaryAttack()
{
    if (!BaseGunFire(0.0f, 0.1f, true))
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    // CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
    if (!pPlayer)
        return;

    // MOM_TODO: Do we want this kickback? Should it be convar'd?
    if (pPlayer->GetAbsVelocity().Length2D() > 5)
        pPlayer->KickBack(1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7);
    else if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        pPlayer->KickBack(2, 1.0, 0.5, 0.35, 9, 6, 5);
    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        pPlayer->KickBack(0.9, 0.35, 0.15, 0.025, 5.5, 1.5, 9);
    else
        pPlayer->KickBack(1, 0.375, 0.175, 0.0375, 5.75, 1.75, 8);
}