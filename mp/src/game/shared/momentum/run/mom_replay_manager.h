#pragma once

#include "cbase.h"
#include "mom_replay_base.h"
#include "utlmap.h"

class CMomentumReplayGhostEntity;

class CMomReplayManager
{
  private:
    abstract_class CReplayCreatorBase
    {
      public:
        virtual CMomReplayBase *CreateReplay() = 0;
        virtual CMomReplayBase *LoadReplay(CBinaryReader *reader, bool bFull) = 0;
    };

    template <typename T> class CReplayCreator : public CReplayCreatorBase
    {
      public:
        virtual CMomReplayBase *CreateReplay() OVERRIDE { return new T(); }
        virtual CMomReplayBase *LoadReplay(CBinaryReader *reader, bool bFull) OVERRIDE { return new T(reader, bFull); }
    };

  public:
    CMomReplayManager();
    ~CMomReplayManager();

    CMomReplayBase *StartRecording();
    void StopRecording();
    bool StoreReplay(const char *path, const char *pathID = "MOD");
    static CMomReplayBase *LoadReplayFile(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD");
    CMomReplayBase *LoadReplay(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD");
    void UnloadPlayback(bool shutdown = false);
    void StopPlayback();
    void SetPlayingBack(bool playing) { m_bPlayingBack = playing; }

    inline CMomReplayBase *GetRecordingReplay() const { return m_pRecordingReplay; }
    inline CMomReplayBase *GetPlaybackReplay() const { return m_pPlaybackReplay; }
    inline bool Recording() const { return m_bRecording; }
    inline bool PlayingBack() const { return m_bPlayingBack; }

  private:
    static bool RegisterCreators();

    CMomReplayBase *m_pRecordingReplay;
    CMomReplayBase *m_pPlaybackReplay;
    bool m_bRecording;
    bool m_bPlayingBack;
    uint8 m_ucCurrentVersion;

    static CUtlMap<uint8, CReplayCreatorBase *> m_mapCreators;
    static bool m_bDummy;
};