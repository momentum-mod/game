#pragma once

#include "mom_ghostdefs.h"
#include "run/mom_run_entity.h"

class CMomentumPlayer;

class CMomentumGhostBaseEntity : public CBaseAnimating, public CMomRunEntity
{
    DECLARE_CLASS(CMomentumGhostBaseEntity, CBaseAnimating);
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

public:
    CMomentumGhostBaseEntity();

    void TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator) OVERRIDE;
    int BloodColor() OVERRIDE { return BLOOD_COLOR_RED; }

    bool ShouldCollide(int collisionGroup, int contentsMask) const OVERRIDE;

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

    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_fFlags);
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
    virtual void Spawn() OVERRIDE;
    virtual void Precache() OVERRIDE;

    void DecalTrace(trace_t* pTrace, char const* decalName) OVERRIDE {} // Don't do any DecalTracing on this entity
    int UpdateTransmitState() OVERRIDE { return SetTransmitState(FL_EDICT_ALWAYS); }
    int ShouldTransmit(const CCheckTransmitInfo* pInfo) OVERRIDE { return FL_EDICT_ALWAYS; }

    bool CanUnduck();
    void HandleDucking();
    CMomentumPlayer *m_pCurrentSpecPlayer;
};