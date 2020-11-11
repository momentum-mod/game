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

enum PatchType : char
{
    PATCH_IMMEDIATE = true,
    PATCH_REFERENCE = false
};

class CEnginePatch final
{
    CEnginePatch(const char*, const char*, const char*, size_t, PatchType, bool);
public:
    CEnginePatch(const char*, const char*, const char*, size_t, PatchType, int);
    CEnginePatch(const char*, const char*, const char*, size_t, PatchType, float);
    CEnginePatch(const char*, const char*, const char*, size_t, PatchType, const char*, size_t);

    void ApplyPatch() const;

private:
    const char *m_sName;

    const char *m_pSignature;
    const char *m_pMask;
    union
    {
        const char *m_pPatch;
        char m_patch[4];
    };

    size_t m_iOffset;
    size_t m_iLength;

    bool m_bImmediate : 1;
    bool m_bIsPtr : 1;
};
