#pragma once

#include "cbase.h"
#include "mom_replay_base.h"
#include "utlmap.h"

class CMomReplayFactory
{
  public:
    CMomReplayFactory();
    ~CMomReplayFactory();

    static CMomReplayBase *CreateEmptyReplay(uint8 version);
    static CMomReplayBase *CreateReplay(uint8 version, CBinaryReader* reader, bool bFullLoad);

    CMomReplayBase *StartRecording(); //server only
    void StopRecording(); //server only
    bool StoreReplay(const char *path, const char *pathID = "MOD"); //server only
    static CMomReplayBase *LoadReplayFile(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD"); //client only
    CMomReplayBase *LoadReplay(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD"); //server only
    void UnloadPlayback(bool shutdown = false); //server only
    void StopPlayback(); //self
    void SetPlayingBack(bool playing) { m_bPlayingBack = playing; } //self

    inline CMomReplayBase *GetRecordingReplay() const { return m_pRecordingReplay; } //server
    inline CMomReplayBase *GetPlaybackReplay() const { return m_pPlaybackReplay; } //server
    inline bool Recording() const { return m_bRecording; } //server
    inline bool PlayingBack() const { return m_bPlayingBack; } //self

  private:
    CMomReplayBase *m_pRecordingReplay;
    CMomReplayBase *m_pPlaybackReplay;
    bool m_bRecording;
    bool m_bPlayingBack;
    uint8 m_ucCurrentVersion;
};