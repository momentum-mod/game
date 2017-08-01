#include "cbase.h"
#include "mom_replay_factory.h"
#include "filesystem.h"
#include "mom_replay_versions.h"
#ifndef CLIENT_DLL
#include "momentum/mom_replay_entity.h"
#else
#include "../../server/momentum/mom_replay_system.h"
#endif


CMomReplayFactory g_ReplayFactory;

CMomReplayFactory::CMomReplayFactory() :
    m_pPlaybackReplay(nullptr),
    m_bPlayingBack(false),
    m_ucCurrentVersion(0)
{

}

CMomReplayFactory::~CMomReplayFactory()
{
    if (m_pPlaybackReplay)
        delete m_pPlaybackReplay;
}

CMomReplayBase *CMomReplayFactory::CreateEmptyReplay(uint8 version)
{
    //Is there a more compact way to do this without introducing more intermediate objects?
    switch(version)
    {
        case 0: //Place 0 before the newest version's case.
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
        case 0:
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

void CMomReplayFactory::StopPlayback()
{
    if (!PlayingBack())
        return;

#ifndef CLIENT_DLL
    Log("Stopping replay playback.\n");
    UnloadPlayback();
#endif
}