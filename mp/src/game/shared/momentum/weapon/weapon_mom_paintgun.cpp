#include "cbase.h"
#include "weapon_mom_paintgun.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumPaintGun, DT_MomentumPaintGun)

BEGIN_NETWORK_TABLE(CMomentumPaintGun, DT_MomentumPaintGun)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumPaintGun)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_paintgun, CMomentumPaintGun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_paintgun);

void CMomentumPaintGun::RifleFire()
{
    // Hardcoded here so people don't change the text files for easy spam
    if (!CSBaseGunFire(0.0f, 0.05f, true))
        return;
}

void CMomentumPaintGun::PrimaryAttack() { RifleFire(); }
