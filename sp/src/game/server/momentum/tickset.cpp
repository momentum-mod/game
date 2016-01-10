#ifdef _WIN32
#include "Windows.h"
#include "Psapi.h"
#pragma comment(lib, "psapi.lib")
#elif defined (POSIX)
//#include <dlfcn.h> // ?
#endif

#include "tickset.h"
#include "tier0/platform.h"

float* TickSet::interval_per_tick = NULL;
const TickSet::Tickrate TickSet::s_DefinedRates[] = {
    { 0.015f, "66" },
    { 0.01f, "100" }
};
TickSet::Tickrate TickSet::m_trCurrent = TickSet::s_DefinedRates[TickSet::TICKRATE_66];

inline bool TickSet::DataCompare(const unsigned char* data, const unsigned char* pattern, const char* mask)
{
	for (; *mask != 0; ++data, ++pattern, ++mask)
		if (*mask == 'x' && *data != *pattern)
			return false;

	return (*mask == 0);
}

void* TickSet::FindPattern(const void* start, size_t length, const unsigned char* pattern, const char* mask)
{
	auto maskLength = strlen(mask);
	for (size_t i = 0; i <= length - maskLength; ++i)
	{
		auto addr = reinterpret_cast<const unsigned char*>(start)+i;
		if (DataCompare(addr, pattern, mask))
			return const_cast<void*>(reinterpret_cast<const void*>(addr));
	}

	return NULL;
}

bool TickSet::TickInit()
{
#ifdef _WIN32
	HMODULE handle = GetModuleHandleA("engine.dll");
	if (!handle)
		return false;	
	
	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), handle, &info, sizeof(info));

	auto moduleBase = info.lpBaseOfDll;
	auto moduleSize = info.SizeOfImage;

	unsigned char pattern[] = { 0x8B, 0x0D, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0xFF, '?', 0xD9, 0x15, '?', '?', '?', '?', 0xDD, 0x05, '?', '?', '?', '?', 0xDB, 0xF1, 0xDD, 0x05, '?', '?', '?', '?', 0x77, 0x08, 0xD9, 0xCA, 0xDB, 0xF2, 0x76, 0x1F, 0xD9, 0xCA };
	auto p = reinterpret_cast<uintptr_t>(FindPattern(moduleBase, moduleSize, pattern, "xx????????????x?xx????xx????xxxx????xxxxxxxxxx"));
	interval_per_tick = *reinterpret_cast<float**>(p + 18);
    // MOM_TODO: do we need to unload/close the module handle?
#elif defined (POSIX)
    //void *handle = dlopen("engine.so", RTLD_LAZY); // or something

    //auto p = reinterpret_cast<unsigned int>(FindPattern(base, size, pattern, "xxpatternstringxx"));//or something

#endif

	return (interval_per_tick ? true : false);
}

bool TickSet::SetTickrate(float tickrate)
{
    if (m_trCurrent.fTickRate != tickrate)
    {
        Tickrate tr;
        if (tickrate == 0.01f) tr = s_DefinedRates[TICKRATE_100];
        else if (tickrate == 0.015f) tr = s_DefinedRates[TICKRATE_66];
        else
        {
            tr.fTickRate = tickrate;
            tr.sType = "CUSTOM";
        }
        return SetTickrate(tr);
    }
    else return false;
}
