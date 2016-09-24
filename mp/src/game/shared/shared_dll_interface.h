#include "interface.h"

class C_SharedDLL
{
public:

	virtual void Something();

	int m_iTotalTicks;
	int m_iCurrentTick;
	bool m_bIsPlaying;
	int m_iTotalTicksT;
	bool LoadedClient;
	bool LoadedServer;
}; 

#define INTERFACEVERSION_SHAREDGAMEDLL			"SHAREDGAMEDLL001"
