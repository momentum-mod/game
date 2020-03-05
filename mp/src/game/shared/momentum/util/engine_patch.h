#pragma once
//------------------------------------------------------------------------------------
// Functions used to find patterns of bytes in the engine's memory to hook or patch
//------------------------------------------------------------------------------------
//
#include "cbase.h"
#include "tier0/platform.h"

class CEngineBinary : CAutoGameSystem
{
public:
    CEngineBinary();

    bool Init() OVERRIDE;
    void PostInit() OVERRIDE;

    static inline bool DataCompare(const unsigned char*, const unsigned char*, const char*);
    static void* FindPattern(const unsigned char*, const char*, size_t);


private:
	static void* moduleBase;
	static size_t moduleSize;
};