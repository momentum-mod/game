#ifndef MOM_REPLAY_GHOST_H
#define MOM_REPLAY_GHOST_H

#pragma once

#include "cbase.h"
#include "in_buttons.h"
#include "run/mom_entity_run_data.h"
#include "mom_player.h"
#include "run/mom_replay_data.h"
#include "mom_replay_system.h"
#include <GameEventListener.h>
#include "mom_ghost_base.h"

class CMomentumPlayer;

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

    void SetRunStats(CMomRunStats *stats) { m_SrvData.m_RunStatsData = *stats->m_pData; }
    inline void SetTickRate(float rate) { m_SrvData.m_flTickRate = rate; }
    inline void SetRunFlags(uint32 flags) { m_SrvData.m_RunData.m_iRunFlags = flags; }
    void SetPlaybackReplay(CMomReplayBase *pPlayback) { m_pPlaybackReplay = pPlayback; }

    CReplayFrame *GetCurrentStep() { return m_pPlaybackReplay->GetFrame(m_SrvData.m_iCurrentTick); }
    CReplayFrame *GetNextStep();
    
    bool IsReplayEnt() { return true; }
    void (*StdDataToReplay)(StdReplayDataFromServer* from);

    bool m_bIsActive;
    bool m_bReplayFirstPerson;

    StdReplayDataFromServer m_SrvData;
    CMomRunStats m_RunStats;

    //override of color so that replayghosts are always somewhat transparent.
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

    // for faking strafe sync calculations
    QAngle m_angLastEyeAngle;
    float m_flLastSyncVelocity;
    int m_nStrafeTicks, m_nPerfectSyncTicks, m_nAccelTicks, m_nOldReplayButtons, m_iTickElapsed;
};

#endif // MOM_REPLAY_GHOST_H
