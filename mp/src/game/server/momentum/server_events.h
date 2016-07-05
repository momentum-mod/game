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
void GameInit();
} // namespace Momentum

class CMOMServerEvents : CAutoGameSystemPerFrame
{
public:
    CMOMServerEvents(const char *pName) : CAutoGameSystemPerFrame(pName), zones(nullptr)
    {
    }
    void PostInit() override;
    void LevelInitPostEntity() override;
    void LevelShutdownPreEntity() override;
    void FrameUpdatePreEntityThink() override;

    void MountAdditionalContent();

private:
    CMapzoneData* zones;
};

#endif // SERVER_EVENTS_H