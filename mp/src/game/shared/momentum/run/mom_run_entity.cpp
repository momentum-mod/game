#include "cbase.h"

#include "mom_run_entity.h"
#include "mom_shareddefs.h"

#include "mom_entity_run_data.h"

#ifndef CLIENT_DLL
#include "momentum/mom_triggers.h"
#else

#endif

#include "tier0/memdbgon.h"

CMomRunEntity::CMomRunEntity() {}

CMomRunEntity::~CMomRunEntity() {}

#ifndef CLIENT_DLL

void CMomRunEntity::OnZoneEnter(CTriggerZone *pTrigger)
{
    CMomRunEntityData *pData = GetRunEntData();
    // Zone-specific things first
    switch (pTrigger->GetZoneType())
    {
    case ZONE_TYPE_START:
        break;
    case ZONE_TYPE_STOP:
        pData->m_iOldTrack = pData->m_iCurrentTrack;
        pData->m_iOldZone = pData->m_iCurrentZone;
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
        pEvent->SetInt("ent", GetEntIndex());
        pEvent->SetInt("zone_ent", pTrigger->entindex());
        pEvent->SetInt("num", pTrigger->GetZoneNumber());
        gameeventmanager->FireEvent(pEvent);
    }
}

void CMomRunEntity::OnZoneExit(CTriggerZone *pTrigger)
{
    CMomRunEntityData *pData = GetRunEntData();

    // Zone-specific things first
    switch (pTrigger->GetZoneType())
    {
    case ZONE_TYPE_START:
        break;
    case ZONE_TYPE_STOP:
        pData->m_iCurrentTrack = pData->m_iOldTrack;
        pData->m_iCurrentZone = pData->m_iOldZone;
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
        pEvent->SetInt("ent", GetEntIndex());
        pEvent->SetInt("zone_ent", pTrigger->entindex());
        pEvent->SetInt("num", pTrigger->GetZoneNumber());
        gameeventmanager->FireEvent(pEvent);
    }
}

#endif