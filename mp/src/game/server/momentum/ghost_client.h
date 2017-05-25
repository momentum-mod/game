#pragma once

#include "cbase.h"
#include "zed_net.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "mom_online_ghost.h"

struct MyThreadParams_t{}; //empty class so we can force the threaded function to work xd

class CMomentumGhostClient : public CAutoGameSystemPerFrame
{
public:
    CMomentumGhostClient() : CAutoGameSystemPerFrame()
    {
    }
    //void PostInit() OVERRIDE;
    //void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    //void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void FrameUpdatePostEntityThink() OVERRIDE;

    static bool initGhostClient();
    static bool exitGhostClient();
    static bool connectToGhostServer(const char* host, unsigned short port);
    static unsigned sendAndRecieveData(void *params);
    static bool isGhostClientConnected() { return m_ghostClientConnected && (m_socket.ready == 0); }
    static bool SendSignonMessage();
    static ghostNetFrame_t CreateNewNetFrame(CMomentumPlayer *pPlayer);
    static ghostAppearance_t CreateAppearance(CMomentumPlayer* pPlayer) { return pPlayer->m_playerAppearanceProps; }
    static bool SendAppearanceData(ghostAppearance_t apps);
    static bool SendNetFrame(ghostNetFrame_t frame);
private:
    static zed_net_socket_t m_socket;
    static zed_net_address_t m_address;
    static bool m_ghostClientConnected, m_bRanThread;
    static char data[256];

    static CThreadMutex m_mtxGhostPlayers, m_mtxpPlayer;
    static CUtlVector<CMomentumOnlineGhostEntity*> ghostPlayers;
    static CMomentumPlayer *m_pPlayer;
    static uint64 m_SteamID;
    static ghostAppearance_t oldAppearance;
};
