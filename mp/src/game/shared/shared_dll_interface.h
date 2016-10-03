#pragma once

#include "interface.h"


class CHudReplay;

class CShared
{
  public:
    bool LoadedClient = false;
    bool LoadedServer = false;


	int m_iCurrentTick_Server = 0;
	int m_iTotalTicks_Client_Timer = 0;

	//ReplayUI Stuffs
	CHudReplay *HudReplay = nullptr;
	float RGUI_TimeScale = 1.0f;
    bool RGUI_bIsPlaying = false;
    int RGUI_HasSelected = 0;
};

#define INTERFACEVERSION_SHAREDGAMEDLL "SHAREDGAMEDLL001"