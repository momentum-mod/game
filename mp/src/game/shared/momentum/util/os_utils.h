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
#ifdef OSX
#define CLIENT_DLL "./momentum/bin/client.dylib" //OSX
#else
#define CLIENT_DLL "./momentum/bin/client.so" //LINUX
#endif

#else //POSIX
#define CLIENT_DLL "./momentum/bin/client.dll" //WIN32
#endif //OS_UTILS_H