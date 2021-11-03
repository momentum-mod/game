#include "cbase.h"
#include "mom_powerup_haste.h"
#include "mom_player_shared.h"
#include "mom_system_gamemode.h"

LINK_ENTITY_TO_CLASS(mom_powerup_haste, CMomPowerupHaste);

BEGIN_DATADESC(CMomPowerupHaste)
	DEFINE_KEYFIELD(m_flHasteTime, FIELD_FLOAT, "HasteTime"),
	DEFINE_KEYFIELD(m_flDisappearTime, FIELD_FLOAT, "DisappearTime")
END_DATADESC()

CMomPowerupHaste::CMomPowerupHaste() {
}

void CMomPowerupHaste::Spawn()
{
    Precache();

    m_bVisible = true;
    SetModel(HASTE_MODEL);
    SetSolid(SOLID_BBOX);
    SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
    UTIL_SetSize(this, Vector(-30, -30, 0), Vector(30, 30, 60));
}

void CMomPowerupHaste::Precache()
{
    PrecacheModel(HASTE_MODEL);
    BaseClass::Precache();
}

void CMomPowerupHaste::Think()
{
    SetModel(HASTE_MODEL);
    m_bVisible = true;
    BaseClass::Think();
}

void CMomPowerupHaste::Touch(CBaseEntity *pOther)
{
    const auto pPlayer = ToCMOMPlayer(pOther);

    if (!m_bVisible || !pPlayer)
    {
        BaseClass::Touch(pOther);
        return;
    }
    if (m_flHasteTime < 0)
    {
        pPlayer->m_flRemainingHaste = -1;
    }
    else
    {
        pPlayer->m_flRemainingHaste = gpGlobals->curtime + m_flHasteTime;
    }

    if (m_flDisappearTime > 0)
    {
        SetNextThink(gpGlobals->curtime + m_flDisappearTime);
        SetModel("");
        m_bVisible = false;
    }
}