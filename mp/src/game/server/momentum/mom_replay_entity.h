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

#define GHOST_MODEL "models/player/player_shape_base.mdl"
enum ghostModelBodyGroup
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
    BODY_CYLINDER
};

class CMomentumPlayer;

class CMomentumReplayGhostEntity : public CBaseAnimating, public CGameEventListener
{
    DECLARE_CLASS(CMomentumReplayGhostEntity, CBaseAnimating);
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

  public:
    CMomentumReplayGhostEntity();
    ~CMomentumReplayGhostEntity();

    void SetGhostModel(const char *model);
    void SetGhostBodyGroup(int bodyGroup);
    static void SetGhostColor(const CCommand &args);
    // Increments the steps intelligently.
    void UpdateStep(int Skip);

    void EndRun();
    void StartRun(bool firstPerson = false);
    void StartTimer(int m_iStartTick);
    void StopTimer();
    void HandleGhost();
    void HandleGhostFirstPerson();
    void UpdateStats(const Vector &ghostVel); // for hud display..

    const char *GetGhostModel() const { return m_pszModel; }
    void SetRunStats(CMomRunStats *stats) { m_SrvData.m_RunStatsData = *stats->m_pData; }

    void SetSpectator(CMomentumPlayer *player)
    {
        m_pPlayerSpectator = player;
    }

    void RemoveSpectator() { m_pPlayerSpectator = nullptr; }

    inline void SetTickRate(float rate) { m_flTickRate = rate; }
    inline void SetRunFlags(uint32 flags) { m_SrvData.m_RunData.m_iRunFlags = flags; }

    void SetPlaybackReplay(CMomReplayBase *pPlayback) { m_pPlaybackReplay = pPlayback; }

    CReplayFrame *GetCurrentStep() { return m_pPlaybackReplay->GetFrame(m_SrvData.m_iCurrentTick); }
    CReplayFrame *GetNextStep();

    void (*StdDataToReplay)(StdReplayDataFromServer* from);

    bool m_bIsActive;
    bool m_bReplayFirstPerson;

    StdReplayDataFromServer m_SrvData;
    CMomRunStats m_RunStats;
    CNetworkVar(float, m_flTickRate);
    CNetworkVar(int, m_iTotalTimeTicks);
    CNetworkString(m_pszPlayerName, MAX_PLAYER_NAME_LENGTH);

  protected:
    void Think(void) OVERRIDE;
    void Spawn(void) OVERRIDE;
    void Precache(void) OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

  private:
    char m_pszModel[256], m_pszMapName[256];

    // These are the players spectating this ghost. This will most likely be used in
    // online mode, where you can play back a ghost and people can watch with you.
    // @Gocnak: I'm not really seeing why though, shouldn't players just download replay files
    // if they want to view them...?
    //CUtlVector<CMomentumPlayer *> m_rgSpectators;
    CMomentumPlayer *m_pPlayerSpectator;

    CMomReplayBase *m_pPlaybackReplay;

    int m_iBodyGroup;
    Color m_GhostColor;
    static Color m_NewGhostColor;
    bool m_bHasJumped;
    // for faking strafe sync calculations
    QAngle m_angLastEyeAngle;
    float m_flLastSyncVelocity;
    int m_nStrafeTicks, m_nPerfectSyncTicks, m_nAccelTicks, m_nOldReplayButtons, m_iTickElapsed;
};

#endif // MOM_REPLAY_GHOST_H
