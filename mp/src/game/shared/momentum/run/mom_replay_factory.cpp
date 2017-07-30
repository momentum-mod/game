#include "cbase.h"
#include "mom_replay_factory.h"
#include "filesystem.h"
#include "mom_replay_versions.h"
#ifndef CLIENT_DLL
#include "momentum/mom_replay_entity.h"
#endif

#define REPLAY_MAGIC_LE 0x524D4F4D
#define REPLAY_MAGIC_BE 0x4D4F4D52

CMomReplayFactory::CMomReplayFactory() :
    m_pRecordingReplay(nullptr),
    m_pPlaybackReplay(nullptr),
    m_bRecording(false),
    m_bPlayingBack(false),
    m_ucCurrentVersion(0)
{
    // DO NOT FORGET to set the latest replay version here.
    m_ucCurrentVersion = 1;
}

CMomReplayFactory::~CMomReplayFactory()
{
    if (m_pRecordingReplay)
        delete m_pRecordingReplay;

    if (m_pPlaybackReplay)
        delete m_pPlaybackReplay;
}

CMomReplayBase* CMomReplayFactory::StartRecording()
{
    if (Recording())
        return m_pRecordingReplay;

    Log("Started recording a replay...\n");

    m_bRecording = true;

    m_pRecordingReplay = CreateEmptyReplay(m_ucCurrentVersion);
    return m_pRecordingReplay;
}

void CMomReplayFactory::StopRecording()
{
    if (!Recording())
        return;

    Log("Stopped recording a replay.\n");

    m_bRecording = false;

    delete m_pRecordingReplay;
    m_pRecordingReplay = nullptr;
}

CMomReplayBase *CMomReplayFactory::CreateEmptyReplay(uint8 version)
{
    //Is there a more compact way to do this without introducing more intermediate objects?
    switch(version)
    {
        case 1:
            return new CMomReplayV1();
            
        default:
            Log("Invalid replay version: %d\n", version);
            return nullptr;
    }
}

CMomReplayBase *CMomReplayFactory::CreateReplay(uint8 version, CBinaryReader* reader, bool bFullLoad)
{
    switch(version)
    {
        case 1:
            return new CMomReplayV1(reader, bFullLoad);
        
        default:
            Log("Invalid replay version: %d\n", version);
            return nullptr;
    }
}

CMomReplayBase* CMomReplayFactory::LoadReplayFile(const char* pFileName, bool bFullLoad, const char* pPathID)
{
    Log("Loading a replay from '%s'...\n", pFileName);

    auto file = filesystem->Open(pFileName, "r+b", pPathID);

    if (!file)
    {
        filesystem->Close(file);
        return nullptr;
    }

    CBinaryReader reader(file);

    uint32 magic = reader.ReadUInt32();

    if (magic != REPLAY_MAGIC_LE && magic != REPLAY_MAGIC_BE)
    {
        filesystem->Close(file);
        return nullptr;
    }

    if (magic == REPLAY_MAGIC_BE)
        reader.ShouldFlipEndianness(true);

    uint8 version = reader.ReadUInt8();

    Log("Loading replay '%s' of version '%d'...\n", pFileName, version);

    // MOM_TODO (OrfeasZ): Verify that replay parsing was successful.
    CMomReplayBase * toReturn = CreateReplay(version, &reader, bFullLoad);

    filesystem->Close(file);
    Log("Successfully loaded replay.\n");

    return toReturn;
}


CMomReplayBase *CMomReplayFactory::LoadReplay(const char *pFileName, bool bFullLoad, const char *pPathID)
{
    if (PlayingBack())
        StopPlayback();

    if (m_pPlaybackReplay)
        UnloadPlayback();

    m_pPlaybackReplay = LoadReplayFile(pFileName, bFullLoad, pPathID);

    if (bFullLoad)
    {
#ifndef CLIENT_DLL
        // Create the run entity here
        CMomentumReplayGhostEntity *pGhost = static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));
        pGhost->SetRunStats(m_pPlaybackReplay->GetRunStats());
        pGhost->m_RunData.m_flRunTime = m_pPlaybackReplay->GetRunTime();
        pGhost->m_RunData.m_iRunFlags = m_pPlaybackReplay->GetRunFlags();
        pGhost->m_flTickRate = m_pPlaybackReplay->GetTickInterval();
        pGhost->SetPlaybackReplay(m_pPlaybackReplay);
        pGhost->m_RunData.m_iStartTickD = m_pPlaybackReplay->GetStartTick();
        m_pPlaybackReplay->SetRunEntity(pGhost);
#endif
    }

    return m_pPlaybackReplay;
}

void CMomReplayFactory::UnloadPlayback(bool shutdown)
{
    SetPlayingBack(false);

    if (m_pPlaybackReplay)
    {
#ifndef CLIENT_DLL
        if (m_pPlaybackReplay->GetRunEntity() && !shutdown)
            m_pPlaybackReplay->GetRunEntity()->EndRun();
#endif

        delete m_pPlaybackReplay;
    }

    m_pPlaybackReplay = nullptr;

    DevLog("Successfully unloaded playback, shutdown: %i\n", shutdown);
}

bool CMomReplayFactory::StoreReplay(const char* path, const char* pathID)
{
    if (!m_pRecordingReplay)
        return false;

    auto file = filesystem->Open(path, "w+b", pathID);

    if (!file)
    {
        filesystem->Close(file);
        return false;
    }

    Log("Storing replay of version '%d' to '%s'...\n", m_pRecordingReplay->GetVersion(), path);

    CBinaryWriter writer(file);

    writer.WriteUInt32(REPLAY_MAGIC_LE);
    writer.WriteUInt8(m_pRecordingReplay->GetVersion());
    m_pRecordingReplay->Serialize(&writer);

    filesystem->Close(file);

    return true;
}

void CMomReplayFactory::StopPlayback()
{
    if (!PlayingBack())
        return;

#ifndef CLIENT_DLL
    Log("Stopping replay playback.\n");
    UnloadPlayback();
#endif
}