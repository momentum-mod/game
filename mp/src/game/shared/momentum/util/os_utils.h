#pragma once
#include "cbase.h"
//-----------------------------------------------------------------------------
// Cross-platform dynamic lib stuff
//-----------------------------------------------------------------------------
//
#ifdef POSIX
#include <dlfcn.h>
//#include <unistd.h>
// Linux doesn't have this function so this emulates its functionality
extern void *GetModuleHandle(const char *name);

//GetProcAddress can be flat-out replaced with dlsym
//So use GetProcAddress on both platforms.
#define GetProcAddress dlsym
#define CLIENT_DLL "./momentum/bin/client.so"

#else //Not POSIX

#define CLIENT_DLL "./momentum/bin/client.dll"

#endif //OS_UTILS_H