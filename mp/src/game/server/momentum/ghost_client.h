#pragma once

#include "cbase.h"
#include "zed_net.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"

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

    bool runGhostClient();
    bool exitGhostClient();
    static unsigned sendAndRecieveData(void *params);
    bool isGhostClientConnected() { return m_ghostClientConnected && (socket.ready == 0); }
private:
    static ghostNetFrame prevFrame;
    static zed_net_socket_t socket;
    static zed_net_address_t address;
    static bool m_ghostClientConnected, m_bRanThread;
    const char* host = "127.0.0.1";
    unsigned short port = 9000;
    static char data[256];

    static CThreadMutex m_mtxGhostPlayers, m_mtxpPlayer, m_mtxbClientConnect;
    static CUtlVector<ghostNetFrame> ghostPlayers;
    static CMomentumPlayer *m_pPlayer;
    static uint64 m_SteamID;
};
