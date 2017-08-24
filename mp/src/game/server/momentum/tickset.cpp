#ifdef _WIN32
#include "Windows.h"
#include "Psapi.h"
#pragma comment(lib, "psapi.lib")
#elif defined (POSIX)
#include "util/os_utils.h"
#endif

#include "tickset.h"
#include "tier0/platform.h"

float* TickSet::interval_per_tick = nullptr;
bool TickSet::m_bInGameUpdate = false;
const Tickrate TickSet::s_DefinedRates[] = {
    { 0.015f, "66" },
    { 0.01f, "100" }
};
Tickrate TickSet::m_trCurrent = s_DefinedRates[TICKRATE_66];

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

	int result = GetModuleInformation_OSX("engine.dylib", &base, &length);
	if (result)
		return false;
	
	//MOM_TODO: FIND BIT PATTERN LOL
	/*
	unsigned char pattern[] = {};
	auto addr = reinterpret_cast<uintptr_t>(FindPattern(base, length, pattern, ""));
	ConColorMsg(2, Color(255, 255, 0, 255),"Found pattern!\n");

	if (addr)
		interval_per_tick = *(float**)((char*)addr + 2);
	*/
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

    case MOMGM_UNKNOWN:
        if (m_bInGameUpdate) // If the user's updating this, ignore the end of map setting
            return false;

    case MOMGM_SURF:
    default:
        return SetTickrate(s_DefinedRates[TICKRATE_66]);
    }
}

bool TickSet::SetTickrate(float tickrate)
{
    if (!g_pMomentumUtil->FloatEquals(m_trCurrent.fTickRate, tickrate))
    {
        Tickrate tr;
        if (g_pMomentumUtil->FloatEquals(tickrate, 0.01f)) tr = s_DefinedRates[TICKRATE_100];
        else if (g_pMomentumUtil->FloatEquals(tickrate, 0.015f)) tr = s_DefinedRates[TICKRATE_66];
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
    if (m_bInGameUpdate)
    {
        m_bInGameUpdate = false;
        return false;
    }

    if (trNew == m_trCurrent) return false;

    if (interval_per_tick)
    {
        DevLog("Testing: %f\n", trNew.fTickRate);
        *interval_per_tick = trNew.fTickRate;
        gpGlobals->interval_per_tick = *interval_per_tick;
        DevLog("Should have set the tickrate to %f\n", *interval_per_tick);
        m_trCurrent = trNew;
        auto pPlayer = UTIL_GetLocalPlayer();
        if (pPlayer)
        {
            m_bInGameUpdate = true;
            engine->ClientCommand(pPlayer->edict(), "reload");
        }
        return true;
    }
    return false;
}

static void onTickRateChange(IConVar *var, const char* pOldValue, float fOldValue)
{
    ConVarRef tr(var);
    float toCheck = tr.GetFloat();
    if (g_pMomentumUtil->FloatEquals(toCheck, fOldValue)) return;
	//MOM_TODO: Re-implement the bound
	
	/*
    if (toCheck < 0.01f || toCheck > 0.015f)
    {
        Warning("Cannot set a tickrate any lower than 66 or higher than 100!\n");
        var->SetValue(((ConVar*) var)->GetDefault());
        return;
    }*/
    bool result = TickSet::SetTickrate(toCheck);
    if (result)
    {
        Msg("Successfully changed the tickrate to %f!\n", toCheck);
        gpGlobals->interval_per_tick = toCheck;
    }
    else Warning("Failed to hook interval per tick, cannot set tick rate!\n");
}

static ConVar tickRate("sv_tickrate", "0.015", FCVAR_CHEAT | FCVAR_HIDDEN, "Changes the tickrate of the game. \
					   Formatted as interval per tick, i.e 100 tickrate = 0.01", onTickRateChange);