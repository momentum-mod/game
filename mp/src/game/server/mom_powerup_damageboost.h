#pragma once

#include "cbase.h"

#define DAMAGEBOOST_MODEL "models/pickups/pickup_powerup_crit.mdl"

class CMomPowerupDamageBoost : public CBaseAnimating
{
  public:
    DECLARE_CLASS(CMomPowerupDamageBoost, CBaseAnimating);
    DECLARE_DATADESC();

    CMomPowerupDamageBoost();

    void Spawn();
    void Precache();
    void Think();
    void Touch(CBaseEntity *pOther);

    float m_flDamageTime;
    float m_flDisappearTime;
    bool m_bVisible;
};
