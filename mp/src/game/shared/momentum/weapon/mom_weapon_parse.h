#pragma once

#include "weapon_parse.h"

enum CWeaponID
{
    WEAPON_NONE = 0,

    WEAPON_PISTOL,
    WEAPON_RIFLE,
    WEAPON_SHOTGUN,
    WEAPON_SMG,
    WEAPON_SNIPER,
    WEAPON_LMG,
    WEAPON_GRENADE,
    WEAPON_KNIFE,
    WEAPON_PAINTGUN,
    WEAPON_ROCKETLAUNCHER,

    WEAPON_MAX, // number of weapons weapon index
};


//--------------------------------------------------------------------------------------------------------
class CWeaponInfo : public FileWeaponInfo_t
{
  public:
    DECLARE_CLASS_GAMEROOT(CWeaponInfo, FileWeaponInfo_t);

    CWeaponInfo();

    void Parse(KeyValues *pKeyValuesData, const char *szWeaponName) OVERRIDE;

    int m_iCrosshairMinDistance;
    int m_iCrosshairDeltaDistance;

    char m_szAddonModel[MAX_WEAPON_STRING]; // If this is set, it is used as the addon model. Otherwise, szWorldModel is
                                            // used.
    char m_szDroppedModel[MAX_WEAPON_STRING];  // Alternate dropped model, if different from the szWorldModel the player
                                               // holds
    char m_szSilencerModel[MAX_WEAPON_STRING]; // Alternate model with silencer attached

    // Parameters for FX_FireBullets:
    int m_iPenetration;
    int m_iDamage;
    float m_flRange;
    float m_flRangeModifier;
    int m_iBullets;

    // Explosion effect
    char m_szExplosionSound[128];
    char m_szExplosionEffect[128];
    char m_szExplosionPlayerEffect[128];
    char m_szExplosionWaterEffect[128];
};