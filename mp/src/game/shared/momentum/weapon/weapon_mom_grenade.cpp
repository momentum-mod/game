#include "cbase.h"
#include "weapon_mom_grenade.h"
#include "basegrenade_shared.h"

#ifdef GAME_DLL
#include <momentum/mom_grenade_projectile.h>
#endif

#include "tier0/memdbgon.h"


#define GRENADE_TIMER 3.0f //Seconds

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumGrenade, DT_MomentumGrenade)

BEGIN_NETWORK_TABLE(CMomentumGrenade, DT_MomentumGrenade)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CMomentumGrenade)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_grenade, CMomentumGrenade);
PRECACHE_WEAPON_REGISTER(weapon_momentum_grenade);


#ifndef CLIENT_DLL

void CMomentumGrenade::EmitGrenade(Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer)
{
    CMomGrenadeProjectile::Create(vecSrc, vecAngles, vecVel, angImpulse, pPlayer, GRENADE_TIMER);
}

#endif