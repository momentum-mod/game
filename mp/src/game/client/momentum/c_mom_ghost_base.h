#pragma once

#include "run/mom_run_entity.h"

class C_MomentumGhostBaseEntity : public C_BaseAnimating, public CMomRunEntity
{
    DECLARE_CLASS(C_MomentumGhostBaseEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()
public:

    C_MomentumGhostBaseEntity();

    bool IsValidIDTarget() OVERRIDE{ return true; }
    void PostDataUpdate(DataUpdateType_t updateType) override;

    virtual bool IsReplayGhost() const { return false; }
    virtual bool IsOnlineGhost() const { return false; }

    CNetworkVar(int32, m_AccountID);
    CNetworkString(m_szGhostName, MAX_PLAYER_NAME_LENGTH);
    CNetworkVar(int, m_nGhostButtons);
    CNetworkVar(int, m_iDisabledButtons);
    CNetworkVar(bool, m_bBhopDisabled);
    CNetworkVar(bool, m_bSpectated); // Is this ghost being spectated by the local player?

    // MomRunEntity Stuff
    RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_GHOST; }
    CNetworkVarEmbedded(CMomRunEntityData, m_Data);
    virtual CMomRunEntityData *GetRunEntData() OVERRIDE { return &m_Data; }
    CNetworkVarEmbedded(CMomRunStats, m_RunStats);
    virtual CMomRunStats *GetRunStats() OVERRIDE {return &m_RunStats;}
    virtual int GetEntIndex() OVERRIDE { return m_index; }
    virtual float GetCurrentRunTime() OVERRIDE;
    uint64 GetSteamID() override { return m_SteamID; }

    CInterpolatedVar<Vector> m_iv_vecViewOffset;

protected:
    bool ShouldInterpolate() OVERRIDE { return true; }

private:
    uint64 m_SteamID;
};