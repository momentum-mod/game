#pragma once

#include "interface.h"


class CHudReplay;

class CShared
{
  public:
    bool LoadedClient = false;
    bool LoadedServer = false;

	//ReplayUI Stuffs
	CHudReplay *HudReplay = nullptr;
	float RGUI_TimeScale = 1.0f;
    bool RGUI_bIsPlaying = false;
    int RGUI_HasSelected = 0;
};

#define INTERFACEVERSION_SHAREDGAMEDLL "SHAREDGAMEDLL001"