#include "interface.h"

class C_SharedDLL
{
public:

	virtual void Something();

	bool Simple = false;
}; 

#define INTERFACEVERSION_SHAREDGAMEDLL			"SHAREDGAMEDLL001"