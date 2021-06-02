#pragma once

#include "cbase.h"

#define WEAPONSPAWNER_MODEL "models/gibs/airboat_broken_engine.mdl"

class CMomWeaponspawner : public CBaseAnimating
{
public:
    DECLARE_CLASS(CMomWeaponspawner, CBaseAnimating);
    //DECLARE_NETWORKCLASS();
    DECLARE_DATADESC();

    CMomWeaponspawner();

    void Spawn();
    void Precache();
    void Touch(CBaseEntity *pOther);
    bool KeyValue(const char *szKeyName, const char *szValue);

    char m_szWeaponName[MAX_WEAPON_STRING];

    //CNetworkString(m_szWeaponName, MAX_WEAPON_STRING);
};