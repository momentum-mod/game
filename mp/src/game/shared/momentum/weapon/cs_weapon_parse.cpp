//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "cs_weapon_parse.h"
#include "icvar.h"
#include "weapon_csbase.h"
#include <KeyValues.h>

//--------------------------------------------------------------------------------------------------------
struct WeaponTypeInfo
{
    CSWeaponType type;
    const char *name;
};

//--------------------------------------------------------------------------------------------------------
WeaponTypeInfo s_weaponTypeInfo[] = {
    {WEAPONTYPE_KNIFE, "Knife"},
    {WEAPONTYPE_PISTOL, "Pistol"},
    {WEAPONTYPE_SUBMACHINEGUN, "Submachine Gun"}, // First match is printable
    {WEAPONTYPE_SUBMACHINEGUN, "submachinegun"},
    {WEAPONTYPE_SUBMACHINEGUN, "smg"},
    {WEAPONTYPE_RIFLE, "Rifle"},
    {WEAPONTYPE_SHOTGUN, "Shotgun"},
    {WEAPONTYPE_SNIPER_RIFLE, "Sniper"},
    {WEAPONTYPE_MACHINEGUN, "Machine Gun"}, // First match is printable
    {WEAPONTYPE_MACHINEGUN, "machinegun"},
    {WEAPONTYPE_MACHINEGUN, "mg"},
    {WEAPONTYPE_GRENADE, "Grenade"},
    {WEAPONTYPE_UNKNOWN, nullptr},
};

//--------------------------------------------------------------------------------------------------------------
static const char *WeaponNames[WEAPON_MAX] = {
    "weapon_none",         "weapon_momentum_pistol", "weapon_momentum_rifle", "weapon_momentum_shotgun",
    "weapon_momentum_smg", "weapon_momentum_sniper", "weapon_momentum_lmg",   "weapon_momentum_grenade",
    "weapon_knife"};

//--------------------------------------------------------------------------------------------------------------
CCSWeaponInfo *GetWeaponInfo(CSWeaponID weaponID)
{
    if (weaponID == WEAPON_NONE)
        return nullptr;

    const char *weaponName = WeaponNames[weaponID];
    WEAPON_FILE_INFO_HANDLE hWpnInfo = LookupWeaponInfoSlot(weaponName);
    if (hWpnInfo == GetInvalidWeaponInfoHandle())
    {
        return nullptr;
    }

    CCSWeaponInfo *pWeaponInfo = dynamic_cast<CCSWeaponInfo *>(GetFileWeaponInfoFromHandle(hWpnInfo));

    return pWeaponInfo;
}

//--------------------------------------------------------------------------------------------------------
const char *WeaponClassAsString(CSWeaponType weaponType)
{
    WeaponTypeInfo *info = s_weaponTypeInfo;
    while (info->name != nullptr)
    {
        if (info->type == weaponType)
        {
            return info->name;
        }
        ++info;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromString(const char *weaponType)
{
    WeaponTypeInfo *info = s_weaponTypeInfo;
    while (info->name != nullptr)
    {
        if (!Q_stricmp(info->name, weaponType))
        {
            return info->type;
        }
        ++info;
    }

    return WEAPONTYPE_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromWeaponID(CSWeaponID weaponID)
{
    const char *weaponStr = WeaponIDToAlias(weaponID);
    const char *translatedAlias = GetTranslatedWeaponAlias(weaponStr);

    char wpnName[128];
    Q_snprintf(wpnName, sizeof(wpnName), "weapon_%s", translatedAlias);
    WEAPON_FILE_INFO_HANDLE hWpnInfo = LookupWeaponInfoSlot(wpnName);
    if (hWpnInfo != GetInvalidWeaponInfoHandle())
    {
        CCSWeaponInfo *pWeaponInfo = dynamic_cast<CCSWeaponInfo *>(GetFileWeaponInfoFromHandle(hWpnInfo));
        if (pWeaponInfo)
        {
            return pWeaponInfo->m_WeaponType;
        }
    }

    return WEAPONTYPE_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------------
void ParseVector(KeyValues *keyValues, const char *keyName, Vector &vec)
{
    vec.x = vec.y = vec.z = 0.0f;

    if (!keyValues || !keyName)
        return;

    const char *vecString = keyValues->GetString(keyName, "0 0 0");
    if (vecString && *vecString)
    {
        float x, y, z;
        if (3 == sscanf(vecString, "%f %f %f", &x, &y, &z))
        {
            vec.x = x;
            vec.y = y;
            vec.z = z;
        }
    }
}

CCSWeaponInfo::CCSWeaponInfo()
    : m_WeaponType(WEAPONTYPE_UNKNOWN), m_iCrosshairMinDistance(4), m_iCrosshairDeltaDistance(3),
      m_iMuzzleFlashStyle(CS_MUZZLEFLASH_NORM), m_flMuzzleScale(1.0f), m_iPenetration(1), m_iDamage(42),
      m_flRange(8192.0f), m_flRangeModifier(0.98f), m_iBullets(1)
{
    m_szAddonModel[0] = 0;
}

FileWeaponInfo_t *CreateWeaponInfo() { return new CCSWeaponInfo(); }

void CCSWeaponInfo::Parse(KeyValues *pKeyValuesData, const char *szWeaponName)
{
    BaseClass::Parse(pKeyValuesData, szWeaponName);

    m_iCrosshairMinDistance = pKeyValuesData->GetInt("CrosshairMinDistance", 4);
    m_iCrosshairDeltaDistance = pKeyValuesData->GetInt("CrosshairDeltaDistance", 3);
    m_flMuzzleScale = pKeyValuesData->GetFloat("MuzzleFlashScale", 1);

    const char *pMuzzleFlashStyle = pKeyValuesData->GetString("MuzzleFlashStyle", "CS_MUZZLEFLASH_NORM");

    if (pMuzzleFlashStyle)
    {
        if (Q_stricmp(pMuzzleFlashStyle, "CS_MUZZLEFLASH_X") == 0)
        {
            m_iMuzzleFlashStyle = CS_MUZZLEFLASH_X;
        }
        else if (Q_stricmp(pMuzzleFlashStyle, "CS_MUZZLEFLASH_NONE") == 0)
        {
            m_iMuzzleFlashStyle = CS_MUZZLEFLASH_NONE;
        }
        else
        {
            m_iMuzzleFlashStyle = CS_MUZZLEFLASH_NORM;
        }
    }

    m_iPenetration = pKeyValuesData->GetInt("Penetration", 1);
    m_iDamage = pKeyValuesData->GetInt("Damage", 42); // Douglas Adams 1952 - 2001
    m_flRange = pKeyValuesData->GetFloat("Range", 8192.0f);
    m_flRangeModifier = pKeyValuesData->GetFloat("RangeModifier", 0.98f);
    m_iBullets = pKeyValuesData->GetInt("Bullets", 1);


    const char *pTypeString = pKeyValuesData->GetString("WeaponType", nullptr);

    m_WeaponType = WEAPONTYPE_UNKNOWN;
    if (!pTypeString)
    {
        // Assert(false);
    }
    else if (Q_stricmp(pTypeString, "Knife") == 0)
    {
        m_WeaponType = WEAPONTYPE_KNIFE;
    }
    else if (Q_stricmp(pTypeString, "Pistol") == 0)
    {
        m_WeaponType = WEAPONTYPE_PISTOL;
    }
    else if (Q_stricmp(pTypeString, "Rifle") == 0)
    {
        m_WeaponType = WEAPONTYPE_RIFLE;
    }
    else if (Q_stricmp(pTypeString, "Shotgun") == 0)
    {
        m_WeaponType = WEAPONTYPE_SHOTGUN;
    }
    else if (Q_stricmp(pTypeString, "SniperRifle") == 0)
    {
        m_WeaponType = WEAPONTYPE_SNIPER_RIFLE;
    }
    else if (Q_stricmp(pTypeString, "SubMachinegun") == 0)
    {
        m_WeaponType = WEAPONTYPE_SUBMACHINEGUN;
    }
    else if (Q_stricmp(pTypeString, "Machinegun") == 0)
    {
        m_WeaponType = WEAPONTYPE_MACHINEGUN;
    }
    else if (Q_stricmp(pTypeString, "Grenade") == 0)
    {
        m_WeaponType = WEAPONTYPE_GRENADE;
    }
    else
    {
        // Assert(false);
    }

    // Read the addon model.
    Q_strncpy(m_szAddonModel, pKeyValuesData->GetString("AddonModel"), sizeof(m_szAddonModel));

    // Read the dropped model.
    Q_strncpy(m_szDroppedModel, pKeyValuesData->GetString("DroppedModel"), sizeof(m_szDroppedModel));

    // Read the silencer model.
    Q_strncpy(m_szSilencerModel, pKeyValuesData->GetString("SilencerModel"), sizeof(m_szSilencerModel));

#ifndef CLIENT_DLL
    // Enforce consistency for the weapon here, since that way we don't need to save off the model bounds
    // for all time.
    // engine->ForceExactFile( UTIL_VarArgs("scripts/%s.ctx", szWeaponName ) );

    // Model bounds are rounded to the nearest integer, then extended by 1
    engine->ForceModelBounds(szWorldModel, Vector(-15, -12, -18), Vector(44, 16, 19));
    if (m_szAddonModel[0])
    {
        engine->ForceModelBounds(m_szAddonModel, Vector(-5, -5, -6), Vector(13, 5, 7));
    }
    if (m_szSilencerModel[0])
    {
        engine->ForceModelBounds(m_szSilencerModel, Vector(-15, -12, -18), Vector(44, 16, 19));
    }
#endif // !CLIENT_DLL
}
