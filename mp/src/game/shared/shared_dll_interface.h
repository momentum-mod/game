#pragma once

#include "interface.h"

class CShared
{
  public:
    int m_iTotalTicks = 0;
    int m_iCurrentTick = 0;
	int m_iCountAfterStartZone_Client_Timer = 0;
    bool m_bIsPlaying = false;
    int m_iTotalTicks_Client_Timer = 0;
    bool LoadedClient = false;
    bool LoadedServer = false;
};

#define INTERFACEVERSION_SHAREDGAMEDLL "SHAREDGAMEDLL001"