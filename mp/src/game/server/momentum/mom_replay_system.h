#pragma once

#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "run/mom_replay_data.h"
#include <momentum/mom_player_shared.h>
#include "run/mom_replay_factory.h"

class CMomentumReplayGhostEntity;

class CMomentumReplaySystem : public CAutoGameSystemPerFrame
{
public:
    bool m_bRecording;
    CMomReplayBase *m_pReplay;
    
    CMomentumReplaySystem(const char *pName) :
        CAutoGameSystemPerFrame(pName),
        m_bShouldStopRec(false),
        m_iTickCount(0),
        m_iStartRecordingTick(-1),
        m_iStartTimerTick(-1),
        m_fRecEndTime(-1.0f),
        m_player(nullptr)
    {
        m_pReplay = g_ReplayFactory.CreateEmptyReplay(0);
    }

    virtual ~CMomentumReplaySystem() OVERRIDE
    {
    }

public:
    // inherited member from CAutoGameSystemPerFrame
    void FrameUpdatePostEntityThink() OVERRIDE
    {
        if (m_bRecording)
            UpdateRecordingParams();
    }

    void LevelShutdownPostEntity() OVERRIDE
    {
        //Stop a recording if there is one while the level shuts down
        if (m_bRecording)
            StopRecording(true, false);

        if (g_ReplayFactory.GetPlaybackReplay())
            g_ReplayFactory.UnloadPlayback(true);
    }

    //Sets the start timer tick, this is used for trimming later on
    void SetTimerStartTick(int tick)
    {
        m_iStartTimerTick = tick;
    }

    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(bool throwaway, bool delay);
    void TrimReplay(); //Trims a replay's start down to only include a defined amount of time in the start trigger

private:
    void UpdateRecordingParams(); // called every game frame after entities think and update
    void SetReplayInfo();
    void SetRunStats();
    bool StoreReplay(const char* path, const char* pathID = "MOD");

    bool m_bShouldStopRec;
    int m_iTickCount;// MOM_TODO: Maybe remove me?
    int m_iStartRecordingTick;//The tick that the replay started, used for trimming.
    int m_iStartTimerTick;//The tick that the player's timer starts, used for trimming.
    float m_fRecEndTime;// The time to end the recording, if delay was passed as true to StopRecording()

    CMomentumPlayer *m_player;
};

extern CMomentumReplaySystem *g_ReplaySystem;