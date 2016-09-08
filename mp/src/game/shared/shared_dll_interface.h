#include "interface.h"

class C_SharedDLL
{
public:

	virtual void Something();

	bool LoadedClient = false;
	bool LoadedServer = false;
}; 

#define INTERFACEVERSION_SHAREDGAMEDLL			"SHAREDGAMEDLL001"