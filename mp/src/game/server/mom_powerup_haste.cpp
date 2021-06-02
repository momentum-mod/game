#include "cbase.h"
#include "mom_powerup_haste.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"

LINK_ENTITY_TO_CLASS(mom_powerup_haste, CMomPowerupHaste);

BEGIN_DATADESC(CMomPowerupHaste)
	DEFINE_KEYFIELD(m_flHasteTime, FIELD_FLOAT, "HasteTime")
END_DATADESC()

CMomPowerupHaste::CMomWeaponspawner()
{
}

void CMomPowerupHaste::Spawn()
{
    Precache();

    SetModel(HASTE_MODEL);
    SetSolid(SOLID_BBOX);
    SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
    UTIL_SetSize(this, Vector(-30, -30, -30), Vector(30, 30, 30));
}

void CMomPowerupHaste::Precache()
{
    PrecacheModel(HASTE_MODEL);
    BaseClass::Precache();
}

void CMomPowerupHaste::Touch(CBaseEntity *pOther)
{
    const auto pPlayer = ToCMOMPlayer(pOther);
    
    BaseClass::Touch(pOther);
}