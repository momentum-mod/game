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
#endif //_WIN32

    return true;
}

void CEngineBinary::PostInit()
{
    CEnginePatch::ApplyAll();
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
    for (size_t i = 0; i <= moduleSize - maskLength; ++i)
    {
        auto addr = reinterpret_cast<char*>(moduleBase) + i;
        if (DataCompare(addr, pattern, mask))
            return const_cast<void*>(reinterpret_cast<const void*>(addr + offset));
    }

    return nullptr;
}

int CEngineBinary::SetMemoryProtection(void* pAddress, size_t iLength, int iProtection, unsigned long* pOriginalProtection)
{
#ifdef _WIN32
    return VirtualProtect(pAddress, iLength, iProtection, pOriginalProtection);
#else //POSIX
    return mprotect(LALIGN(pAddress), iLength + LALDIF(pAddress), iProtection) == 0;
#endif //_WIN32
}

CEngineBinary g_EngineBinary;

// Engine Patch format:
//==============================
// m_sName:         Patch name
// m_pSignature:    Memory signature
// m_pMask:         Signature mask
// m_iOffset:       Patch offset
// m_bImmediate:    Immediate or referenced variable
// m_pPatch:        Patch bytes (int/float/char*)
//==============================
#ifdef _WIN32
CEnginePatch g_EnginePatches[] =
{
    // Prevent the culling of skyboxes at high FOVs
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_warp.cpp#L315
    // TODO: Use a value derived from FOV instead
    {
        "SkyboxCulling",
        "\xF3\x0F\x59\x15\x00\x00\x00\x00\xF3\x0F\x58\xC1\xF3\x0F\x10\x0D",
        "xxxx????xxxxxxxx",
        16,
        PATCH_REFERENCE,
        -1.0f
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
    //    "\x00\x40\x00\x00"
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
        "\xF3\x0F\x59\x0D\x00\x00\x00\x00\xF3\x0F\x58\xC2\xF3\x0F\x58\xC1\xF3\x0F\x10\x0D",
        "xxxx????xxxxxxxxxxxx",
        20,
        PATCH_REFERENCE,
        -1.0f
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
    //    "\x00\x40\x00\x00"
    //}
};
#endif //_WIN32

void CEnginePatch::ApplyAll()
{
#if !defined (OSX) // No OSX patches
    int err;

    for (int i = 0; i < sizeof(g_EnginePatches) / sizeof(*g_EnginePatches); i++)
    {
        err = g_EnginePatches[i].ApplyPatch();
        switch (err)
        {
        case PATCH_SUCCESS:
            DevLog("Engine patch \"%s\" applied successfully\n", g_EnginePatches[i].GetName());
            break;
        case PATCH_MEMPROTECT_FAIL:
            Warning("Engine patch \"%s\" FAILED: Could not override memory protection\n", g_EnginePatches[i].GetName());
            break;
        case PATCH_INVALID_SIGNATURE:
            Warning("Engine patch \"%s\" FAILED: Could not find signature\n", g_EnginePatches[i].GetName());
            break;
        case PATCH_NO_VALUE:
            Warning("Engine patch \"%s\" FAILED: No value provided\n", g_EnginePatches[i].GetName());
        }
    }
#endif // (OSX)
}

// Apply a patch to engine memory
int CEnginePatch::ApplyPatch()
{
    if (!m_pPatch)
        return PATCH_NO_VALUE;

    void* addr = CEngineBinary::FindPattern(m_pSignature, m_pMask, m_iOffset);

    if (addr)
    {
        auto pMemory = m_bImmediate ? (uintptr_t*)addr : *reinterpret_cast<uintptr_t**>(addr);

        // Memory is write-protected so it needs to be lifted before the patch is applied
#ifdef _WIN32
        // https://docs.microsoft.com/en-us/windows/win32/memory/memory-protection-constants
        unsigned long iOldProtection;
        unsigned long iNewProtection = PAGE_EXECUTE_READWRITE;
#else //POSIX
        // http://man7.org/linux/man-pages/man2/mprotect.2.html
        unsigned long iOldProtection = PROT_READ | PROT_EXEC;
        unsigned long iNewProtection = PROT_READ | PROT_WRITE | PROT_EXEC;
#endif //_WIN32

        if (CEngineBinary::SetMemoryProtection(pMemory, m_iLength, iNewProtection, &iOldProtection))
        {
            Q_memcpy(pMemory, m_pPatch, m_iLength);

            // Restore old protections
            CEngineBinary::SetMemoryProtection(pMemory, m_iLength, iOldProtection, &iNewProtection);

            return PATCH_SUCCESS;
        }
        return PATCH_MEMPROTECT_FAIL;
    }
    return PATCH_INVALID_SIGNATURE;
}

// Constructors
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

CEnginePatch::CEnginePatch(const char* name, char* signature, char* mask, size_t offset, bool immediate, char* bytes)
    : CEnginePatch(name, signature, mask, offset, immediate)
{
    m_iLength = sizeof(bytes);
    m_pPatch = bytes;
}
