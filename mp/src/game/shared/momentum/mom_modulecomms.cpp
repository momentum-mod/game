#include "cbase.h"
#include "run/run_stats.h"
#include "mom_modulecomms.h"
#include "util/os_utils.h"

#ifdef CLIENT_DLL

#include "c_mom_player.h"
#include "c_mom_replay_entity.h"
StdDataBuffer g_MomServerDataBuf;
StdReplayDataBuffer g_MomReplayDataBuf;

DLL_EXPORT void StdDataToPlayer(StdDataFromServer *from)
{
    CAutoLock guard(g_MomServerDataBuf._mutex);

    g_MomServerDataBuf.m_bWritten = true;
    memcpy(&g_MomServerDataBuf, from, sizeof(StdDataFromServer));
}

DLL_EXPORT void StdDataToReplay(StdReplayDataFromServer *from)
{
    CAutoLock guard(g_MomServerDataBuf._mutex);

    g_MomReplayDataBuf.m_bWritten = true;
    memcpy(&g_MomReplayDataBuf, from, sizeof(StdReplayDataFromServer));
}

void FetchStdData(C_MomentumPlayer *pPlayer)
{
    if(pPlayer)
    {
        CAutoLock guard(g_MomServerDataBuf._mutex);

        if (!g_MomServerDataBuf.m_bWritten)
            return;
        
        g_MomServerDataBuf.m_bWritten = false;
        memcpy(&pPlayer->m_SrvData, &g_MomServerDataBuf, sizeof(StdDataFromServer));
    }
}

void FetchStdReplayData(C_MomentumReplayGhostEntity *pGhost)
{
    if (pGhost)
    {
        CAutoLock guard(g_MomServerDataBuf._mutex);

        if (!g_MomReplayDataBuf.m_bWritten)
            return;
        
        g_MomReplayDataBuf.m_bWritten = false;
        memcpy(&pGhost->m_SrvData, &g_MomReplayDataBuf, sizeof(StdReplayDataFromServer));
    }
}


// The server will hook into this function to fire the event on the client (server -> client)
DLL_EXPORT void FireEventFromServer(KeyValues *pKv)
{
    g_pModuleComms->OnEvent(pKv);
}

#else //Is not CLIENT_DLL

// The client hooks into this function to pass an event to the server (client -> server)
DLL_EXPORT void FireEventFromClient(KeyValues *pKv)
{
    g_pModuleComms->OnEvent(pKv);
}

#endif //CLIENT_DLL

ModuleCommunication::ModuleCommunication() : CAutoGameSystem("ModuleCommunication"), CallMeToFireEvent(nullptr)
{
}

bool ModuleCommunication::Init()
{
    CallMeToFireEvent = (EventFireFn) (GetProcAddress(
#ifdef CLIENT_DLL
        GetModuleHandle(SERVER_DLL_NAME), "FireEventFromClient" // client -> server
#else
        GetModuleHandle(CLIENT_DLL_NAME), "FireEventFromServer" // server -> client
#endif
    ));

    return true;
}

void ModuleCommunication::Shutdown()
{
    m_dictListeners.RemoveAll();
    m_vecListeners.PurgeAndDeleteElements();
}


void ModuleCommunication::FireEvent(KeyValues* pKv, EVENT_FIRE_TYPE type /* = FIRE_BOTH */)
{
    KeyValues *pCopy = type == FIRE_BOTH ? pKv->MakeCopy() : nullptr;

    // This fires across the DLL boundary
    if (CallMeToFireEvent && (type == FIRE_BOTH || type == FIRE_FOREIGN_ONLY))
        CallMeToFireEvent(pKv); // pKv->deleteThis is called in here

    // This fires it for the DLL we're currently on, allowing "local" listeners for events
    if (type == FIRE_BOTH || type == FIRE_LOCAL_ONLY)
        OnEvent(type == FIRE_BOTH ? pCopy : pKv); //pCopy->deleteThis is called in here
}

void ModuleCommunication::ListenForEvent(const char* pName, CUtlDelegate<void (KeyValues*)> listener)
{
    int found = m_dictListeners.Find(pName);
    if (m_dictListeners.IsValidIndex(found))
    {
        int indx = m_dictListeners[found];
        m_vecListeners[indx]->m_listeners.AddToTail(listener);
    }
    else
    {
        EventListenerContainer *pContainer = new EventListenerContainer;
        pContainer->m_listeners.AddToTail(listener);
        int indx = m_vecListeners.AddToTail(pContainer);
        m_dictListeners.Insert(pName, indx);
    }
}


void ModuleCommunication::OnEvent(KeyValues* pKv)
{
    // Find the event listeners for this particular event
    int found = m_dictListeners.Find(pKv->GetName());
    if (m_dictListeners.IsValidIndex(found))
    {
        int indx = m_dictListeners[found];
        EventListenerContainer *pContainer = m_vecListeners[indx];
        FOR_EACH_LL(pContainer->m_listeners, i)
        {
            pContainer->m_listeners[i](pKv);
        }
    }
    else
    {
        Warning("Trying to fire modulecom event %s with no registered listeners!\n", pKv->GetName());
    }

    pKv->deleteThis();
}

//Expose this to the DLL
static ModuleCommunication mod;
ModuleCommunication *g_pModuleComms = &mod;