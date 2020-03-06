//------------------------------------------------------------------------------------
// Functions used to find patterns of bytes in the engine's memory to hook or patch
//------------------------------------------------------------------------------------
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "Psapi.h"
#pragma comment(lib, "psapi.lib")
#elif defined (POSIX)
#include "util/os_utils.h"
#include "sys/mman.h"
#endif

#include "engine_patch.h"
#include "mom_shareddefs.h"
#include "tier0/platform.h"

void* CEngineBinary::moduleBase = nullptr;
size_t CEngineBinary::moduleSize;

CEngineBinary::CEngineBinary() : CAutoGameSystem("CEngineBinary")
{
}

// Get the engine's base address and size
bool CEngineBinary::Init()
{
#ifdef _WIN32
    HMODULE handle = GetModuleHandleA("engine.dll");
    if (!handle)
        return false;

    MODULEINFO info;
    GetModuleInformation(GetCurrentProcess(), handle, &info, sizeof(info));

    moduleBase = info.lpBaseOfDll;
    moduleSize = info.SizeOfImage;
#else //POSIX
    if (GetModuleInformation(ENGINE_DLL_NAME, &moduleBase, &moduleSize))
        return false;
#endif //WIN32

    return true;
}

void CEngineBinary::PostInit()
{
    CEnginePatch::ApplyAll();
}

inline bool CEngineBinary::DataCompare(const unsigned char* data, const unsigned char* pattern, const char* mask)
{
    for (; *mask != 0; ++data, ++pattern, ++mask)
        if (*mask == 'x' && *data != *pattern)
            return false;

    return (*mask == 0);
}

//---------------------------------------------------------------------------------------------------------
// Finds a pattern of bytes in the engine memory given a signature and a mask
// Returns the address of the first (and hopefully only) match with an optional offset, otherwise nullptr
//---------------------------------------------------------------------------------------------------------
void* CEngineBinary::FindPattern(const unsigned char* pattern, const char* mask, size_t offset = 0)
{
    auto maskLength = strlen(mask);
    for (size_t i = 0; i <= moduleSize - maskLength; ++i)
    {
        auto addr = reinterpret_cast<const unsigned char*>(moduleBase) + i;
        if (DataCompare(addr, pattern, mask))
            return const_cast<void*>(reinterpret_cast<const void*>(addr + offset));
    }

    return nullptr;
}

CEngineBinary g_EngineBinary;

// Engine Patch format:
//
// Patch name
// Memory signature
// Signature mask
// Patch offset
// Immediate or referenced variable
// Patch bytes (int/float/char*)
#ifdef _WIN32
CEnginePatch g_EnginePatches[] =
{
    // Prevent the culling of skyboxes at high FOVs
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_warp.cpp#L315
    // TODO: Use a value derived from FOV instead
    {
        "SkyboxCulling",
        (unsigned char*)"\xF3\x0F\x59\x15\x00\x00\x00\x00\xF3\x0F\x58\xC1\xF3\x0F\x10\x0D",
        "xxxx????xxxxxxxx",
        16,
        false,
        (float)-1
    },
    // Example patch: Trigger "Map has too many brushes" error at 16384 brushes instead of 8192
    //{
    //    "BrushLimit",
    //    (unsigned char*)"\xC2\xEF\x03\x81\xFF\x00\x00\x00\x00",
    //    "xxxxx????",
    //    5,
    //    true,
    //    16384
    //},
    // The same patch as above but using a pure hex value
    //{
    //    "BrushLimit2",
    //    (unsigned char*)"\xC1\xEF\x03\x81\xFF\x00\x00\x00\x00",
    //    "xxxxx????",
    //    5,
    //    true,
    //    (unsigned char*)"\x00\x40\x00\x00"
    //}
};
#elif __linux__
CEnginePatch g_EnginePatches[] =
{
    // Prevent the culling of skyboxes at high FOVs
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_warp.cpp#L315
    // TODO: Use a value derived from FOV instead
    {
        "SkyboxCulling",
        (unsigned char*)"\xF3\x0F\x59\x0D\x00\x00\x00\x00\xF3\x0F\x58\xC2\xF3\x0F\x58\xC1\xF3\x0F\x10\x0D",
        "xxxx????xxxxxxxxxxxx",
        20,
        false,
        (float)-1
    }
};
#endif //WIN32

void CEnginePatch::ApplyAll()
{
#if !defined (OSX) // No OSX patches yet
    int err;

    for (int i = 0; i < sizeof(g_EnginePatches) / sizeof(*g_EnginePatches); i++)
    {
        err = g_EnginePatches[i].ApplyPatch();
        switch (err)
        {
        case 1:
            Warning("Engine patch \"%s\" FAILED: Could not override memory protection\n", g_EnginePatches[i].getName());
            break;
        case 2:
            Warning("Engine patch \"%s\" FAILED: Could not find signature\n", g_EnginePatches[i].getName());
        }
    }
#endif // (OSX)
}

// Apply a patch to engine memory
int CEnginePatch::ApplyPatch()
{
#ifdef _WIN32

    uintptr_t addr = reinterpret_cast<uintptr_t>(CEngineBinary::FindPattern(m_pPatchSignature, m_pMask, m_iOffset));

    if (addr)
    {
        auto pMemory = m_bImmediate ? (uintptr_t*)addr : *reinterpret_cast<uintptr_t**>(addr);

        // Memory is write-protected so it needs to be lifted before the patch is applied
        // 0x40 is read,write,execute
        // https://docs.microsoft.com/en-us/windows/win32/memory/memory-protection-constants
        unsigned long iOldProtection, iNewProtection = 0x40;

        if (VirtualProtect(pMemory, m_iPatchLength, iNewProtection, &iOldProtection))
        {
            Q_memcpy(pMemory, m_pPatch, m_iPatchLength);

            // Restore old protections
            VirtualProtect(pMemory, m_iPatchLength, iOldProtection, &iNewProtection);

            return 0;
        }
        return 1;
    }
    return 2;

#elif defined __linux__

    void* addr = CEngineBinary::FindPattern(m_pPatchSignature, m_pMask, m_iOffset);

    if (addr)
    {
        auto pMemory = m_bImmediate ? (uintptr_t*)addr : *(uintptr_t**)(addr);

        // Memory is write-protected so it needs to be lifted before the patch is applied
        // http://man7.org/linux/man-pages/man2/mprotect.2.html
        if (!mprotect(pMemory, m_iPatchLength, PROT_READ | PROT_WRITE | PROT_EXEC))
        {
            Q_memcpy(pMemory, m_pPatch, m_iPatchLength);

            // Restore old protections
            mprotect(pMemory, m_iPatchLength, PROT_READ | PROT_EXEC);

            return 0;
        }
        return 1;
    }
    return 2;

#endif //WIN32
}

// Constructors
CEnginePatch::CEnginePatch(const char* name, unsigned char* signature, char* mask, size_t offset, bool immediate)
{
    m_sName = name;
    m_pPatchSignature = signature;
    m_pMask = mask;
    m_iOffset = offset;
    m_iPatchLength = 0;
    m_bImmediate = immediate;
    m_pPatch = new unsigned char;
}

CEnginePatch::CEnginePatch(const char* name, unsigned char* signature, char* mask, size_t offset, bool immediate, int value)
    : CEnginePatch(name, signature, mask, offset, immediate)
{
    m_iPatchLength = sizeof(value);
    Q_memcpy(m_pPatch, &value, m_iPatchLength);
}

CEnginePatch::CEnginePatch(const char* name, unsigned char* signature, char* mask, size_t offset, bool immediate, float value)
    : CEnginePatch(name, signature, mask, offset, immediate)
{
    m_iPatchLength = sizeof(value);
    Q_memcpy(m_pPatch, &value, m_iPatchLength);
}

CEnginePatch::CEnginePatch(const char* name, unsigned char* signature, char* mask, size_t offset, bool immediate, unsigned char* bytes)
    : CEnginePatch(name, signature, mask, offset, immediate)
{
    m_iPatchLength = sizeof(bytes);
    m_pPatch = (unsigned char*)bytes;
}
