#ifndef CLIENT_EVENTS_H
#define CLIENT_EVENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

class CMOMClientEvents : public CAutoGameSystem
{
public:
    CMOMClientEvents(const char *pName) : CAutoGameSystem(pName)
    {
    }

    //After DLL inits successfully
    void PostInit() override;

    //From the Valve SDK wiki
    static void MountAdditionalContent();

};

#endif // CLIENT_EVENTS_H