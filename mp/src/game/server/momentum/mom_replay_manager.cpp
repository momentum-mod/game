#include "cbase.h"
#include "mom_replay_manager.h"
#include "filesystem.h"
#include "mom_replay_v1.h"
#include "mom_replay_entity.h"

#define REPLAY_MAGIC_LE 0x524D4F4D
#define REPLAY_MAGIC_BE 0x4D4F4D52

CUtlMap<uint8, CMomReplayManager::CReplayCreatorBase*> CMomReplayManager::m_mapCreators;
bool CMomReplayManager::m_bDummy = CMomReplayManager::RegisterCreators();

//////////////////////////////////////////////////////////////////////////

bool CMomReplayManager::RegisterCreators()
{
    SetDefLessFunc(CMomReplayManager::m_mapCreators);

    // Register any new replay versions here.
    m_mapCreators.Insert(1, new CMomReplayManager::CReplayCreator<CMomReplayV1>());

    return true;
}

//////////////////////////////////////////////////////////////////////////

CMomReplayManager::CMomReplayManager() :
    m_pRecordingReplay(nullptr),
    m_pPlaybackReplay(nullptr),
    m_bRecording(false),
    m_bPlayingBack(false),
    m_ucCurrentVersion(0)
{
    // DO NOT FORGET to set the latest replay version here.
    // This could be automated but CUltMap is a piece of work.
    m_ucCurrentVersion = 1;
}

CMomReplayManager::~CMomReplayManager()
{
    if (m_pRecordingReplay)
        delete m_pRecordingReplay;

    if (m_pPlaybackReplay)
        delete m_pPlaybackReplay;
}

CMomReplayBase* CMomReplayManager::StartRecording()
{
    if (Recording())
        return m_pRecordingReplay;

    Log("Started recording a replay...\n");

    m_bRecording = true;

    m_pRecordingReplay = m_mapCreators.Element(m_mapCreators.Find(m_ucCurrentVersion))->CreateReplay();
    return m_pRecordingReplay;
}

void CMomReplayManager::StopRecording()
{
    if (!Recording())
        return;

    Log("Stopped recording a replay.\n");

    m_bRecording = false;

    delete m_pRecordingReplay;
    m_pRecordingReplay = nullptr;
}

CMomReplayBase* CMomReplayManager::LoadReplay(const char* path, const char* pathID)
{
    if (PlayingBack())
        StopPlayback();

    if (m_pPlaybackReplay)
        UnloadPlayback();

    Log("Loading a replay from '%s'...\n", path);

    auto file = filesystem->Open(path, "r+b", pathID);

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

    if (m_mapCreators.Find(version) == m_mapCreators.InvalidIndex())
    {
        filesystem->Close(file);
        return nullptr;
    }

    Log("Loading replay '%s' of version '%d'...\n", path, version);

    // MOM_TODO (OrfeasZ): Verify that replay parsing was successful.
    m_pPlaybackReplay = m_mapCreators.Element(m_mapCreators.Find(version))->LoadReplay(&reader);

    filesystem->Close(file);

    Log("Successfully loaded replay.\n");

    //Create the run entity here
    CMomentumReplayGhostEntity *pGhost = static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));
    pGhost->SetRunStats(m_pPlaybackReplay->GetRunStats());
    pGhost->m_RunData.m_flRunTime = m_pPlaybackReplay->GetRunTime();
    pGhost->m_RunData.m_iRunFlags = m_pPlaybackReplay->GetRunFlags();
    pGhost->m_flTickRate = m_pPlaybackReplay->GetTickInterval();
    pGhost->SetPlaybackReplay(m_pPlaybackReplay);
    m_pPlaybackReplay->SetRunEntity(pGhost);

    return m_pPlaybackReplay;
}

bool CMomReplayManager::StoreReplay(const char* path, const char* pathID)
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

void CMomReplayManager::StopPlayback()
{
    if (!PlayingBack())
        return;

    Log("Stopping replay playback.\n");
    UnloadPlayback();
}

void CMomReplayManager::UnloadPlayback(bool shutdown)
{
    SetPlayingBack(false);

    if (m_pPlaybackReplay)
    {
        if (m_pPlaybackReplay->GetRunEntity() && !shutdown)
            m_pPlaybackReplay->GetRunEntity()->EndRun();

        delete m_pPlaybackReplay;
    }

    m_pPlaybackReplay = nullptr;

    DevLog("Successfully unloaded playback, shutdown: %i\n", shutdown);
}