#pragma once

#include "cbase.h"

#define HASTE_MODEL "models/pickups/pickup_powerup_haste.mdl"

class CMomPowerupHaste : public CBaseAnimating
{
  public:
    DECLARE_CLASS(CMomPowerupHaste, CBaseAnimating);
    DECLARE_DATADESC();

    CMomPowerupHaste();

    void Spawn();
    void Precache();
    void Think();
    void Touch(CBaseEntity *pOther);

    float m_flHasteTime;
    float m_flDisappearTime;
    bool m_bVisible;
};
#pragma once
