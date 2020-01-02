#include "cbase.h"
#include "mom_weapon_parse.h"
#include "weapon_base.h"

#include "tier0/memdbgon.h"

//--------------------------------------------------------------------------------------------------------------
static const char *WeaponNames[WEAPON_MAX] = {
    "weapon_none",         "weapon_momentum_pistol",  "weapon_momentum_rifle", "weapon_momentum_shotgun",
    "weapon_momentum_smg", "weapon_momentum_sniper",  "weapon_momentum_lmg",   "weapon_momentum_grenade",
    "weapon_knife",        "weapon_momentum_paintgun","weapon_momentum_rocketlauncher",
    "weapon_momentum_stickylauncher"};

//--------------------------------------------------------------------------------------------------------------
CWeaponInfo *GetWeaponInfo(CWeaponID weaponID)
{
    if (weaponID == WEAPON_NONE)
        return nullptr;

    const char *weaponName = WeaponNames[weaponID];
    WEAPON_FILE_INFO_HANDLE hWpnInfo = LookupWeaponInfoSlot(weaponName);
    if (hWpnInfo == GetInvalidWeaponInfoHandle())
    {
        return nullptr;
    }

    return static_cast<CWeaponInfo *>(GetFileWeaponInfoFromHandle(hWpnInfo));
}

CWeaponInfo::CWeaponInfo()
    : m_iCrosshairMinDistance(4), m_iCrosshairDeltaDistance(3)
{
    m_szExplosionEffect[0] = '\0';
    m_szExplosionPlayerEffect[0] = '\0';
    m_szExplosionSound[0] = '\0';
    m_szExplosionWaterEffect[0] = '\0';
}

FileWeaponInfo_t *CreateWeaponInfo() { return new CWeaponInfo(); }

void CWeaponInfo::Parse(KeyValues *pKeyValuesData, const char *szWeaponName)
{
    BaseClass::Parse(pKeyValuesData, szWeaponName);

    m_iCrosshairMinDistance = pKeyValuesData->GetInt("CrosshairMinDistance", 4);
    m_iCrosshairDeltaDistance = pKeyValuesData->GetInt("CrosshairDeltaDistance", 3);

    // Explosion effects
    const char *pszSound = pKeyValuesData->GetString("ExplosionSound", nullptr);
    if (pszSound)
    {
        Q_strncpy(m_szExplosionSound, pszSound, sizeof(m_szExplosionSound));
    }

    const char *pszEffect = pKeyValuesData->GetString("ExplosionEffect", nullptr);
    if (pszEffect)
    {
        Q_strncpy(m_szExplosionEffect, pszEffect, sizeof(m_szExplosionEffect));
    }

    pszEffect = pKeyValuesData->GetString("ExplosionPlayerEffect", nullptr);
    if (pszEffect)
    {
        Q_strncpy(m_szExplosionPlayerEffect, pszEffect, sizeof(m_szExplosionPlayerEffect));
    }

    pszEffect = pKeyValuesData->GetString("ExplosionWaterEffect", nullptr);
    if (pszEffect)
    {
        Q_strncpy(m_szExplosionWaterEffect, pszEffect, sizeof(m_szExplosionWaterEffect));
    }

#ifndef CLIENT_DLL
    // Enforce consistency for the weapon here, since that way we don't need to save off the model bounds
    // for all time.
    // engine->ForceExactFile( UTIL_VarArgs("scripts/%s.ctx", szWeaponName ) );

    // Model bounds are rounded to the nearest integer, then extended by 1
    engine->ForceModelBounds(szWorldModel, Vector(-15, -12, -18), Vector(44, 16, 19));
#endif
}
