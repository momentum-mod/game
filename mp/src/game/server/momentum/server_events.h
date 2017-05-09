#ifndef SERVER_EVENTS_H
#define SERVER_EVENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include <movevars_shared.h>
#include "mapzones.h"
#include "mom_timer.h"
#include "mapzones_edit.h"
#include "zed_net.h"


namespace Momentum {
void GameInit();
} // namespace Momentum

class CMOMServerEvents : CAutoGameSystemPerFrame
{
public:
    CMOMServerEvents(const char *pName) : CAutoGameSystemPerFrame(pName), zones(nullptr)
    {
    }
    void PostInit() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;

    void MountAdditionalContent();

    bool runGhostClient();
    bool exitGhostClient();
    static unsigned sendAndRecieveData(void *params);
    bool isGhostClientConnected() { return m_ghostClientConnected; }
private:
    CMapzoneData* zones;
    static zed_net_socket_t socket;
    static zed_net_address_t address;
    bool m_ghostClientConnected = false;
    const char* host = DEFAULT_HOST;
    unsigned short port = DEFAULT_PORT;
    static char data[256];

    static CUtlVector<ghostNetFrame> ghostPlayers;
};


#endif // SERVER_EVENTS_H
