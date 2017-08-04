#pragma once

#include "cbase.h"
#include <run/mom_entity_run_data.h>
#include <run/run_stats.h>
#include <../interpolatedvar.h>
#include "mom_modulecomms.h"

class C_MomentumReplayGhostEntity : public C_BaseAnimating
{
    DECLARE_CLASS(C_MomentumReplayGhostEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()

  public:
    C_MomentumReplayGhostEntity();

    void ClientThink(void) OVERRIDE;

    void Spawn() OVERRIDE;
    StdReplayDataFromServer m_SrvData;
    CMomRunStats m_RunStats;

    float m_flTickRate;
    
    // These are stored here because run stats already has the ones obtained from the run

    int m_iTotalTimeTicks; // The total tick count of the playback

    char m_pszPlayerName[MAX_PLAYER_NAME_LENGTH];
    bool ShouldInterpolate() OVERRIDE { return true; }
    CInterpolatedVar<Vector> m_iv_vecViewOffset;
};