#pragma once

#include "cbase.h"
#include "mom_player_shared.h"

class CMOMCheckpointSystem : CAutoGameSystem
{
public:
    CMOMCheckpointSystem(const char* pName) : CAutoGameSystem(pName)
    {
        m_pCheckpointsKV = new KeyValues(pName);
    }
    ~CMOMCheckpointSystem()
    {
        if (m_pCheckpointsKV)
            m_pCheckpointsKV->deleteThis();
        m_pCheckpointsKV = nullptr;
    }

    void LevelInitPostEntity() override;
    void LevelShutdownPreEntity() override;

    void LoadMapCheckpoints(CMomentumPlayer *pPlayer) const;

private:

    KeyValues *m_pCheckpointsKV;
};

extern CMOMCheckpointSystem *g_MOMCheckpointSystem;