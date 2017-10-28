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
    void PostInit() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;

    //From the Valve SDK wiki
    static void MountAdditionalContent();

    // Precaching things we want to here, done every level start
    // Used mostly for materials
    void Precache();

};

#endif // CLIENT_EVENTS_H