#pragma once

#include "cbase.h"
#include "mom_ghost_base.h"
#include "mom_ghostdefs.h"
#include "utlstack.h"
#include "utlqueue.h"

struct ReceivedFrame
{
    float recvTime;
    ghostNetFrame_t frame;

    ReceivedFrame(float recvTime, ghostNetFrame_t recvFrame)
    {
        this->recvTime = recvTime;
        frame = recvFrame;
    }
};

class CMomentumOnlineGhostEntity : public CMomentumGhostBaseEntity
{
    DECLARE_CLASS(CMomentumOnlineGhostEntity, CMomentumGhostBaseEntity)
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

public:
    CMomentumOnlineGhostEntity();
    ~CMomentumOnlineGhostEntity();

    void SetCurrentNetFrame(ghostNetFrame_t newFrame);
    //ghostNetFrame_t GetCurrentNetFrame() const { return m_currentFrame; }
    void SetGhostSteamID(CSteamID steamID) { m_GhostSteamID = steamID; }
    CSteamID GetGhostSteamID() const { return m_GhostSteamID; }

    bool IsOnlineGhost() const OVERRIDE { return true; }

    void Spawn() OVERRIDE;
    void HandleGhost() OVERRIDE;
    void HandleGhostFirstPerson() OVERRIDE;
    void UpdateStats(const Vector &ghostVel) OVERRIDE; // for hud display..

    CNetworkString(m_pszGhostName, MAX_PLAYER_NAME_LENGTH);

protected:
    void Think(void) OVERRIDE;
    void Precache(void) OVERRIDE;
private:
    CUtlQueue<ReceivedFrame*> m_vecFrames;
    ReceivedFrame* m_pCurrentFrame;
    ReceivedFrame* m_pNextFrame;

    CSteamID m_GhostSteamID;
    ghostAppearance_t m_currentAppearance;
    int m_ghostButtons;
};
