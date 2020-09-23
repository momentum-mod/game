#include "cbase.h"

#include "mom_system_xp.h"

#include "tier0/memdbgon.h"

#define COSXP_STARTING_VALUE 20000
#define COSXP_linearScaleBaseIncrease 1000
#define COSXP_linearScaleInterval 10.0
#define COSXP_linearScaleIntervalMultiplier 1.0
#define COSXP_staticScaleStart 101
#define COSXP_staticScaleBaseMultiplier 1.5
#define COSXP_staticScaleInterval 25
#define COSXP_staticScaleIntervalMultiplier 0.5

MomentumXPSystem::MomentumXPSystem()
{
    // Looping to MaxLvl + 1 because we're filling the "for" array. See header for clarification on why "for" needs it but "in" does not.
    for (auto i = 1; i < COSXP_MAX_LEVEL + 1; i++)
    {
        if (i < COSXP_MAX_LEVEL)
            m_arrXPInLevels[i] = GetCosmeticXPInLevel(i);

        if (i > 1)
            m_arrXPForLevels[i] = m_arrXPForLevels[i - 1] + m_arrXPInLevels[i - 1];
    }
}

int MomentumXPSystem::GetCosmeticXPForLevel(int level)
{
    if (level < 1 || level > COSXP_MAX_LEVEL)
    {
        Warning("Trying to get XP for level %i which is invalid!\n", level);
        return -1;
    }

    return m_arrXPForLevels[level];
}

int MomentumXPSystem::GetCosmeticXPInLevel(int level)
{
    if (level < 1 || level >= COSXP_MAX_LEVEL)
        return -1;

    if (level < COSXP_staticScaleStart)
    {
        return COSXP_STARTING_VALUE + COSXP_linearScaleBaseIncrease * level * int(COSXP_linearScaleIntervalMultiplier * ceil((double)level / COSXP_linearScaleInterval));
    }

    return COSXP_linearScaleBaseIncrease * (COSXP_staticScaleStart - 1) * (COSXP_linearScaleIntervalMultiplier * ceil((COSXP_staticScaleStart - 1) / COSXP_linearScaleInterval)) *
        (level >= COSXP_staticScaleStart + COSXP_staticScaleInterval ? COSXP_staticScaleBaseMultiplier + floor(double(level - COSXP_staticScaleStart) / (double)COSXP_staticScaleInterval) * COSXP_staticScaleIntervalMultiplier : COSXP_staticScaleBaseMultiplier);
}

static MomentumXPSystem s_MomentumXPSystem;
MomentumXPSystem *g_pXPSystem = &s_MomentumXPSystem;