#pragma once

#include "cbase.h"
#include "mom_replay_base.h"
#include "utlmap.h"

class CMomReplayManager
{
  public:
    CMomReplayManager();
    ~CMomReplayManager();

    static CMomReplayBase *CreateEmptyReplay(uint8 version);
    static CMomReplayBase *CreateReplay(uint8 version, CBinaryReader* reader, bool bFullLoad);

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
    CMomReplayBase *m_pRecordingReplay;
    CMomReplayBase *m_pPlaybackReplay;
    bool m_bRecording;
    bool m_bPlayingBack;
    uint8 m_ucCurrentVersion;
};