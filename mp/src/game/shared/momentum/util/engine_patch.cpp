#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "Psapi.h"
#pragma comment(lib, "psapi.lib")
#define MEM_READ 1
#define MEM_WRITE 2
#define MEM_EXEC 4
#elif defined (POSIX)
#include "sys/mman.h"
#define MEM_READ PROT_READ
#define MEM_WRITE PROT_WRITE
#define MEM_EXEC PROT_EXEC
// Addresses must be aligned to page size for linux
#define LALIGN(addr) (void*)((uintptr_t)(addr) & ~(getpagesize() - 1))
#define LALDIF(addr) ((uintptr_t)(addr) % getpagesize())
#endif

#include "cbase.h"
#include "util/os_utils.h"
#include "engine_patch.h"

// Engine Patch format:
//==============================
// m_sName:         Patch name
// m_pSignature:    Memory signature
// m_pMask:         Signature mask
// m_iOffset:       Patch offset
// m_bImmediate:    Immediate or referenced variable
// m_pPatch:        Patch bytes (int/float/char*)
// m_iLength:       Patch length (only with char* patches)
//==============================
CEnginePatch g_EnginePatches[] =
{
#ifdef _WIN32
    // Prevent the culling of skyboxes at high FOVs
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_warp.cpp#L315
    {
        "SkyboxCulling",
        "\xF3\x0F\x59\x15\x00\x00\x00\x00\xF3\x0F\x58\xC1\xF3\x0F\x10\x0D",
        "xxxx????xxxxxxxx",
        16,
        PATCH_REFERENCE,
        -1.0f
    },
    // Replace == (jz = 74) with >= (jge = 7D) to prevent static props above 4095 from bypassing this check and causing crashes
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/staticpropmgr.cpp#L1748
    {
        "IsStaticPropPatch",
        "\x55\x8B\xEC\x8B\x4D\x08\x85\xC9\x74\x19\x8B\x01",
        "xxxxxxxxxxxx",
        27,
        PATCH_IMMEDIATE,
        "\x7D",
        1
    },
    // Example patch: Trigger "Map has too many brushes" error at 16384 brushes instead of 8192
    //{
    //    "BrushLimit",
    //    "\xC1\xEF\x03\x81\xFF\x00\x00\x00\x00",
    //    "xxxxx????",
    //    5,
    //    PATCH_IMMEDIATE,
    //    16384
    //},
    // The same patch as above but using a pure hex value
    //{
    //    "BrushLimitHex",
    //    "\xC1\xEF\x03\x81\xFF\x00\x00\x00\x00",
    //    "xxxxx????",
    //    5,
    //    PATCH_IMMEDIATE,
    //    "\x00\x40\x00\x00",
    //    4
    //}
#elif __linux__
    // Prevent the culling of skyboxes at high FOVs
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_warp.cpp#L315
    {
        "SkyboxCulling",
        "\xF3\x0F\x59\x0D\x00\x00\x00\x00\xF3\x0F\x58\xC2\xF3\x0F\x58\xC1\xF3\x0F\x10\x0D",
        "xxxx????xxxxxxxxxxxx",
        20,
        PATCH_REFERENCE,
        -1.0f
    },
    // Replace == (setz = 0F 94) with >= (setae = 0F 93) to prevent static props above 4095 from bypassing this check and causing crashes
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/staticpropmgr.cpp#L1748
    {
        "IsStaticPropPatch",
        "\x55\xB8\x00\x00\x00\x00\x89\xE5\x83\xEC\x18\x8B\x55\x0C\x85\xD2\x74\x15",
        "xx????xxxxxxxxxxxx",
        36,
        PATCH_IMMEDIATE,
        "\x0F\x93",
        2
    },
    // Example patch: Trigger "Map has too many brushes" error at 16384 brushes instead of 8192
    //{
    //    "BrushLimit",
    //    "\xBE\x00\x00\x00\x00\xF7\xE6\x89\xD6\xC1\xEE\x03\x81\xFE",
    //    "x????xxxxxxxxx",
    //    14,
    //    PATCH_IMMEDIATE,
    //    16384
    //},
    // The same patch as above but using a pure hex value
    //{
    //    "BrushLimitHex",
    //    "\xBE\x00\x00\x00\x00\xF7\xE6\x89\xD6\xC1\xEE\x03\x81\xFE",
    //    "x????xxxxxxxxx",
    //    14,
    //    PATCH_IMMEDIATE,
    //    "\x00\x40\x00\x00",
    //    4
    //}
#endif //_WIN32
};

CEngineBinary::CEngineBinary() : CAutoGameSystem("CEngineBinary")
{
}

void* CEngineBinary::m_pModuleBase = nullptr;
size_t CEngineBinary::m_iModuleSize = 0;

// Get the engine's base address and size
bool CEngineBinary::Init()
{
#ifdef _WIN32
    HMODULE handle = GetModuleHandleA(ENGINE_DLL_NAME);
    if (!handle)
        return false;

    MODULEINFO info;
    GetModuleInformation(GetCurrentProcess(), handle, &info, sizeof(info));

    m_pModuleBase = info.lpBaseOfDll;
    m_iModuleSize = info.SizeOfImage;
#else //POSIX
    if (GetModuleInformation(ENGINE_DLL_NAME, &m_pModuleBase, &m_iModuleSize))
        return false;
#endif //_WIN32

    return true;
}

void CEngineBinary::PostInit()
{
#ifdef GAME_DLL
    ApplyAllPatches();
#endif
}

inline bool CEngineBinary::DataCompare(const char* data, const char* pattern, const char* mask)
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
void* CEngineBinary::FindPattern(const char* pattern, const char* mask, size_t offset)
{
    auto maskLength = strlen(mask);
    for (size_t i = 0; i <= m_iModuleSize - maskLength; ++i)
    {
        auto addr = reinterpret_cast<char*>(m_pModuleBase) + i;
        if (DataCompare(addr, pattern, mask))
            return reinterpret_cast<void*>(addr + offset);
    }

    return nullptr;
}

bool CEngineBinary::SetMemoryProtection(void* pAddress, size_t iLength, int iProtection)
{
#ifdef _WIN32
    // VirtualProtect requires a valid pointer to store the old protection value
    DWORD tmp;
    DWORD prot;

    switch (iProtection)
    {
    case MEM_READ:
        prot = PAGE_READONLY; break;
    case MEM_READ | MEM_WRITE:
        prot = PAGE_READWRITE; break;
    case MEM_READ | MEM_EXEC:
        prot = PAGE_EXECUTE_READ; break;
    default:
    case MEM_READ | MEM_WRITE | MEM_EXEC:
        prot = PAGE_EXECUTE_READWRITE; break;
    }

    return VirtualProtect(pAddress, iLength, prot, &tmp);
#else //POSIX
    return mprotect(LALIGN(pAddress), iLength + LALDIF(pAddress), iProtection) == 0;
#endif //_WIN32
}

void CEngineBinary::ApplyAllPatches()
{
#if !defined (OSX) // No OSX patches
    for (int i = 0; i < sizeof(g_EnginePatches) / sizeof(*g_EnginePatches); i++)
        g_EnginePatches[i].ApplyPatch();
#endif
}

CEngineBinary g_EngineBinary;

void CEnginePatch::ApplyPatch()
{
    if (!m_pPatch)
    {
        Warning("Engine patch \"%s\" FAILED: No value provided\n", m_sName);
        return;
    }

    void* addr = CEngineBinary::FindPattern(m_pSignature, m_pMask, m_iOffset);

    if (addr)
    {
        auto pMemory = m_bImmediate ? (uintptr_t*)addr : *reinterpret_cast<uintptr_t**>(addr);

        // Memory is write-protected so it needs to be lifted before the patch is applied
        if (CEngineBinary::SetMemoryProtection(pMemory, m_iLength, MEM_READ|MEM_WRITE|MEM_EXEC))
        {
            Q_memcpy(pMemory, m_pPatch, m_iLength);

            CEngineBinary::SetMemoryProtection(pMemory, m_iLength, MEM_READ|MEM_EXEC);

            DevLog("Engine patch \"%s\" applied successfully\n", m_sName);
        }
        else
        {
            Warning("Engine patch \"%s\" FAILED: Could not override memory protection\n", m_sName);
        }
    }
    else
    {
        Warning("Engine patch \"%s\" FAILED: Could not find signature\n", m_sName);
    }
}

CEnginePatch::CEnginePatch(const char* name, char* signature, char* mask, size_t offset, bool immediate)
{
    m_sName = name;
    m_pSignature = signature;
    m_pMask = mask;
    m_iOffset = offset;
    m_iLength = 0;
    m_bImmediate = immediate;
    m_pPatch = nullptr;
}

// Converting numeric types into bytes
CEnginePatch::CEnginePatch(const char* name, char* signature, char* mask, size_t offset, bool immediate, int value)
    : CEnginePatch(name, signature, mask, offset, immediate)
{
    m_iLength = sizeof(value);
    m_pPatch = new char[m_iLength];
    Q_memcpy(m_pPatch, &value, m_iLength);
}

CEnginePatch::CEnginePatch(const char* name, char* signature, char* mask, size_t offset, bool immediate, float value)
    : CEnginePatch(name, signature, mask, offset, immediate)
{
    m_iLength = sizeof(value);
    m_pPatch = new char[m_iLength];
    Q_memcpy(m_pPatch, &value, m_iLength);
}

CEnginePatch::CEnginePatch(const char* name, char* signature, char* mask, size_t offset, bool immediate, char* bytes, size_t length)
    : CEnginePatch(name, signature, mask, offset, immediate)
{
    m_iLength = length;
    m_pPatch = bytes;
}
