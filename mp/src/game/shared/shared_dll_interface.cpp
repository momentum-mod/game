#include "shared_dll_interface.h"

static CShared g_Shared;
CShared *Shared()
{
	return &g_Shared;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CShared, CShared, INTERFACEVERSION_SHAREDGAMEDLL, g_Shared);