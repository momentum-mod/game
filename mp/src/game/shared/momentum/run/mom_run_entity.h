#pragma once

#include "mom_entity_run_data.h"
#include "run_stats.h"

#ifdef CLIENT_DLL
#define CMomRunEntity C_MomRunEntity
#else
class CBaseMomZoneTrigger;
class CTriggerZone;
#endif

enum RUN_ENT_TYPE
{
    RUN_ENT_PLAYER = 0, // Local player
    RUN_ENT_GHOST,      // Base number for ghost types, anything > this is a ghost type
    RUN_ENT_REPLAY,     // Replay ghost
    RUN_ENT_ONLINE,     // Online ghost
};

class CMomRunEntity
{
  public:
    DECLARE_CLASS_NOBASE(CMomRunEntity);
    CMomRunEntity();
    virtual ~CMomRunEntity();

    virtual RUN_ENT_TYPE GetEntType() = 0;

#ifndef CLIENT_DLL
    // Used by all zone triggers, moves logic to per-entity-impl.
    virtual void OnZoneEnter(CTriggerZone *pTrigger);
    virtual void OnZoneExit(CTriggerZone *pTrigger);

    // Used by limitmovement trigger
    virtual void SetButtonsEnabled(int iButtonFlags, bool bEnable) = 0;
    virtual void SetBhopEnabled(bool bEnable) = 0;
    virtual bool GetBhopEnabled() const = 0;
#else
    virtual float GetCurrentRunTime() = 0;
#endif

    // Networked shared variables between all run entities
    virtual int GetEntIndex() = 0;
    virtual CMomRunStats *GetRunStats() = 0;
    virtual CMomRunEntityData *GetRunEntData() = 0;
};
