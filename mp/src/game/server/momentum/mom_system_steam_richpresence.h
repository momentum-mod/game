#pragma once

#include "igamesystem.h"

class CSteamRichPresenceSystem : public CAutoGameSystem
{
public:
    CSteamRichPresenceSystem();

    void Update(bool bLevelShutdown = false);

protected:
    void PostInit() override { Update(); }
    void LevelInitPostEntity() override { Update(); }
    void LevelShutdownPostEntity() override { Update(true); }
};

extern CSteamRichPresenceSystem *g_pSteamRichPresence;