#pragma once

#include "cbase.h"
#include "run/run_stats.h"
#include "threadtools.h"
/*
 * Members of this class will be calculated server-side but updated
 * on the client every tick.
 * 
 * Note: different instances of this are used for replay entities and players!
 * Note: different functions are used to invoke a transfer on replay entities and players!
 */
struct StdDataFromServer
{
    //MOM_TODO: Deprecate the usage of weapon data? (m_iLastZoom m_iShotsFired m_bResumeZoom m_iDirection)
    
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
#ifdef CLIENT_DLL
//Forward Decls

class C_MomentumPlayer;
class C_MomentumReplayGhostEntity;
void FetchStdData(C_MomentumPlayer *pPlayer);
void FetchStdReplayData(C_MomentumReplayGhostEntity *pGhost);

#endif

/*
 * Buffer-like objects that exist to make the goal of no-boilerplate thread safety possible.
 * An intermediate object like this allows the data to be moved to an area that isn't locked
 * by default like it would be if it was coppied straight from the server player to client player.
 * 
 * In other words, the server doesn't force the client to take in data when the server is ready;
 * the client can decide when it is ready and then the server can do that too.
 */
struct StdDataBuffer : StdDataFromServer
{
    CThreadMutex _mutex;
    bool m_bWritten;
};

struct StdReplayDataBuffer : StdReplayDataFromServer
{
    CThreadMutex _mutex;
    bool m_bWritten;
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