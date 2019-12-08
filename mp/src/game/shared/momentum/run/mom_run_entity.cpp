#include "cbase.h"

#include "mom_run_entity.h"
#include "mom_shareddefs.h"

#include "mom_entity_run_data.h"

#ifndef CLIENT_DLL
#include "momentum/mom_triggers.h"
#include "util/mom_util.h"
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
        pData->m_bMapFinished = false;
        break;
    case ZONE_TYPE_STOP:
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
        break;
    case ZONE_TYPE_CHECKPOINT:
        break;
    case ZONE_TYPE_STAGE:
    default:
        break;
    }

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

bool CMomRunEntity::SetAppearanceData(const AppearanceData_t &newApp, bool bForceUpdate)
{
    bool bSomethingChanged = false;
    if (m_AppearanceData.m_iBodyGroup != newApp.m_iBodyGroup || bForceUpdate)
    {
        AppearanceBodygroupChanged(newApp);
        bSomethingChanged = true;
    }

    if (m_AppearanceData.m_iModelRGBAColorAsHex != newApp.m_iModelRGBAColorAsHex || bForceUpdate)
    {
        AppearanceModelColorChanged(newApp);
        bSomethingChanged = true;
    }

    if (m_AppearanceData.m_iTrailRGBAColorAsHex != newApp.m_iTrailRGBAColorAsHex ||
        m_AppearanceData.m_iTrailLength != newApp.m_iTrailLength ||
        m_AppearanceData.m_bTrailEnabled != newApp.m_bTrailEnabled || bForceUpdate)
    {
        AppearanceTrailChanged(newApp);
        bSomethingChanged = true;
    }

    if (m_AppearanceData.m_bFlashlightEnabled != newApp.m_bFlashlightEnabled || bForceUpdate)
    {
        AppearanceFlashlightChanged(newApp);
        bSomethingChanged = true;
    }

    return bSomethingChanged;
}

void CMomRunEntity::AppearanceTrailChanged(const AppearanceData_t &newApp)
{
    m_AppearanceData.m_iTrailLength = newApp.m_iTrailLength;
    m_AppearanceData.m_iTrailRGBAColorAsHex = newApp.m_iTrailRGBAColorAsHex;
    m_AppearanceData.m_bTrailEnabled = newApp.m_bTrailEnabled;
    m_AppearanceData.ValidateValues();

    CreateTrail();
}

void CMomRunEntity::AppearanceBodygroupChanged(const AppearanceData_t &newApp)
{
    m_AppearanceData.m_iBodyGroup = newApp.m_iBodyGroup;
    m_AppearanceData.ValidateValues();

    const auto pBaseAnimating = static_cast<CBaseAnimating*>(CBaseEntity::Instance(GetEntIndex()));
    if (pBaseAnimating)
    {
        pBaseAnimating->SetBodygroup(1, m_AppearanceData.m_iBodyGroup);
    }
}

void CMomRunEntity::AppearanceModelColorChanged(const AppearanceData_t &newApp)
{
    m_AppearanceData.m_iModelRGBAColorAsHex = newApp.m_iModelRGBAColorAsHex;

    const auto pBaseEnt = CBaseEntity::Instance(GetEntIndex());
    if (pBaseEnt)
    {
        Color newColor;
        if (MomUtil::GetColorFromHex(m_AppearanceData.m_iModelRGBAColorAsHex, newColor))
            pBaseEnt->SetRenderColor(newColor.r(), newColor.g(), newColor.b(), newColor.a());
    }
}

void CMomRunEntity::AppearanceFlashlightChanged(const AppearanceData_t &newApp)
{
    m_AppearanceData.m_bFlashlightEnabled = newApp.m_bFlashlightEnabled;
}
#endif