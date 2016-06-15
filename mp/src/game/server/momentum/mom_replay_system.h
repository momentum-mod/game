#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H

#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "mom_replay_data.h"
#include "mom_player_shared.h"
#include "mom_replay_manager.h"

#define END_RECORDING_DELAY 1.0f //Delay the ending by this amount of seconds
#define START_TRIGGER_TIME_SEC 2.0f //We only want this amount in seconds of being in the start trigger

class CMomentumReplayGhostEntity;

class CMomentumReplaySystem : CAutoGameSystemPerFrame
{
public:
    CMomentumReplaySystem(const char *pName) :
        CAutoGameSystemPerFrame(pName),
        m_bShouldStopRec(false),
        m_iTickCount(0),
        m_iStartRecordingTick(-1),
        m_iStartTimerTick(-1),
        m_fRecEndTime(-1.0f), 
        m_player(nullptr)
    {
        m_pReplayManager = new CMomReplayManager();
    }

    virtual ~CMomentumReplaySystem() override
    {
        delete m_pReplayManager;
    }

public:
    // inherited member from CAutoGameSystemPerFrame
    void FrameUpdatePostEntityThink() override
    {
        if (m_pReplayManager->Recording())
            UpdateRecordingParams();
    }

    void LevelShutdownPostEntity() override
    {
        //Stop a recording if there is one while the level shuts down
        if (m_pReplayManager->Recording())
            StopRecording(nullptr, true, false);
    }

    void SetTimerStartTick(int tick)
    {
        m_iStartTimerTick = tick;
    }

    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CBasePlayer *pPlayer, bool throwaway, bool delay);

    void StartReplay(bool firstperson = false);
    void EndReplay(CMomentumReplayGhostEntity *pGhost);// Stops a given replay. If null is passed, it ends all replays.
    void TrimReplay(); //Trims a replay's start down to only include a defined amount of time in the start trigger
    void OnGhostEntityRemoved(CMomentumReplayGhostEntity*);// Called when a ghost entity is done being played.

    void AddGhost(CMomentumReplayGhostEntity*);
    void RemoveGhost(CMomentumReplayGhostEntity*);

    inline CMomReplayManager* GetReplayManager() const { return m_pReplayManager; }

private:
    void UpdateRecordingParams(); // called every game frame after entities think and update
    void SetReplayInfo();
    void SetRunStats();

private:
    bool m_bShouldStopRec;
    int m_iTickCount;// MOM_TODO: Maybe remove me?
    int m_iStartRecordingTick;//The tick that the replay started, used for trimming.
    int m_iStartTimerTick;//The tick that the player's timer starts, used for trimming.
    float m_fRecEndTime;// The time to end the recording, if delay was passed as true to StopRecording()

    CMomentumPlayer *m_player;
    CUtlVector<CMomentumReplayGhostEntity*> m_rgGhosts;

    CMomReplayManager* m_pReplayManager;
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif // MOM_REPLAY_H