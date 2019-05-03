#pragma once

class CMomServerEvents : CAutoGameSystem
{
public:
    CMomServerEvents();

    void PostInit() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;

    STEAM_CALLBACK(CMomServerEvents, OnGameOverlay, GameOverlayActivated_t);

    void MountAdditionalContent();
};