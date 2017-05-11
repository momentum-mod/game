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
    //TODO: the rest of the data
    CMomRunStats::data m_RunStatsData;
};

struct StdReplayDataFromServer
{
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
typedef void (*DataToReplayFn)(StdDataFromServer*);