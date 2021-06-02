#include "cbase.h"
#include "mom_weaponspawner.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"

LINK_ENTITY_TO_CLASS(mom_weaponspawner, CMomWeaponspawner);

BEGIN_DATADESC(CMomWeaponspawner)
	DEFINE_AUTO_ARRAY_KEYFIELD(m_szWeaponName, FIELD_CHARACTER, "WeaponName"),
END_DATADESC()

CMomWeaponspawner::CMomWeaponspawner()
{
	m_szWeaponName[0] = 0;
}

void CMomWeaponspawner::Spawn()
{
    SetSolid(SOLID_BBOX);
    SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
    UTIL_SetSize(this, Vector(-30, -30, -30), Vector(30, 30, 30));
}

void CMomWeaponspawner::Think()
{
    QAngle angle;
    angle = GetAbsAngles();
    angle.x++;
    Msg("%f %f %f\n", angle.x, angle.y, angle.z);
    
    SetAbsAngles(angle);
    SetNextThink(gpGlobals->curtime + 1);
}

void CMomWeaponspawner::Precache(const char *weaponModel)
{
    PrecacheModel(weaponModel);
    BaseClass::Precache();
}

void CMomWeaponspawner::Touch(CBaseEntity *pOther)
{
    const auto pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer && !pPlayer->IsObserver())
    {
        WeaponID_t foundID = WEAPON_NONE;
        for (int weaponID = WEAPON_FIRST; weaponID < WEAPON_MAX; weaponID++)
        {
            if (!Q_strnicmp(g_szWeaponNames[weaponID], m_szWeaponName, 64))
            {
                foundID = (WeaponID_t)weaponID;
                break;
            }
        }

        if (foundID == WEAPON_NONE)
        {
            Warning("Could not give weapon with name %s, weapon not found!\n", m_szWeaponName);
            return;
        }

        if (!g_pGameModeSystem->GetGameMode()->WeaponIsAllowed(foundID))
        {
            Warning("The weapon %s is not allowed in this gamemode!\n", m_szWeaponName);
            return;
        }

        if (gpGlobals->eLoadType == MapLoad_Background)
        {
            Warning("Cannot give weapons in this map!");
            return;
        }

        if (!pPlayer->GetWeapon(foundID))
        {
            pPlayer->GiveWeapon(foundID);
        }
    }
    BaseClass::Touch(pOther);
}


bool CMomWeaponspawner::KeyValue(const char* szKeyName, const char* szValue)
{
    if (FStrEq(szKeyName, "WeaponName"))
    {
        Q_strncpy(m_szWeaponName, szValue, MAX_WEAPON_STRING);
        return true;
    }
    else if (FStrEq(szKeyName, "model"))
    {
        Precache(szValue);
        SetModel(szValue);
        return true;
    }

    return BaseClass::KeyValue(szKeyName, szValue);
}