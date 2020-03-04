#pragma once
//---------------------------------------------------------------------------------------------
// Functions used to find patterns of bytes mainly in the engine binary for patching/hooking
//---------------------------------------------------------------------------------------------
//
#include "cbase.h"
#include "tier0/platform.h"

class EnginePatch
{
public:
	static void InitPatches();
	static inline bool DataCompare(const unsigned char*, const unsigned char*, const char*);
	static void* FindPattern(const unsigned char*, const char*, size_t);

private:
	static void* moduleBase;
	static size_t moduleSize;
};