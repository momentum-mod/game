#include "cbase.h"

#include "mom_run_entity.h"
#include "mom_shareddefs.h"

#ifndef CLIENT_DLL
#include "momentum/mom_triggers.h"
#else

#endif

#include "tier0/memdbgon.h"

CMomRunEntity::CMomRunEntity()
{
}

CMomRunEntity::~CMomRunEntity()
{
}


#ifndef CLIENT_DLL

void CMomRunEntity::OnZoneEnter(CTriggerZone *pTrigger, CBaseEntity *pEnt)
{
    CMomRunEntityData *pData = GetRunEntData();
    // Zone-specific things first
    switch (pTrigger->GetZoneType())
    {
    case ZONE_TYPE_START:
        pData->m_bMapFinished = false;
        pData->m_bTimerRunning = false;
        break;
    case ZONE_TYPE_STOP:
        pData->m_iOldZone = pData->m_iCurrentZone;
        pData->m_iOldBonusZone = pData->m_iBonusZone;

        break;

    case ZONE_TYPE_CHECKPOINT:
        break;
    case ZONE_TYPE_STAGE:

    default:
        break;
    }

    pData->m_bIsInZone = true;
    pData->m_iCurrentZone = pTrigger->GetZoneNumber();

    IGameEvent *pEvent = gameeventmanager->CreateEvent("zone_enter");
    if (pEvent)
    {
        pEvent->SetInt("ent", pEnt->entindex());
        pEvent->SetInt("zone_ent", pTrigger->entindex());
        pEvent->SetInt("num", pTrigger->GetZoneNumber());
        gameeventmanager->FireEvent(pEvent);
    }
}

void CMomRunEntity::OnZoneExit(CTriggerZone *pTrigger, CBaseEntity *pEnt)
{
    CMomRunEntityData *pData = GetRunEntData();

    // Needs to be done because OnStartTouch is called before OnEndTouch, when 
    // teleporting out of this zone (saveloc, spectating, etc)
    if (pData->m_iCurrentZone == pTrigger->GetZoneNumber())
    {
        // Zone-specific things first
        switch (pTrigger->GetZoneType())
        {
        case ZONE_TYPE_START:
            break;
        case ZONE_TYPE_STOP:
            pData->m_iCurrentZone = pData->m_iOldZone;
            pData->m_iBonusZone = pData->m_iOldBonusZone;
            break;

        case ZONE_TYPE_CHECKPOINT:
            break;
        case ZONE_TYPE_STAGE:

        default:
            break;
        }


        pData->m_bMapFinished = false;
        pData->m_bIsInZone = false;

        IGameEvent *pEvent = gameeventmanager->CreateEvent("zone_exit");
        if (pEvent)
        {
            pEvent->SetInt("ent", pEnt->entindex());
            pEvent->SetInt("zone_ent", pTrigger->entindex());
            pEvent->SetInt("num", pTrigger->GetZoneNumber());
            gameeventmanager->FireEvent(pEvent);
        }
    }
}

#endif