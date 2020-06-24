#include "cbase.h"

#include "mom_run_entity.h"
#include "mom_shareddefs.h"

#include "mom_entity_run_data.h"
#include "mom_system_gamemode.h"

#ifdef GAME_DLL
#include "mom_explosive.h"
#include "momentum/mom_triggers.h"
#include "util/mom_util.h"
#endif

#include "tier0/memdbgon.h"

CMomRunEntity::CMomRunEntity()
{
#ifdef GAME_DLL
    gEntList.AddListenerEntity(this);
#endif
}

CMomRunEntity::~CMomRunEntity()
{
#ifdef GAME_DLL
    gEntList.RemoveListenerEntity(this);
    DestroyExplosives();
#endif
}

#ifndef CLIENT_DLL

void CMomRunEntity::OnZoneEnter(CTriggerZone *pTrigger)
{
    CMomRunEntityData *pData = GetRunEntData();
    // Zone-specific things first
    switch (pTrigger->GetZoneType())
    {
    case ZONE_TYPE_START:
        pData->m_bMapFinished = false;
        DestroyExplosives();
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

void CMomRunEntity::CreateTrail()
{
    RemoveTrail();

    if (!m_AppearanceData.m_bTrailEnabled)
        return;

    const auto pMyEnt = CBaseEntity::Instance(GetEntIndex());

    m_hTrailEntity = CreateEntityByName("env_spritetrail");
    m_hTrailEntity->SetAbsOrigin(pMyEnt->GetAbsOrigin());
    m_hTrailEntity->SetParent(pMyEnt);
    m_hTrailEntity->SetRenderMode(kRenderTransAdd);
    m_hTrailEntity->KeyValue("spritename", "materials/sprites/laser.vmt");
    m_hTrailEntity->KeyValue("startwidth", "9.5");
    m_hTrailEntity->KeyValue("endwidth", "1.05");
    m_hTrailEntity->KeyValue("lifetime", m_AppearanceData.m_iTrailLength);

    Color newColor;
    if (MomUtil::GetColorFromHex(m_AppearanceData.m_iTrailRGBAColorAsHex, newColor))
    {
        m_hTrailEntity->SetRenderColor(newColor.r(), newColor.g(), newColor.b(), newColor.a());
    }

    DispatchSpawn(m_hTrailEntity);
}

void CMomRunEntity::RemoveTrail()
{
    if (m_hTrailEntity.IsValid())
    {
        UTIL_RemoveImmediate(m_hTrailEntity);
        m_hTrailEntity.Term();
    }
}

void CMomRunEntity::OnEntitySpawned(CBaseEntity *pEntity)
{
    if (pEntity->GetFlags() & FL_GRENADE)
    {
        const auto pExplosive = dynamic_cast<CMomExplosive *>(pEntity);
        if (pExplosive && pExplosive->GetOwnerEntity() == CBaseEntity::Instance(GetEntIndex()))
        {
            m_vecExplosives.AddToTail(pExplosive);
        }
    }
}

void CMomRunEntity::OnEntityDeleted(CBaseEntity *pEntity)
{
    if ((pEntity->GetFlags() & FL_GRENADE) && !m_vecExplosives.IsEmpty())
    {
        const auto pExplosive = dynamic_cast<CMomExplosive *>(pEntity);
        if (pExplosive && pExplosive->GetOwnerEntity() == CBaseEntity::Instance(GetEntIndex()))
        {
            m_vecExplosives.FindAndRemove(pExplosive);
        }
    }
}

void CMomRunEntity::DestroyExplosives()
{
    FOR_EACH_VEC(m_vecExplosives, i)
    {
        const auto pExplosive = m_vecExplosives[i];
        if (pExplosive)
        {
            pExplosive->Destroy(true);
        }
    }

    m_vecExplosives.RemoveAll();
}
#endif