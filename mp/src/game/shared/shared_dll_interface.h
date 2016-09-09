#include "interface.h"

class C_SharedDLL
{
public:

	virtual void Something();

	int m_iTotalTicks = 0;
	int m_iCurrentTick = 0;
	bool m_bIsPlaying = false;

	bool LoadedClient = false;
	bool LoadedServer = false;
}; 

#define INTERFACEVERSION_SHAREDGAMEDLL			"SHAREDGAMEDLL001"