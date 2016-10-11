#pragma once

#include "interface.h"

class CShared
{
  public:
    bool LoadedClient = false;
    bool LoadedServer = false;
};

#define INTERFACEVERSION_SHAREDGAMEDLL "SHAREDGAMEDLL001"