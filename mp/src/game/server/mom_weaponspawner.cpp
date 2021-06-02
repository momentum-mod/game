#include "cbase.h"
#include "mom_weaponspawner.h"

LINK_ENTITY_TO_CLASS(mom_weaponspawner, CMomWeaponspawner);

BEGIN_DATADESC(CMomWeaponspawner)
	DEFINE_AUTO_ARRAY_KEYFIELD(m_szWeaponName, FIELD_CHARACTER, "WeaponName"),
END_DATADESC()
/*
IMPLEMENT_SERVERCLASS_ST(CMomWeaponspawner, DT_MomWeaponspawner)
	SendPropString(SENDINFO(m_szWeaponName)),
END_SEND_TABLE();
*/
CMomWeaponspawner::CMomWeaponspawner()
{
	m_szWeaponName[0] = 0;
}

void CMomWeaponspawner::Spawn()
{
    Precache();

    SetModel(WEAPONSPAWNER_MODEL);
    SetSolid(SOLID_BBOX);
    UTIL_SetSize(this, Vector(-20, -20, -20), Vector(20, 20, 20));
}

void CMomWeaponspawner::Precache()
{
    PrecacheModel(WEAPONSPAWNER_MODEL);
    BaseClass::Precache();
}

void CMomWeaponspawner::Touch(CBaseEntity *pOther)
{
    Msg("touched!\n");
    BaseClass::Touch(pOther);
}


bool CMomWeaponspawner::KeyValue(const char* szKeyName, const char* szValue)
{
    if (FStrEq(szKeyName, "WeaponName"))
    {
        Q_strncpy(m_szWeaponName, szValue, MAX_WEAPON_STRING);
        return true;
    }

    return BaseClass::KeyValue(szKeyName, szValue);
}