#pragma once

#include "run/mom_replay_base.h"

class CMomentumReplayGhostEntity;
class CMomentumPlayer;

class CMomentumReplaySystem : public CAutoGameSystemPerFrame
{
public:

    CMomentumReplaySystem(const char* pName);

    virtual ~CMomentumReplaySystem() OVERRIDE;

    // inherited member from CAutoGameSystemPerFrame
    void FrameUpdatePostEntityThink() OVERRIDE;

    void LevelShutdownPostEntity() OVERRIDE;

    void PostInit() OVERRIDE;

    //Sets the start timer tick, this is used for trimming later on
    void SetTimerStartTick(int tick)
    {
        m_iStartTimerTick = tick;
    }

    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(bool throwaway, bool delay);
    void TrimReplay(); //Trims a replay's start down to only include a defined amount of time in the start trigger
    
    void Start(bool firstperson);
    CMomReplayBase *LoadPlayback(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD");
    void UnloadPlayback(bool shutdown = false);
    void StopPlayback();

    bool m_bRecording;
    bool m_bPlayingBack;
    CMomReplayBase *m_pRecordingReplay;
    CMomReplayBase *m_pPlaybackReplay;
    CMomentumPlayer *m_player;
    
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
};

extern CMomentumReplaySystem g_ReplaySystem;