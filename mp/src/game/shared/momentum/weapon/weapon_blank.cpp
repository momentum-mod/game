#include "cbase.h"

#include "weapon_blank.h"

#include "weapon_def.h"


#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(Blank, DT_WeaponBlank);

BEGIN_NETWORK_TABLE(CBlank, DT_WeaponBlank)
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(weapon_blank, CBlank);
PRECACHE_WEAPON_REGISTER(weapon_blank);

CBlank::CBlank() { }

void CBlank::PrimaryAttack() { }

void CBlank::SecondaryAttack() { }


void CBlank::WeaponIdle() { }


void CBlank::ItemPostFrame() { }

#ifdef CLIENT_DLL
// We want a blank viewmodel, so we don't render anything
bool C_Blank::IsOverridingViewmodel()
{
    return true;
}

// Our overridden viewmodel is not drawn, due to being hidden
int C_Blank::DrawOverriddenViewmodel(C_BaseViewModel* pViewmodel, int flags)
{
    return 0;
}
#endif

