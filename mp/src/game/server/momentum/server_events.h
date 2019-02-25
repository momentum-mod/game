#ifndef SERVER_EVENTS_H
#define SERVER_EVENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "utlbuffer.h"

class CMapzoneData;

namespace Momentum {
void GameInit();
} // namespace Momentum

class CMOMServerEvents : CAutoGameSystem
{
public:
    CMOMServerEvents(const char* pName);

    void PostInit() OVERRIDE;
    void Shutdown() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;

    STEAM_CALLBACK(CMOMServerEvents, OnGameOverlay, GameOverlayActivated_t);

    void MountAdditionalContent();

private:
    CMapzoneData* zones;
};


#endif // SERVER_EVENTS_H
