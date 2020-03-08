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

    static inline bool DataCompare(const char*, const char*, const char*);
    static void* FindPattern(const char*, const char*, size_t = 0);

    static bool SetMemoryProtection(void*, size_t, int);

    static void* GetModuleBase() { return moduleBase; }
    static size_t GetModuleSize() { return moduleSize; }

private:
    static void* moduleBase;
    static size_t moduleSize;
};

enum PatchType
{
    PATCH_IMMEDIATE = true,
    PATCH_REFERENCE = false
};

enum PatchStatus
{
    PATCH_SUCCESS,
    PATCH_MEMPROTECT_FAIL,
    PATCH_INVALID_SIGNATURE,
    PATCH_NO_VALUE
};

class CEnginePatch
{
public:
    CEnginePatch(const char*, char*, char*, size_t, bool);
    CEnginePatch(const char*, char*, char*, size_t, bool, int);
    CEnginePatch(const char*, char*, char*, size_t, bool, float);
    CEnginePatch(const char*, char*, char*, size_t, bool, char*);

    int ApplyPatch();
    static void ApplyAll();

    const char* GetName() { return m_sName; }

private:
    const char* m_sName;
    char *m_pSignature;
    const char *m_pMask;
    size_t m_iOffset;
    bool m_bImmediate;
    char* m_pPatch;
    size_t m_iLength;
};
