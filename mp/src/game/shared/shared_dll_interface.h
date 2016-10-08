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
    bool RGUI_bIsPlaying = false;
    int RGUI_HasSelected = 0;
};

#define INTERFACEVERSION_SHAREDGAMEDLL "SHAREDGAMEDLL001"