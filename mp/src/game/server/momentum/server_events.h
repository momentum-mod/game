#pragma once

class CMomServerEvents : CAutoGameSystem
{
public:
    CMomServerEvents();

    bool Init() OVERRIDE;
    void PostInit() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;

    STEAM_CALLBACK(CMomServerEvents, OnGameOverlay, GameOverlayActivated_t);

    void MountAdditionalContent();
};