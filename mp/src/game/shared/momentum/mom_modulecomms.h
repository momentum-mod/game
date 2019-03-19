#pragma once

#include "run/run_stats.h"
#include "threadtools.h"
#include "tier1/utllinkedlist.h"
#include <utldelegate.h>

/*
 * Members of this class will be calculated server-side but updated
 * on the client every tick.
 * 
 * Note: different instances of this are used for replay entities and players!
 * Note: different functions are used to invoke a transfer on replay entities and players!
 */
struct StdDataFromServer
{
    // CMOMRunEntityData m_RunData;
    CMomRunStats::data m_RunStatsData;
};

struct StdReplayDataFromServer
{
    // CMOMRunEntityData m_RunData;
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

/*
 * 
 */
typedef void (*EventFireFn)(KeyValues *pKv);

enum EVENT_FIRE_TYPE
{
    FIRE_BOTH = 0,
    FIRE_FOREIGN_ONLY,
    FIRE_LOCAL_ONLY
};

struct EventListenerContainer
{
    ~EventListenerContainer()
    {
        m_listeners.RemoveAll();
    }
    CUtlLinkedList<CUtlDelegate<void (KeyValues*)>> m_listeners;
};

class ModuleCommunication : public CAutoGameSystem
{
public:
    ModuleCommunication();

    bool Init() OVERRIDE;
    void Shutdown() OVERRIDE;

    // The event needs to be fired and sent out,
    // The KeyValues here are deleted inside this call!
    // type can control which way the event is fired, see EVENT_FIRE_TYPE
    void FireEvent(KeyValues *pKv, EVENT_FIRE_TYPE type = FIRE_BOTH);
    void OnEvent(KeyValues *pKv); // The event has been caught by the recieving end
    void ListenForEvent(const char *pName, CUtlDelegate<void (KeyValues *)> listener);

private:
    void (*CallMeToFireEvent)(KeyValues *pKv);
    CUtlDict<int> m_dictListeners;
    CUtlVector<EventListenerContainer*> m_vecListeners;
};

extern ModuleCommunication *g_pModuleComms;