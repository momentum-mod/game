#pragma once

#include "interface.h"

class CShared
{
  public:
    bool LoadedClient = false;
    bool LoadedServer = false;
	void* HudReplay = NULL;
	float TickRate = 0;
	int m_iTotalTicks = 0;
	int m_iCurrentTick = 0;
	bool m_bIsPlaying = false;
	int m_iTotalTicks_Client_Timer = 0;
	int HasSelected = 0;
};

#define INTERFACEVERSION_SHAREDGAMEDLL "SHAREDGAMEDLL001"