#ifndef MOM_REPLAY_GHOST_H
#define MOM_REPLAY_GHOST_H

#pragma once

#include "cbase.h"
#include "mom_player.h"
#include "in_buttons.h"
#include "mom_entity_run_data.h"
#include "replayformat.h"

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

class CMomentumReplayGhostEntity : public CBaseAnimating
{
    DECLARE_CLASS(CMomentumReplayGhostEntity, CBaseAnimating);
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

  public:
    CMomentumReplayGhostEntity();
    ~CMomentumReplayGhostEntity();
    const char *GetGhostModel() const;
    void SetGhostModel(const char *model);
    void SetGhostBodyGroup(int bodyGroup);
    static void SetGhostColor(const CCommand &args);
    // Increments the steps intelligently.
    void UpdateStep();

    void EndRun();
    void StartRun(bool firstPerson = false, bool shouldLoop = false);
    void StartTimer(int m_iStartTick);
    void StopTimer();
    void HandleGhost();
    void HandleGhostFirstPerson();
    void UpdateStats(Vector ghostVel); // for hud display..
    void SetHeader(replay_header_t &head)
    {
        header = head;
        m_flRunTime = header.runTime;
        m_flTickRate = header.interval_per_tick;
        m_RunData.m_iRunFlags = header.runFlags;
    }
    void SetRunStats(RunStats_t &stats);
    RunStats_t *GetRunStats() { return &m_RunStats; }

    void AddSpectator(CMomentumPlayer* player)
    {
        spectators.AddToTail(player);
    }

    void RemoveSpectator(CMomentumPlayer* player)
    {
        spectators.FindAndRemove(player);
    }

    bool m_bIsActive;
    int m_nStartTick;

    CNetworkVarEmbedded(CMOMRunEntityData, m_RunData);
    CNetworkVar(int, m_nReplayButtons);
    CNetworkVar(int, m_iTotalStrafes);
    CNetworkVar(int, m_iTotalJumps);
    CNetworkVar(float, m_flRunTime);
    CNetworkVar(float, m_flTickRate);
    CNetworkString(m_pszPlayerName, MAX_PLAYER_NAME_LENGTH);

  protected:
    void Think(void) override;
    void Spawn(void) override;
    void Precache(void) override;

  private:
    char m_pszModel[256], m_pszMapName[256];
    replay_header_t header;
    replay_frame_t currentStep;
    replay_frame_t nextStep;
    RunStats_t m_RunStats;

    CUtlVector<CMomentumPlayer *> spectators;

    int step;
    int m_iBodyGroup = BODY_PROLATE_ELLIPSE;
    Color m_ghostColor;
    static Color m_newGhostColor;
    bool m_bHasJumped;
    // for faking strafe sync calculations
    QAngle m_qLastEyeAngle;
    float m_flLastSyncVelocity;
    int m_nStrafeTicks, m_nPerfectSyncTicks, m_nAccelTicks, m_nOldReplayButtons;
    bool m_bReplayShouldLoop, m_bReplayFirstPerson;
};

#endif // MOM_REPLAY_GHOST_H
