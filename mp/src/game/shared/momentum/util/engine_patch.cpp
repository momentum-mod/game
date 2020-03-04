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

void* EnginePatch::moduleBase = nullptr;
size_t EnginePatch::moduleSize;

inline bool EnginePatch::DataCompare(const unsigned char* data, const unsigned char* pattern, const char* mask)
{
	for (; *mask != 0; ++data, ++pattern, ++mask)
		if (*mask == 'x' && *data != *pattern)
			return false;

	return (*mask == 0);
}

void* EnginePatch::FindPattern(const unsigned char* pattern, const char* mask, size_t offset = 0)
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

void EnginePatch::InitPatches()
{
	// Get the engine's base address and size
#ifdef _WIN32
	HMODULE handle = GetModuleHandleA("engine.dll");
	if (!handle)
		return;

	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), handle, &info, sizeof(info));

	moduleBase = info.lpBaseOfDll;
	moduleSize = info.SizeOfImage;
#else //POSIX
	if (GetModuleInformation(ENGINE_DLL_NAME, &moduleBase, &moduleSize))
		return;
#endif //WIN32

#ifdef _WIN32
#else //POSIX
#endif //WIN32
}