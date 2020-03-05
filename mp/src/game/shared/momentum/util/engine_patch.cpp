#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "Psapi.h"
#pragma comment(lib, "psapi.lib")
#elif defined (POSIX)
#include "util/os_utils.h"
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

#endif //WIN32

#ifdef _WIN32
	// Prevent the culling of skyboxes at high FOVs
	// https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_warp.cpp#L315
	// TODO: Use a value derived from FOV instead
	unsigned char pattern[] = { 0xF3, 0x0F, 0x59, 0x15, '?', '?', '?', '?', 0xF3, 0x0F, 0x58, 0xC1, 0xF3, 0x0F, 0x10, 0x0D };
	auto addr = reinterpret_cast<uintptr_t>(FindPattern(pattern, "xxxx????xxxxxxxx", 16));

	if (addr)
	{
		// The value is stored in the data segment so it needs write permission
		float* fValue = *reinterpret_cast<float**>(addr);

		// 0x40 is read,write,execute
		unsigned long iOldProtection, iNewProtection = 0x40;

		if (VirtualProtect((void*)fValue, sizeof(float), iNewProtection, &iOldProtection))
		{
			*fValue = -1;

			// Restore old protections
			VirtualProtect((void*)fValue, sizeof(float), iOldProtection, &iNewProtection);
		}
	}
#else //POSIX
#endif //WIN32
}