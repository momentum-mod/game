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

extern int GetModuleInformation(const char *name, void **base, size_t *length);

//GetProcAddress can be flat-out replaced with dlsym
//So use GetProcAddress on both platforms.

#define GetProcAddress dlsym
#ifdef OSX
#define CLIENT_DLL_NAME "./momentum/bin/client.dylib" //OSX
#define SERVER_DLL_NAME "./momentum/bin/server.dylib" //OSX
#define ENGINE_DLL_NAME "engine.dylib"
#include <mach-o/dyld_images.h>
#include <mach-o/dyld.h>
#else
#define CLIENT_DLL_NAME "./momentum/bin/client.so" //LINUX
#define SERVER_DLL_NAME "./momentum/bin/server.so" //LINUX
#define ENGINE_DLL_NAME "engine.so"

#endif

#else //POSIX
#ifdef _WIN32
#pragma warning( disable: 4005 )
#include "Windows.h"
#endif

#define CLIENT_DLL_NAME "./momentum/bin/client.dll" //WIN32
#define SERVER_DLL_NAME "./momentum/bin/server.dll" //WIN32
#define ENGINE_DLL_NAME "engine.dll"

#endif //OS_UTILS_H