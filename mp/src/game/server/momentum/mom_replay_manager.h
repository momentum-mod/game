#pragma once

#include "cbase.h"
#include "mom_replay_base.h"
#include "utlmap.h"

class CMomentumReplayGhostEntity;

class CMomReplayManager
{
private:
    class CReplayCreatorBase
    {
    public:
        virtual CMomReplayBase* CreateReplay() = 0;
        virtual CMomReplayBase* LoadReplay(CBinaryReader* reader) = 0;
    };

    template <typename T>
    class CReplayCreator :
        public CReplayCreatorBase
    {
    public:
        virtual CMomReplayBase* CreateReplay() override { return new T(); }
        virtual CMomReplayBase* LoadReplay(CBinaryReader* reader) override { return new T(reader); }
    };

public:
    CMomReplayManager();
    ~CMomReplayManager();

public:
    CMomReplayBase* StartRecording();
    void StopRecording();
    bool StoreReplay(const char* path, const char* pathID = "MOD");
    CMomReplayBase* LoadReplay(const char* path, const char* pathID = "MOD");
    void StopPlayback();
    void UnloadPlayback(bool shutdown = false);
    void SetPlayingBack(bool playing) { m_bPlayingBack = playing; }

public:
    inline CMomReplayBase* GetRecordingReplay() const { return m_pRecordingReplay; }
    inline CMomReplayBase* GetPlaybackReplay() const { return m_pPlaybackReplay; }
    inline bool Recording() const { return m_bRecording; }
    inline bool PlayingBack() const { return m_bPlayingBack; }

private:
    static bool RegisterCreators();

private:
    CMomReplayBase* m_pRecordingReplay;
    CMomReplayBase* m_pPlaybackReplay;
    bool m_bRecording;
    bool m_bPlayingBack;
    uint8 m_ucCurrentVersion;

private:
    static CUtlMap<uint8, CReplayCreatorBase*> m_mapCreators;
    static bool m_bDummy;
};