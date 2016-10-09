#pragma once

#include "interface.h"

class CShared
{
  public:
    bool LoadedClient = false;
    bool LoadedServer = false;

    // ReplayUI Stuffs
    bool RGUI_bIsPlaying = false;
    int RGUI_HasSelected = 0;
};

#define INTERFACEVERSION_SHAREDGAMEDLL "SHAREDGAMEDLL001"