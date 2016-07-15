#ifndef MOMRUNPOSTER_H
#define MOMRUNPOSTER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"

#include "GameEventListener.h"
#include "KeyValues.h"
#include "gason.h"
#include "momentum/mom_shareddefs.h"
#include "steam/steam_api.h"

class CRunPoster : CGameEventListener
{
  public:
    CRunPoster();
    ~CRunPoster();

    void Init();
    void FireGameEvent(IGameEvent *pEvent) override;

    void PostTimeCallback(HTTPRequestCompleted_t *, bool);
    CCallResult<CRunPoster, HTTPRequestCompleted_t> cbPostTimeCallback;
};
extern CRunPoster *g_MOMRunPoster;
#endif // MOMRUNPOSTER_H