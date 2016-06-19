#ifndef SERVER_EVENTS_H
#define SERVER_EVENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "movevars_shared.h"
#include "mapzones.h"
#include "Timer.h"
#include "mapzones_edit.h"

namespace Momentum {

void OnServerDLLInit();
void GameInit();
} // namespace Momentum

class CMOMServerEvents : CAutoGameSystemPerFrame
{
public:
    CMOMServerEvents(const char *pName) : CAutoGameSystemPerFrame(pName), zones(nullptr)
    {
    }

    void LevelInitPostEntity() override;
    void LevelShutdownPreEntity() override;
    void FrameUpdatePreEntityThink() override;

private:
    CMapzoneData* zones;
};

#endif // SERVER_EVENTS_H