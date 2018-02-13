//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef CS_WEAPON_PARSE_H
#define CS_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"
#include "weapon_parse.h"

//--------------------------------------------------------------------------------------------------------
enum CSWeaponID
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

    WEAPON_MAX, // number of weapons weapon index
};

//--------------------------------------------------------------------------------------------------------
enum CSWeaponID;


//--------------------------------------------------------------------------------------------------------
class CCSWeaponInfo : public FileWeaponInfo_t
{
  public:
    DECLARE_CLASS_GAMEROOT(CCSWeaponInfo, FileWeaponInfo_t);

    CCSWeaponInfo();

    void Parse(KeyValues *pKeyValuesData, const char *szWeaponName) OVERRIDE;

    int m_iCrosshairMinDistance;
    int m_iCrosshairDeltaDistance;

    char m_szAddonModel[MAX_WEAPON_STRING]; // If this is set, it is used as the addon model. Otherwise, szWorldModel is
                                            // used.
    char m_szDroppedModel[MAX_WEAPON_STRING];  // Alternate dropped model, if different from the szWorldModel the player
                                               // holds
    char m_szSilencerModel[MAX_WEAPON_STRING]; // Alternate model with silencer attached

    int m_iMuzzleFlashStyle;
    float m_flMuzzleScale;

    // Parameters for FX_FireBullets:
    int m_iPenetration;
    int m_iDamage;
    float m_flRange;
    float m_flRangeModifier;
    int m_iBullets;
};

#endif // CS_WEAPON_PARSE_H
