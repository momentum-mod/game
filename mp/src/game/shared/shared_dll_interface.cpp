#include "shared_dll_interface.h"

C_SharedDLL SharedDLL;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(C_SharedDLL, C_SharedDLL, INTERFACEVERSION_SHAREDGAMEDLL, SharedDLL);

void C_SharedDLL::Something()
{
	
}