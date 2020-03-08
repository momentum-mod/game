#pragma once

struct Tickrate
{
    float fTickRate;
    const char* sType;

    Tickrate()
    {
        fTickRate = 0.0f;
        sType = nullptr;
    }
    Tickrate(float f, const char * type)
    {
        fTickRate = f;
        sType = type;
    }
    Tickrate& operator =(const Tickrate &other)
    {
        this->fTickRate = other.fTickRate;
        this->sType = other.sType;
        return *this;
    }
    bool operator ==(const Tickrate &other) const
    {
        return (CloseEnough(other.fTickRate, fTickRate, FLT_EPSILON)
            && !Q_strcmp(other.sType, sType));
    }
};

class TickSet {
public:
    static const Tickrate s_DefinedRates[];

    enum
    {
        TICKRATE_66 = 0,
        TICKRATE_100 = 1    
    };

    static bool TickInit();

    static Tickrate GetCurrentTickrate() { return (m_trCurrent.fTickRate > 0.0f ? m_trCurrent : s_DefinedRates[TICKRATE_66]); }

    static bool SetTickrate(Tickrate trNew);
    static bool SetTickrate(float);
    static float GetTickrate() { return *interval_per_tick; }

private:
    static float *interval_per_tick;

    static Tickrate m_trCurrent;
    static bool m_bInGameUpdate;
};

extern ConVar sv_interval_per_tick;
