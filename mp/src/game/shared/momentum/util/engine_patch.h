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
    static void* FindPattern(const unsigned char*, const char*, size_t = 0);

	static void* getModuleBase() { return moduleBase; }
	static size_t getModuleSize() { return moduleSize; }

private:
    static void* moduleBase;
    static size_t moduleSize;
};

class CEnginePatch
{
public:
    CEnginePatch(const char*, unsigned char*, char*, size_t, bool);
    CEnginePatch(const char*, unsigned char*, char*, size_t, bool, int);
    CEnginePatch(const char*, unsigned char*, char*, size_t, bool, float);
    CEnginePatch(const char*, unsigned char*, char*, size_t, bool, unsigned char*);

    int ApplyPatch();
    static void ApplyAll();

    const char* getName() { return m_sName; }

private:
    const char* m_sName;
    unsigned char *m_pPatchSignature;
    const char *m_pMask;
    size_t m_iOffset;
    size_t m_iPatchLength;
    unsigned char* m_pPatch;
    bool m_bImmediate;
};
