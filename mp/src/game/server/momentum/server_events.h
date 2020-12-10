#pragma once

class CMomServerEvents : CAutoGameSystem
{
public:
    CMomServerEvents();

    bool Init() OVERRIDE;
    void PostInit() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
};