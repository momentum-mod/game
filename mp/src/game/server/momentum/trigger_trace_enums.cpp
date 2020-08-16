#include "cbase.h"
#include "trigger_trace_enums.h"
#include "mom_triggers.h"

#include "tier0/memdbgon.h"

CTeleportTriggerTraceEnum::CTeleportTriggerTraceEnum(Ray_t *pRay)
        : m_pRay(pRay), m_pTeleportEnt(nullptr)
{
}

bool CTeleportTriggerTraceEnum::EnumEntity(IHandleEntity *pHandleEntity)
{
    trace_t tr;
    // store entity that we found on the trace
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

    // Done to avoid hitting an entity that's both solid & a trigger.
    if (pEnt->IsSolid())
        return false;

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);
    
    // tr.fraction = 1.0 means the trace completed
    if (tr.fraction < 1.0f 
        && (FStrEq(pEnt->GetClassname(), "trigger_teleport")
            || FStrEq(pEnt->GetClassname(), "trigger_momentum_teleport"))) 
    {
        // arguments are initialized in the constructor of CTeleportTriggerTraceEnum
        SetTeleportEntity(pEnt);
        return false; // Stop, we hit our target.
    }
    // Continue until fraction == 1.0f
    return true;
}


CZoneTriggerTraceEnum::CZoneTriggerTraceEnum() 
    : m_pZone(nullptr) 
{
}

bool CZoneTriggerTraceEnum::EnumEntity(IHandleEntity *pHandleEntity)
{
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

    // Stop the trace if this entity is solid.
    if (pEnt->IsSolid())
    {
        return false;
    }

    m_pZone = dynamic_cast<CBaseMomentumTrigger *>(pEnt);
    if (m_pZone != nullptr)
    {
        // Found our target, stop here
        return false;
    }

    // No dice, let's keep searching
    return true;
}


CTimeTriggerTraceEnum::CTimeTriggerTraceEnum(Ray_t *pRay, Vector velocity)
        : m_vecVelocity(velocity), m_pRay(pRay) 
{
    m_flOffset = 0.0f;
}
        
bool CTimeTriggerTraceEnum::EnumEntity(IHandleEntity *pHandleEntity)
{
    trace_t tr;
    // store entity that we found on the trace
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

    // Stop the trace if this entity is solid.
    if (pEnt->IsSolid())
        return false;

    // if we aren't hitting a momentum trigger
    // the return type of EnumEntity tells the engine whether to continue enumerating future entities
    // or not.
    if (Q_strnicmp(pEnt->GetClassname(), "trigger_momentum_", 17) == 1)
        return false;

    // In this case, we want to continue in case we hit another type of trigger.

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);
    if (tr.fraction > 0.0f)
    {
        m_flOffset = tr.startpos.DistTo(tr.endpos) / m_vecVelocity.Length();

        // Account for slowmotion/timescale
        m_flOffset /= gpGlobals->interval_per_tick / gpGlobals->frametime;
        return true; // We hit our target
    }

    return false;
}
