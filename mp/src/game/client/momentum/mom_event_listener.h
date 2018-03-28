#pragma once

#include "mom_shareddefs.h"
#include "run/run_stats.h"

class C_Momentum_EventListener : public CGameEventListener
{
public:
    C_Momentum_EventListener() :
        m_bMapIsLinear(false), m_iMapZoneCount(0)
    { }

    void Init();

    void FireGameEvent(IGameEvent* pEvent) OVERRIDE;

    bool m_bMapIsLinear;

    int m_iMapZoneCount;
};

extern C_Momentum_EventListener *g_MOMEventListener;