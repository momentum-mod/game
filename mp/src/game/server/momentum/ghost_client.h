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

    bool initGhostClient();
    bool exitGhostClient();
    static bool connectToGhostServer(const char* host, unsigned short port);
    static unsigned sendAndRecieveData(void *params);
    bool isGhostClientConnected() { return m_ghostClientConnected && (m_socket.ready == 0); }
private:
    static ghostNetFrame prevFrame;
    static zed_net_socket_t m_socket;
    static zed_net_address_t m_address;
    static bool m_ghostClientConnected, m_bRanThread;
    static const char* m_host;
    static unsigned short m_port;
    static char data[256];

    static CThreadMutex m_mtxGhostPlayers, m_mtxpPlayer;
    static CUtlVector<ghostNetFrame> ghostPlayers;
    static CMomentumPlayer *m_pPlayer;
    static uint64 m_SteamID;
};
