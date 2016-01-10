#ifndef TICKSET_H
#define TICKSET_H

#include "cbase.h"

class TickSet {
public:
    
    struct Tickrate
    {
        float fTickRate;
        const char* sType;

        Tickrate()
        {
            fTickRate = 0.0f;
            sType = '\0';
        }
        Tickrate(float f, const char * type)
        {
            fTickRate = f;
            sType = type;
        }
    };

    static const Tickrate s_DefinedRates[];

    enum
    {
        TICKRATE_66 = 0,
        TICKRATE_100 = 1    
    };

    static Tickrate GetCurrentTickrate() { return (m_trCurrent.fTickRate > 0.0f ? m_trCurrent : s_DefinedRates[TICKRATE_66]); }
	static bool TickInit();
    static bool SetTickrate(Tickrate trNew)
    {
        if (interval_per_tick)
        {
            *interval_per_tick = trNew.fTickRate;
            m_trCurrent = trNew;
            return true;
        }
        return false;
    }
	static bool SetTickrate(float);
    static float GetTickrate() { return *interval_per_tick; }

private:
	static inline bool DataCompare(const unsigned char*, const unsigned char*, const char*);
	static void *FindPattern(const void*, size_t, const unsigned char*, const char*);
	static float *interval_per_tick;
    static Tickrate m_trCurrent;
};

#endif // TICKSET_H
