//------------------------------------------------------------------------------------
// Functions used to find patterns of bytes in the engine's memory to hook or patch
//-----------------------------------------------------------------------------------
#pragma once

class CEngineBinary : public CAutoGameSystem
{
public:
    CEngineBinary();

    bool Init() OVERRIDE;
    void PostInit() OVERRIDE;

    static inline bool DataCompare(const char*, const char*, const char*);
    static void* FindPattern(const char*, const char*, size_t = 0);

    static bool SetMemoryProtection(void*, size_t, int);

    static void* GetModuleBase() { return m_pModuleBase; }
    static size_t GetModuleSize() { return m_iModuleSize; }

private:
    void ApplyAllPatches();

    static void* m_pModuleBase;
    static size_t m_iModuleSize;
};

enum PatchType
{
    PATCH_IMMEDIATE = true,
    PATCH_REFERENCE = false
};

class CEnginePatch
{
public:
    CEnginePatch(const char*, char*, char*, size_t, bool);
    CEnginePatch(const char*, char*, char*, size_t, bool, int);
    CEnginePatch(const char*, char*, char*, size_t, bool, float);
    CEnginePatch(const char*, char*, char*, size_t, bool, char*, size_t);

    void ApplyPatch();

private:
    const char *m_sName;

    char *m_pSignature;
    char *m_pMask;
    char *m_pPatch;

    size_t m_iOffset;
    size_t m_iLength;

    bool m_bImmediate;
};
