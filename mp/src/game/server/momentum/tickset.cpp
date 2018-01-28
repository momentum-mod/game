#ifdef _WIN32
#include "Windows.h"
#include "Psapi.h"
#pragma comment(lib, "psapi.lib")
#elif defined (POSIX)
#include "util/os_utils.h"
#endif

#include "tickset.h"
#include "mom_shareddefs.h"
#include "tier0/platform.h"

float* TickSet::interval_per_tick = nullptr;
const Tickrate TickSet::s_DefinedRates[] = {
    { 0.015f, "66" },
    { 0.01f, "100" }
};
Tickrate TickSet::m_trCurrent = s_DefinedRates[TICKRATE_66];
bool TickSet::m_bInGameUpdate = false;

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

    return nullptr;
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

    unsigned char pattern[] = { 0x8B, 0x0D, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0xFF, '?', 0xD9, 0x15, '?', '?',
		'?', '?', 0xDD, 0x05, '?', '?', '?', '?', 0xDB, 0xF1, 0xDD, 0x05, '?', '?', '?', '?', 0x77, 0x08, 0xD9, 0xCA, 0xDB, 0xF2, 0x76, 0x1F, 0xD9, 0xCA };
    auto p = reinterpret_cast<uintptr_t>(FindPattern(moduleBase, moduleSize, pattern, "xx????????????x?xx????xx????xxxx????xxxxxxxxxx"));
    if (p)
        interval_per_tick = *reinterpret_cast<float**>(p + 18);
	
#elif defined (__linux__)
	void *base;
	size_t length;

	if (GetModuleInformation_LINUX("engine.so", &base, &length))
		return false;

    // mov ds:interval_per_tick, 3C75C28Fh         <-- float for 0.015
    unsigned char pattern[] = { 0xC7,0x05, '?','?','?','?', 0x8F,0xC2,0x75,0x3C, 0xE8 };
    void* addr = FindPattern(base, length, pattern, "xx????xxxxx");
    if (addr)
        interval_per_tick = *(float**)(addr + 2); //MOM_TODO: fix pointer arithmetic on void pointer?

#elif defined (OSX)
	void *base;
	size_t length;

	if (GetModuleInformation_OSX("engine.dylib", &base, &length))
		return false;
	
	if (length == 12581936) //magic engine.dylib file size as of august 2017
	{
		interval_per_tick = reinterpret_cast<float*>((char*)base + 0x7DC120); //use offset since it's quicker than searching
		printf("engine.dylib not updated. using offset! address: %#08x\n", interval_per_tick);
	}
	else //valve updated engine, try to use search pattern...
	{
		unsigned char pattern[] = {0x8F, 0xC2, 0x75, 0x3C, 0x78, '?', '?', 0x0C, 0x6C, '?', '?', '?', 0x01, 0x00};
		auto addr = reinterpret_cast<uintptr_t>(FindPattern(base, length, pattern, "xxxxx??xx???xx"));
		if (addr)
		{
			interval_per_tick = reinterpret_cast<float*>(addr);
			printf("Found interval_per_tick using search! address: %#08x\n", interval_per_tick);
		}
	}
#endif

    return (interval_per_tick ? true : false);
}


bool TickSet::SetTickrate(int gameMode)
{
    switch (gameMode)
    {
    case MOMGM_BHOP:
    case MOMGM_SCROLL:
        //MOM_TODO: add more gamemodes
        return SetTickrate(s_DefinedRates[TICKRATE_100]);
    case MOMGM_SURF:
    default:
        return SetTickrate(s_DefinedRates[TICKRATE_66]);
    }
}

bool TickSet::SetTickrate(float tickrate)
{
    if (!CloseEnough(m_trCurrent.fTickRate, tickrate, FLT_EPSILON))
    {
        Tickrate tr;
        if (CloseEnough(tickrate, 0.01f, FLT_EPSILON)) tr = s_DefinedRates[TICKRATE_100];
        else if (CloseEnough(tickrate, 0.015f, FLT_EPSILON)) tr = s_DefinedRates[TICKRATE_66];
        else
        {
            tr.fTickRate = tickrate;
            tr.sType = "CUSTOM";
        }
        return SetTickrate(tr);
    }
    
    return false;
}

bool TickSet::SetTickrate(Tickrate trNew)
{
    if (trNew == m_trCurrent)
    {
        DevLog("Tickrate not changed: new == current\n");
        return false;
    }

    if (interval_per_tick)
    {
        *interval_per_tick = trNew.fTickRate;
        gpGlobals->interval_per_tick = *interval_per_tick;
        m_trCurrent = trNew;
        auto pPlayer = UTIL_GetLocalPlayer();
        if (pPlayer)
        {
            engine->ClientCommand(pPlayer->edict(), "reload");
        }
        DevLog("Interval per tick set to %f\n", trNew.fTickRate);
        return true;
    }
    Warning("Failed to set tickrate: bad hook\n");
    return false;
}

static void onTickRateChange(IConVar *var, const char* pOldValue, float fOldValue)
{
    ConVarRef tr(var);
    float tickrate = tr.GetFloat();
    if (CloseEnough(tickrate, TickSet::GetTickrate(), FLT_EPSILON)) return;
	//MOM_TODO: Re-implement the bound
	
	/*
    if (toCheck < 0.01f || toCheck > 0.015f)
    {
        Warning("Cannot set a tickrate any lower than 66 or higher than 100!\n");
        var->SetValue(((ConVar*) var)->GetDefault());
        return;
    }*/
    TickSet::SetTickrate(tickrate);
}

static ConVar tickRate("sv_tickrate", "0.015", FCVAR_CHEAT,
					   "Changes the tickrate of the game. Formatted as interval per tick, i.e 100 tickrate = 0.01", onTickRateChange);
