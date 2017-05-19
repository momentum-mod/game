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
    ghostNetFrame_t GetCurrentNetFrame() { return m_currentFrame; }

    void SetGhostApperence(ghostAppearance_t app);
    ghostAppearance_t GetAppearance() { return m_currentAppearence; }

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

    ghostAppearance_t m_currentAppearence;
    bool hasSpawned;
    int m_ghostButtons;
};
