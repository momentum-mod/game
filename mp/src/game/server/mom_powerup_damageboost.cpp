#include "cbase.h"
#include "mom_player_shared.h"
#include "mom_powerup_damageboost.h"
#include "mom_system_gamemode.h"

LINK_ENTITY_TO_CLASS(mom_powerup_damageboost, CMomPowerupDamageBoost);

BEGIN_DATADESC(CMomPowerupDamageBoost)
DEFINE_KEYFIELD(m_flDamageTime, FIELD_FLOAT, "DamageTime"),
    DEFINE_KEYFIELD(m_flDisappearTime, FIELD_FLOAT, "DisappearTime")
END_DATADESC()

CMomPowerupDamageBoost::CMomPowerupDamageBoost()
{
}

void CMomPowerupDamageBoost::Spawn()
{
    Precache();

    m_bVisible = true;
    SetModel(DAMAGEBOOST_MODEL);
    SetSolid(SOLID_BBOX);
    SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
    UTIL_SetSize(this, Vector(-30, -30, 0), Vector(30, 30, 60));
}

void CMomPowerupDamageBoost::Precache()
{
    PrecacheModel(DAMAGEBOOST_MODEL);
    BaseClass::Precache();
}

void CMomPowerupDamageBoost::Think()
{
    SetModel(DAMAGEBOOST_MODEL);
    m_bVisible = true;
    BaseClass::Think();
}

void CMomPowerupDamageBoost::Touch(CBaseEntity *pOther)
{
    const auto pPlayer = ToCMOMPlayer(pOther);

    if (!m_bVisible)
    {
        BaseClass::Touch(pOther);
        return;
    }
    if (m_flDamageTime < 0)
    {
        pPlayer->m_flRemainingDamageBoost = -1;
    }
    else
    {
        pPlayer->m_flRemainingDamageBoost = gpGlobals->curtime + m_flDamageTime;
    }

    if (m_flDisappearTime > 0)
    {
        SetNextThink(gpGlobals->curtime + m_flDisappearTime);
        SetModel("");
        m_bVisible = false;
    }
}