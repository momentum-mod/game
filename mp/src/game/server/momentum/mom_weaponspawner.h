#pragma once

#include "cbase.h"

class CMomWeaponspawner : public CBaseAnimating
{
public:
    DECLARE_CLASS(CMomWeaponspawner, CBaseAnimating);
    DECLARE_DATADESC();

    CMomWeaponspawner();

    void Spawn();
    void Precache(const char *weaponModel);
    void Touch(CBaseEntity *pOther);
    bool KeyValue(const char *szKeyName, const char *szValue);

    char m_szWeaponName[MAX_WEAPON_STRING];
};