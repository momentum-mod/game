#pragma once

#include "cbase.h"
#include "mom_ghost_base.h"
#include "mom_ghostdefs.h"

class CMomentumOnlineGhostEntity : public CMomentumGhostBaseEntity
{
    DECLARE_CLASS(CMomentumOnlineGhostEntity, CMomentumGhostBaseEntity)
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

public:
    CMomentumOnlineGhostEntity();
    ~CMomentumOnlineGhostEntity();

    void SetCurrentNetFrame(ghostNetFrame_t newFrame) { m_currentFrame = newFrame; }
    ghostNetFrame_t GetCurrentNetFrame() const { return m_currentFrame; }
    void SetGhostSteamID(uint64_t steamID) { m_u64GhostSteamID = steamID; }
    uint64_t GetGhostSteamID() const { return m_u64GhostSteamID; }

    bool IsOnlineGhost() const OVERRIDE{ return true; }

    bool HasSpawned() const { return hasSpawned; }
    void Spawn() OVERRIDE;
    void HandleGhost() OVERRIDE;
    void HandleGhostFirstPerson() OVERRIDE;
    void UpdateStats(const Vector &ghostVel) OVERRIDE; // for hud display..

    CNetworkString(m_pszGhostName, MAX_PLAYER_NAME_LENGTH);

protected:
    void Think(void) OVERRIDE;
    void Precache(void) OVERRIDE;
private:
    ghostNetFrame_t m_currentFrame;
    ghostNetFrame_t m_previousFrame;

    uint64_t m_u64GhostSteamID;
    ghostAppearance_t m_currentAppearance;
    bool hasSpawned;
    int m_ghostButtons;
};
