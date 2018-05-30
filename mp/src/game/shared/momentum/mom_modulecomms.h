#pragma once

#include "cbase.h"
#include "run/run_stats.h"
#include "threadtools.h"
#include "run/mom_entity_run_data.h"
#include "run/mom_slide_data.h"
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
    //MOM_TODO: Deprecate the usage of weapon data? (m_iLastZoom m_iShotsFired m_bResumeZoom m_iDirection)
    
    bool m_bHasPracticeMode;
    bool m_bResumeZoom;
    bool m_bDidPlayerBhop;
    bool m_bPreventPlayerBhop;
    int m_iLandTick; // Tick at which the player landed on the ground
    int m_iShotsFired;
    int m_iDirection;
    int m_iLastZoom;
    int m_iSuccessiveBhops;
    CMOMRunEntityData m_RunData;
    CMomPlayerSlideData m_SlideData;
    CMomRunStats::data m_RunStatsData;
};

struct StdReplayDataFromServer
{
    bool m_bIsPaused;
    int m_iTotalStrafes;
    int m_iTotalJumps;
    int m_nReplayButtons;
    int m_iCurrentTick;
    float m_flTickRate;
    int m_iTotalTimeTicks; // The total tick count of the playback
    char m_pszPlayerName[MAX_PLAYER_NAME_LENGTH];
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

/*
 * 
 */
typedef void (*EventFireFn)(KeyValues *pKv);

/*
abstract_class EventListener
{
public:
    virtual void FireEvent(KeyValues *pKv) = 0;
};
*/

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
    // fireLocal is controlling whether the event is fired on the same DLL it is created on
    void FireEvent(KeyValues *pKv, bool fireLocal = true);
    void OnEvent(KeyValues *pKv); // The event has been caught by the recieving end
    void ListenForEvent(const char *pName, CUtlDelegate<void (KeyValues *)> listener);

private:
    void (*CallMeToFireEvent)(KeyValues *pKv);
    CUtlDict<int> m_dictListeners;
    CUtlVector<EventListenerContainer*> m_vecListeners;
};

extern ModuleCommunication *g_pModuleComms;