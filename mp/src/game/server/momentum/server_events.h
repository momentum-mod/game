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

class CMOMServerEvents : CAutoGameSystemPerFrame
{
public:
    CMOMServerEvents(const char* pName);

    void PostInit() OVERRIDE;
    void Shutdown() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;

    void OnAuthHTTP(HTTPRequestCompleted_t *pParam, bool bIOFailure);
    CCallResult<CMOMServerEvents, HTTPRequestCompleted_t> m_cAuthCallresult;
    STEAM_CALLBACK(CMOMServerEvents, OnGameOverlay, GameOverlayActivated_t);
    STEAM_CALLBACK(CMOMServerEvents, OnAuthTicket, GetAuthSessionTicketResponse_t);

    void DoAuth();
    void MountAdditionalContent();

private:
    CMapzoneData* zones;
    HAuthTicket m_hAuthTicket;
    byte* m_bufAuthBuffer;
    uint32 m_iAuthActualSize;
    char *m_pAPIKey;
};


#endif // SERVER_EVENTS_H
