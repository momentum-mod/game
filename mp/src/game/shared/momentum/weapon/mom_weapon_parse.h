#pragma once

#include "weapon_parse.h"

//--------------------------------------------------------------------------------------------------------
class CWeaponInfo : public FileWeaponInfo_t
{
  public:
    DECLARE_CLASS_GAMEROOT(CWeaponInfo, FileWeaponInfo_t);

    CWeaponInfo();

    void Parse(KeyValues *pKeyValuesData, const char *szWeaponName) OVERRIDE;

    int m_iCrosshairMinDistance;
    int m_iCrosshairDeltaDistance;

    // Explosion effect
    char m_szExplosionSound[128];
    char m_szExplosionEffect[128];
    char m_szExplosionPlayerEffect[128];
    char m_szExplosionWaterEffect[128];
};