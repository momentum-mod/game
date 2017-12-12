#pragma once
#include "cbase.h"
//-----------------------------------------------------------------------------
// Cross-platform dynamic lib stuff
//-----------------------------------------------------------------------------
//
#ifdef POSIX
#include <dlfcn.h>
#include <libgen.h>
//#include <unistd.h>
// Linux doesn't have this function so this emulates its functionality
extern void *GetModuleHandle(const char *name);

// Moved from tickset.cpp because fuck that
extern int GetModuleInformation_LINUX(const char *name, void **base, size_t *length);
extern int GetModuleInformation_OSX(const char *name, void **base, size_t *length);

//GetProcAddress can be flat-out replaced with dlsym
//So use GetProcAddress on both platforms.

#define GetProcAddress dlsym
#ifdef OSX
#define CLIENT_DLL_NAME "./momentum/bin/client.dylib" //OSX
#include <mach-o/dyld_images.h>
#include <mach-o/dyld.h>
#else
#define CLIENT_DLL_NAME "./momentum/bin/client.so" //LINUX
#endif

#else //POSIX
#define CLIENT_DLL_NAME "./momentum/bin/client.dll" //WIN32
#endif //OS_UTILS_H