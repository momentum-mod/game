#pragma once

#include "cbase.h"
#include "mom_replay_base.h"
#include "utlmap.h"

#define REPLAY_MAGIC_LE 0x524D4F4D
#define REPLAY_MAGIC_BE 0x4D4F4D52

class CMomReplayFactory
{
  public:
    CMomReplayFactory();
    //~CMomReplayFactory();

    //Pass 0 as a version number to get whatever the latest version is.
    CMomReplayBase *CreateEmptyReplay(uint8 version);
    CMomReplayBase *CreateReplay(uint8 version, CBinaryReader* reader, bool bFullLoad);

    //Returns a replay file and constructs a versioned replay object.
    CMomReplayBase *LoadReplayFile(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD");

    uint8 m_ucCurrentVersion;
};

extern CMomReplayFactory g_ReplayFactory;