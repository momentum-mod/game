#include "cbase.h"

#include "weapon_mom_lmg.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"


IMPLEMENT_NETWORKCLASS_ALIASED(MomentumLMG, DT_MomentumLMG)

BEGIN_NETWORK_TABLE(CMomentumLMG, DT_MomentumLMG)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumLMG)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_lmg, CMomentumLMG);
PRECACHE_WEAPON_REGISTER(weapon_momentum_lmg);

CMomentumLMG::CMomentumLMG()
{
    m_flTimeToIdleAfterFire = 1.6f;
    m_flIdleInterval = 20.0f;
};

void CMomentumLMG::PrimaryAttack()
{
    if (!BaseGunFire(0.0f, 0.08f, true))
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    // CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
    if (!pPlayer)
        return;

    //MOM_TODO: Do we want this kickback? Should it be convar'd?
    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        pPlayer->KickBack(1.8, 0.65, 0.45, 0.125, 5, 3.5, 8);

    else if (pPlayer->GetAbsVelocity().Length2D() > 5)
        pPlayer->KickBack(1.1, 0.5, 0.3, 0.06, 4, 3, 8);

    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        pPlayer->KickBack(0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9);

    else
        pPlayer->KickBack(0.8, 0.35, 0.3, 0.03, 3.75, 3, 9);
}