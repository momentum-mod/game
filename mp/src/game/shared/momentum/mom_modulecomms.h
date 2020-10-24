#pragma once

#include "tier1/utllinkedlist.h"
#include <utldelegate.h>

/*
 * Event firing function, used in the event code below
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
    void OnEvent(KeyValues *pKv); // The event has been caught by the receiving end
    int ListenForEvent(const char *pName, CUtlDelegate<void (KeyValues *)> listener);
    void RemoveListener(const char *pName, int index);

private:
    void (*CallMeToFireEvent)(KeyValues *pKv);
    CUtlDict<int> m_dictListeners;
    CUtlVector<EventListenerContainer*> m_vecListeners;
};

extern ModuleCommunication *g_pModuleComms;
