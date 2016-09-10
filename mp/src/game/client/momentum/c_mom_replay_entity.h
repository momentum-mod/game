#pragma once

#include "cbase.h"
#include "mom_entity_run_data.h"
#include "util/run_stats.h"

class C_MomentumReplayGhostEntity : public C_BaseAnimating
{
	DECLARE_CLASS(C_MomentumReplayGhostEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION()

public:
    C_MomentumReplayGhostEntity();

    CMOMRunEntityData m_RunData;
    CMomRunStats m_RunStats;

    float m_flTickRate;

    int m_nReplayButtons;
    //These are stored here because run stats already has the ones obtained from the run
    int m_iTotalStrafes;
    int m_iTotalJumps;

    char m_pszPlayerName[MAX_PLAYER_NAME_LENGTH];
    bool ShouldInterpolate() override
    {
        return true;
    }

};