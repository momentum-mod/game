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
    bool recieveGhostData();
    bool isGhostClientConnected() { return socket.ready == 0; }
private:
    CMapzoneData* zones;
    zed_net_socket_t socket;
    zed_net_address_t address;
    const char* host = DEFAULT_HOST;
    unsigned short port = DEFAULT_PORT;
    bool recievedPacket;
    ThreadHandle_t thread;
};

#endif // SERVER_EVENTS_H
