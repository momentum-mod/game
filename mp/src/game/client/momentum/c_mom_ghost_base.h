#pragma once
#include "run/mom_run_entity.h"

class C_MomentumGhostBaseEntity : public C_BaseAnimating, public CMomRunEntity
{
    DECLARE_CLASS(C_MomentumGhostBaseEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()
public:

    C_MomentumGhostBaseEntity();

    bool IsValidIDTarget(void) OVERRIDE{ return true; }

    virtual bool IsReplayGhost() const { return false; }
    virtual bool IsOnlineGhost() const { return false; }

    CNetworkString(m_szGhostName, MAX_PLAYER_NAME_LENGTH);
    CNetworkVar(int, m_nGhostButtons);
    CNetworkVar(int, m_iDisabledButtons);
    CNetworkVar(bool, m_bBhopDisabled);

    // MomRunEntity Stuff
    RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_GHOST; }
    CNetworkVarEmbedded(CMomRunEntityData, m_Data);
    virtual CMomRunEntityData *GetRunEntData() override { return &m_Data; }

    CInterpolatedVar<Vector> m_iv_vecViewOffset;

protected:
    bool ShouldInterpolate() OVERRIDE { return true; }

};
