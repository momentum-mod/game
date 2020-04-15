#pragma once

#include "mom_entity_run_data.h"
#include "run_stats.h"
#include "mom_ghostdefs.h"

#ifdef CLIENT_DLL
#define CMomRunEntity C_MomRunEntity
#else
class CBaseMomZoneTrigger;
class CTriggerZone;
class CMomExplosive;
#endif

enum RUN_ENT_TYPE
{
    RUN_ENT_PLAYER = 0, // Local player
    RUN_ENT_GHOST,      // Base number for ghost types, anything > this is a ghost type
    RUN_ENT_REPLAY,     // Replay ghost
    RUN_ENT_ONLINE,     // Online ghost
};

class CMomRunEntity
#ifdef GAME_DLL
    : public IEntityListener
#endif
{
 public:
    DECLARE_CLASS_NOBASE(CMomRunEntity);
    CMomRunEntity();
    virtual ~CMomRunEntity();

    virtual RUN_ENT_TYPE GetEntType() = 0;

#ifdef GAME_DLL
    // Used by all zone triggers, moves logic to per-entity-impl.
    virtual void OnZoneEnter(CTriggerZone *pTrigger);
    virtual void OnZoneExit(CTriggerZone *pTrigger);

    // Used by limitmovement trigger
    virtual void SetButtonsEnabled(int iButtonFlags, bool bEnable) = 0;
    virtual void SetBhopEnabled(bool bEnable) = 0;
    virtual bool GetBhopEnabled() const = 0;

    virtual void CreateTrail();
    virtual void RemoveTrail();
    // Sets the appearance data but only if something has changed.
    // Returns true if something has changed, else false
    virtual bool SetAppearanceData(const AppearanceData_t &newApp, bool bForceUpdate);
    virtual AppearanceData_t *GetAppearanceData() { return &m_AppearanceData; }
    virtual void AppearanceBodygroupChanged(const AppearanceData_t &newApp);
    virtual void AppearanceModelColorChanged(const AppearanceData_t &newApp);
    virtual void AppearanceTrailChanged(const AppearanceData_t &newApp);
    virtual void AppearanceFlashlightChanged(const AppearanceData_t &newApp);

    // IEntityListener
    void OnEntitySpawned(CBaseEntity *pEntity) override;
    void OnEntityDeleted(CBaseEntity *pEntity) override;

    void DestroyExplosives();
#else
    virtual float GetCurrentRunTime() = 0;
#endif

    // Networked shared variables between all run entities
    virtual int GetEntIndex() = 0;
    virtual CMomRunStats *GetRunStats() = 0;
    virtual CMomRunEntityData *GetRunEntData() = 0;
    virtual uint64 GetSteamID() = 0;

#ifdef GAME_DLL
protected:
    EHANDLE m_hTrailEntity;
    AppearanceData_t m_AppearanceData;
    CUtlVector<CMomExplosive*> m_vecExplosives;
#endif
};