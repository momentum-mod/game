#pragma once

#include "mom_ghost_base.h"
#include "GameEventListener.h"

class CMomRunStats;
class CMomReplayBase;
class CReplayFrame;

class CMomentumReplayGhostEntity : public CMomentumGhostBaseEntity, public CGameEventListener
{
    DECLARE_CLASS(CMomentumReplayGhostEntity, CMomentumGhostBaseEntity);
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

  public:
    CMomentumReplayGhostEntity();
    ~CMomentumReplayGhostEntity();

    // Increments the steps intelligently.
    void UpdateStep(int Skip);

    void EndRun();
    void StartRun(bool firstPerson = false);

    void StartTimer(int m_iStartTick) OVERRIDE;

    void HandleGhost() OVERRIDE;
    void HandleGhostFirstPerson() OVERRIDE;
    void UpdateStats(const Vector &ghostVel) OVERRIDE; // for hud display..
    bool IsReplayGhost() const OVERRIDE { return true; }

    inline void SetTickRate(float rate) { m_Data.m_flTickRate = rate; }
    inline void SetRunFlags(uint32 flags) { m_Data.m_iRunFlags = flags; }
    void SetPlaybackReplay(CMomReplayBase *pPlayback) { m_pPlaybackReplay = pPlayback; }

    CReplayFrame* GetCurrentStep();
    CReplayFrame *GetNextStep();
    CReplayFrame *GetPreviousStep();

    bool IsReplayEnt() { return true; }

    bool m_bIsActive;
    bool m_bReplayFirstPerson;

    RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_REPLAY; }
    virtual void OnZoneEnter(CTriggerZone *pTrigger, CBaseEntity *pEnt) OVERRIDE;
    virtual void OnZoneExit(CTriggerZone *pTrigger, CBaseEntity *pEnt) OVERRIDE;
    // Ghost-only
    CNetworkVar(bool, m_bIsPaused); // Is the replay paused?
    CNetworkVar(int, m_iCurrentTick); // Current tick of the replay
    CNetworkVar(int, m_iStartTickD); // The tick difference between timer and record
    CNetworkVar(int, m_iTotalTicks); // Total ticks for the replay (run time + start + end)

    // override of color so that replayghosts are always somewhat transparent.
    void SetGhostColor(const uint32 newColor) OVERRIDE;

  protected:
    void Think(void) OVERRIDE;
    void Spawn(void) OVERRIDE;
    void Precache(void) OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

    void CreateTrail() OVERRIDE;

  private:
    CMomReplayBase *m_pPlaybackReplay;

    bool m_bHasJumped;

    ConVarRef m_cvarReplaySelection;

    // for faking strafe sync calculations
    QAngle m_angLastEyeAngle;
    float m_flLastSyncVelocity;
    int m_nStrafeTicks, m_nPerfectSyncTicks, m_nAccelTicks, m_nOldReplayButtons, m_iTickElapsed;
    Vector m_vecLastVel;
};
