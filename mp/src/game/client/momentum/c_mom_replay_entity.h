#pragma once

#include "mom_modulecomms.h"
#include "c_mom_ghost_base.h"

class C_MomentumReplayGhostEntity : public C_MomentumGhostBaseEntity
{
    DECLARE_CLASS(C_MomentumReplayGhostEntity, C_MomentumGhostBaseEntity);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()

  public:
    C_MomentumReplayGhostEntity();

    void ClientThink(void) OVERRIDE;

    void Spawn() OVERRIDE;
    bool IsReplayGhost() const OVERRIDE { return true; }

    StdReplayDataFromServer m_SrvData;
    CMomRunStats m_RunStats;

};