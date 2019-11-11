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

    void LoadFromReplayBase(CMomReplayBase *pReplay);

    void StartRun(bool firstPerson = false);
    void EndRun();

    void HandleGhost() OVERRIDE;
    void HandleGhostFirstPerson() OVERRIDE;
    void UpdateStats(const Vector &ghostVel) OVERRIDE; // for hud display..
    bool IsReplayGhost() const OVERRIDE { return true; }

    void GoToTick(int tick);

    CReplayFrame* GetCurrentStep();
    CReplayFrame *GetNextStep();
    CReplayFrame *GetPreviousStep();

    bool IsReplayEnt() { return true; }


    RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_REPLAY; }
    virtual void OnZoneEnter(CTriggerZone *pTrigger) OVERRIDE;
    virtual void OnZoneExit(CTriggerZone *pTrigger) OVERRIDE;
    // Ghost-only
    CNetworkVar(bool, m_bIsPaused); // Is the replay paused?
    CNetworkVar(int, m_iCurrentTick); // Current tick of the replay
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
    bool m_bIsActive;
    bool m_bReplayFirstPerson;

    // for faking strafe sync calculations
    QAngle m_angLastEyeAngle;
    float m_flLastSyncVelocity;
    int m_nStrafeTicks, m_nPerfectSyncTicks, m_nAccelTicks, m_nOldReplayButtons, m_iTickElapsed;
    Vector m_vecLastVel;
};
