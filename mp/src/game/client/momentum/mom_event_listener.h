#pragma once

#include "mom_shareddefs.h"
#include "util/run_stats.h"

class C_Momentum_EventListener : public CGameEventListener
{
public:
    C_Momentum_EventListener() :
        m_bTimeDidSave(false),
        m_bTimeDidUpload(false), m_bMapIsLinear(false), m_iMapZoneCount(0),
        m_flLastRunTime(0)
    { }

    ~C_Momentum_EventListener()
    {
        m_EntRunStats.PurgeAndDeleteElements();
    }

    void Init();

    void FireGameEvent(IGameEvent* pEvent) override;

    RunStats_t *GetRunStats(int entIndex)
    {
        unsigned short index;
        if ((index = m_EntRunStats.Find(entIndex)) != m_EntRunStats.InvalidIndex())
        {
            return m_EntRunStats.Element(index);
        }
        RunStats_t *temp = new RunStats_t(m_iMapZoneCount);
        m_EntRunStats.InsertOrReplace(entIndex, temp);
        return GetRunStats(entIndex);
    }

    bool m_bTimeDidSave, m_bTimeDidUpload;
    bool m_bMapIsLinear;

    int m_iMapZoneCount;

    CUtlMap<int, RunStats_t*> m_EntRunStats;
    float m_flLastRunTime; //this is the "adjusted" precision-fixed time value that was calculated on the server DLL

    char m_szRunUploadStatus[512];//MOM_TODO: determine best (max) size for this
};

extern C_Momentum_EventListener *g_MOMEventListener;