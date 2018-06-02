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

    STEAM_CALLBACK(CMOMServerEvents, OnGameOverlay, GameOverlayActivated_t);

    void MountAdditionalContent();

private:
    CMapzoneData* zones;
};


#endif // SERVER_EVENTS_H
