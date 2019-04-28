#pragma once

class CMomentumReplayGhostEntity;
class CMomentumPlayer;
class CMomReplayBase;

class CMomentumReplaySystem : public CAutoGameSystemPerFrame
{
public:

    CMomentumReplaySystem(const char* pName);

    virtual ~CMomentumReplaySystem() OVERRIDE;

    // inherited member from CAutoGameSystemPerFrame
    void FrameUpdatePostEntityThink() OVERRIDE;

    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;

    void PostInit() OVERRIDE;

    // Sets the start timer tick, this is used for trimming later on
    void SetTimerStartTick(uint32 tick) { m_iStartTimerTick = tick; }
    void SetTimerStopTick(uint32 tick) { m_iStopTimerTick = tick; }

    void BeginRecording();
    void StopRecording(bool throwaway, bool delay);
    bool IsRecording() const { return m_bRecording; }
    bool IsPlayingBack() const { return m_bPlayingBack; }
    void TrimReplay(); // Trims a replay's start down to only include a defined amount of time in the start trigger

    CMomReplayBase *LoadPlayback(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD");
    void UnloadPlayback(bool shutdown = false);
    void LoadReplayGhost();
    void StartPlayback(bool firstperson);
    void StopPlayback();

    void SetTeleportedThisFrame(); // Call me when player teleports.
    const CMomReplayBase *GetRecordingReplay() const { return m_pRecordingReplay; }
    CMomReplayBase *GetRecordingReplay() { return m_pRecordingReplay; }
    const CMomReplayBase *GetPlaybackReplay() const { return m_pPlaybackReplay; }
    CMomReplayBase *GetPlaybackReplay() { return m_pPlaybackReplay; }

    //CMomRunStats *SavedRunStats() { return &m_SavedRunStats; }

  private:
    void UpdateRecordingParams(); // called every game frame after entities think and update
    void SetReplayInfo();
    void SetRunStats();
    bool StoreReplay(char *pPathOut, size_t outSize);

  private:
    bool m_bRecording;
    bool m_bPlayingBack;
    CMomReplayBase *m_pRecordingReplay;
    CMomReplayBase *m_pPlaybackReplay;

    bool m_bShouldStopRec;
    int m_iTickCount;          // MOM_TODO: Maybe remove me?
    uint32 m_iStartRecordingTick; // The tick that the replay started, used for trimming.
    uint32 m_iStartTimerTick;     // The tick that the player's timer starts, used for trimming.
    uint32 m_iStopTimerTick;      // The tick that the player's timer stopped, used for the hud
    float m_fRecEndTime;       // The time to end the recording, if delay was passed as true to StopRecording()
    //CMomRunStats m_SavedRunStats;
    // Map SHA1 hash for version purposes
    char m_szMapHash[41];
    bool m_bTeleportedThisFrame;
};

extern CMomentumReplaySystem g_ReplaySystem;