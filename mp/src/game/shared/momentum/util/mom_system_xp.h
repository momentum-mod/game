#pragma once

#define COSXP_MAX_LEVEL 500

class MomentumXPSystem
{
public:
    MomentumXPSystem();

    // Returns the total amount of XP required for a specific level
    int GetCosmeticXPForLevel(int level);
    // Returns the amount of XP that one particular level has until the next level
    int GetCosmeticXPInLevel(int level);

private:
    // 0 and 1 will be empty as you start at level 1 anyways.
    // XP for the max level will be xpFor(maxLvl - 1) + xpIn(maxLvl - 1) = (e.g.) xpFor(499) + xpIn(499)
    int m_arrXPForLevels[COSXP_MAX_LEVEL + 1];
    // There is no XP in the max level; you've maxed. The only thing left to do is prestige.
    int m_arrXPInLevels[COSXP_MAX_LEVEL];
};

extern MomentumXPSystem *g_pXPSystem;