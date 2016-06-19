#pragma once

#include "mom_shareddefs.h"
#include "util/run_stats.h"

class C_Momentum_EventListener : public CGameEventListener
{
public:
    C_Momentum_EventListener() :
        m_bTimeDidSave(false),
        m_bTimeDidUpload(false), m_bMapIsLinear(false), m_iMapZoneCount(0)
    { }

    void Init();

    void FireGameEvent(IGameEvent* pEvent) override;

    bool m_bTimeDidSave, m_bTimeDidUpload;
    bool m_bMapIsLinear;

    int m_iMapZoneCount;

    char m_szRunUploadStatus[512];//MOM_TODO: determine best (max) size for this
};

extern C_Momentum_EventListener *g_MOMEventListener;