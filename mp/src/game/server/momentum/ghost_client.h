#pragma once

#include "cbase.h"
#include "zed_net.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"

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
    void LevelShutdownPostEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;

    bool runGhostClient();
    bool exitGhostClient();
    static unsigned sendAndRecieveData(void *params);
    bool isGhostClientConnected() { return m_ghostClientConnected && (socket.ready == 0); }
private:
    static ghostNetFrame prevFrame;
    static zed_net_socket_t socket;
    static zed_net_address_t address;
    bool m_ghostClientConnected = false;
    const char* host = DEFAULT_HOST;
    unsigned short port = DEFAULT_PORT;
    static char data[256];

    static CUtlVector<ghostNetFrame> ghostPlayers;

};
