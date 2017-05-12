#pragma once

#include "cbase.h"
#include "run/run_stats.h"

/*
 * Members of this class will be calculated server-side but updated
 * on the client every tick.
 * 
 * Note: different instances of this are used for replay entities and players!
 * Note: different functions are used to invoke a transfer on replay entities and players!
 */
struct StdDataFromServer
{
    //TODO: Deprecate the usage of weapon data? (m_iLastZoom m_iShotsFired m_bResumeZoom m_iDirection)
    
    bool m_bHasPracticeMode;
    bool m_bResumeZoom;
    bool m_bDidPlayerBhop;
    bool m_bUsingCPMenu;
    int m_iCheckpointCount;
    int m_iCurrentStepCP;
    int m_iShotsFired;
    int m_iDirection;
    int m_iLastZoom;
    int m_fSliding;
    int m_iSuccessiveBhops;
    CMOMRunEntityData m_RunData;
    CMomRunStats::data m_RunStatsData;
};

struct StdReplayDataFromServer
{
    bool m_bIsPaused;
    int m_iTotalStrafes;
    int m_iTotalJumps;
    int m_nReplayButtons;
    int m_iCurrentTick;
    CMOMRunEntityData m_RunData;
    CMomRunStats::data m_RunStatsData;
};

/*
 * Function pointer type to exported 'StdDataToPlayer()'
 * Casts from pointers to function aren't technically legal,
 * but this circumvents that.
 */
typedef void (*DataToPlayerFn)(StdDataFromServer*);

/*
 * Function pointer to exported 'StdDataToReplay()'
 */
typedef void (*DataToReplayFn)(StdReplayDataFromServer*);