#pragma once

#include "cbase.h"
#include "mom_entity_run_data.h"

class C_MomentumReplayGhostEntity : public C_BaseAnimating
{
    DECLARE_CLASS(C_MomentumReplayGhostEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();

public:
    C_MomentumReplayGhostEntity();

    CMOMRunEntityData m_RunData;

    float m_flRunTime;

    int m_nReplayButtons;
    //These are stored here because run stats already has the ones obtained from the run
    int m_iTotalStrafes;
    int m_iTotalJumps;

    bool ShouldInterpolate() override
    {
        return true;
    }

};