#include "cbase.h"

#include "mom_modulecomms.h"
#include "util/os_utils.h"

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
// The server will hook into this function to fire the event on the client (server -> client)
DLL_EXPORT void FireEventFromServer(KeyValues *pKv)
{
    g_pModuleComms->OnEvent(pKv);
}
#else
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

int ModuleCommunication::ListenForEvent(const char* pName, CUtlDelegate<void (KeyValues*)> listener)
{
    const auto found = m_dictListeners.Find(pName);
    if (m_dictListeners.IsValidIndex(found))
    {
        const auto indx = m_dictListeners[found];
        return m_vecListeners[indx]->m_listeners.AddToTail(listener);
    }

    EventListenerContainer *pContainer = new EventListenerContainer;
    const auto toRet = pContainer->m_listeners.AddToTail(listener);
    const auto indx = m_vecListeners.AddToTail(pContainer);
    m_dictListeners.Insert(pName, indx);
    return toRet;
}

void ModuleCommunication::RemoveListener(const char *pName, int index)
{
    // Find the event listeners for this particular event
    const auto found = m_dictListeners.Find(pName);
    if (m_dictListeners.IsValidIndex(found))
    {
        const auto indx = m_dictListeners[found];
        const auto pContainer = m_vecListeners[indx];
        if (index < pContainer->m_listeners.MaxElementIndex())
        {
            pContainer->m_listeners.Remove(index);
        }
    }
}


void ModuleCommunication::OnEvent(KeyValues* pKv)
{
    // Find the event listeners for this particular event
    const auto found = m_dictListeners.Find(pKv->GetName());
    if (m_dictListeners.IsValidIndex(found))
    {
        const auto indx = m_dictListeners[found];
        const auto pContainer = m_vecListeners[indx];
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