#pragma once

#include "mom_ghostdefs.h"
#include "run/mom_run_entity.h"

#define GHOST_MODEL "models/player/player_shape_base.mdl"

class CMomentumPlayer;

enum GhostModelBodyGroup
{
    BODY_THREE_SIDED_PYRAMID = 0,
    BODY_FOUR_SIDED_PYRAMID,
    BODY_SIX_SIDED_PYRAMID,
    BODY_CUBE,
    BODY_FOUR_SIDED_PRISM,
    BODY_THREE_SIDED_PRISM,
    BODY_KITE,
    BODY_FIVE_SIDED_PRISM,
    BODY_SIX_SIDED_PRISM,
    BODY_PENTAGON_BALL,
    BODY_BALL,
    BODY_PROLATE_ELLIPSE,
    BODY_TRIANGLE_BALL,
    BODY_CONE,
    BODY_CYLINDER,
    LAST
};
class CMomentumGhostBaseEntity : public CBaseAnimating, public CMomRunEntity
{
    DECLARE_CLASS(CMomentumGhostBaseEntity, CBaseAnimating);
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

public:
    CMomentumGhostBaseEntity();

    virtual void SetGhostBodyGroup(int bodyGroup);
    virtual void SetGhostColor(const uint32 newHexColor);
    virtual void SetGhostTrailProperties(const uint32 newHexColor, int newLen, bool enable);

    virtual void SetGhostAppearance(GhostAppearance_t app, bool bForceUpdate = false);
    virtual GhostAppearance_t GetAppearance() { return m_ghostAppearance; }

    virtual void StartTimer(int m_iStartTick);
    virtual void FinishTimer();

    //Pure virtual functions
    virtual void HandleGhost() = 0;
    virtual void HandleGhostFirstPerson() = 0;
    virtual void UpdateStats(const Vector &ghostVel) = 0; // for hud display..

    virtual bool IsReplayGhost() const { return false; }
    virtual bool IsOnlineGhost() const { return false; }

    void SetSpectator(CMomentumPlayer *player);
    void RemoveSpectator();
    CMomentumPlayer* GetCurrentSpectator() const { return m_pCurrentSpecPlayer; }

    CNetworkString(m_szGhostName, MAX_PLAYER_NAME_LENGTH);
    CNetworkVar(int, m_nGhostButtons);
    CNetworkVar(int, m_iDisabledButtons);
    CNetworkVar(bool, m_bBhopDisabled);
    CNetworkVar(bool, m_bSpectated); // Is the ghost being spectated by us?

    void HideGhost();
    void UnHideGhost();

    // Run entity stuff
    virtual RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_GHOST; }
    virtual void SetButtonsEnabled(int iButtonFlags, bool bEnable) OVERRIDE;
    virtual void SetBhopEnabled(bool bEnable) OVERRIDE;
    virtual bool GetBhopEnabled() const OVERRIDE;
    CNetworkVarEmbedded(CMomRunEntityData, m_Data);
    virtual CMomRunEntityData *GetRunEntData() OVERRIDE { return &m_Data; }
    CNetworkVarEmbedded(CMomRunStats, m_RunStats);
    virtual CMomRunStats *GetRunStats() OVERRIDE { return &m_RunStats; }
    virtual int GetEntIndex() OVERRIDE { return entindex(); }

protected:
    virtual void Think(void) OVERRIDE;
    virtual void Spawn(void) OVERRIDE;
    virtual void Precache(void) OVERRIDE;
    void DecalTrace(trace_t* pTrace, char const* decalName) OVERRIDE {} // Don't do any DecalTracing on this entity
    int UpdateTransmitState() OVERRIDE { return SetTransmitState(FL_EDICT_ALWAYS); }
    int ShouldTransmit(const CCheckTransmitInfo* pInfo) OVERRIDE { return FL_EDICT_ALWAYS; }

    virtual void CreateTrail();
    virtual void RemoveTrail();

    static bool CanUnduck(CMomentumGhostBaseEntity *pGhost);
    CMomentumPlayer *m_pCurrentSpecPlayer;
    GhostAppearance_t m_ghostAppearance;

private:
    CBaseEntity *m_eTrail;
};