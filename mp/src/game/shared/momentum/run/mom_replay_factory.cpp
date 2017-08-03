#include "cbase.h"
#include "mom_replay_factory.h"
#include "filesystem.h"
#include "mom_replay_versions.h"
#ifdef GAME_DLL
#include "momentum/mom_replay_entity.h"
#include "../../server/momentum/mom_replay_system.h"
#endif


CMomReplayFactory g_ReplayFactory;

CMomReplayFactory::CMomReplayFactory() :
    m_ucCurrentVersion(0)
{

}

CMomReplayBase *CMomReplayFactory::CreateEmptyReplay(uint8 version)
{
    //Is there a more compact way to do this without introducing more intermediate objects?
    switch(version)
    {
        case 0: //Place 0 before the newest version's case, without a `break;`
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
        Log("Replay file not found: %s\n", pFileName);
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
#ifndef CLIENT_DLL
    if (g_ReplaySystem.m_bPlayingBack)
        g_ReplaySystem.StopPlayback();

    if (g_ReplaySystem.m_pPlaybackReplay)
        g_ReplaySystem.UnloadPlayback();

    g_ReplaySystem.m_pPlaybackReplay = LoadReplayFile(pFileName, bFullLoad, pPathID);

    if (bFullLoad && g_ReplaySystem.m_pPlaybackReplay)
    {
        // Create the run entity here
        CMomentumReplayGhostEntity *pGhost = static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));
        pGhost->SetRunStats(g_ReplaySystem.m_pPlaybackReplay->GetRunStats());
        pGhost->m_RunData.m_flRunTime = g_ReplaySystem.m_pPlaybackReplay->GetRunTime();
        pGhost->m_RunData.m_iRunFlags = g_ReplaySystem.m_pPlaybackReplay->GetRunFlags();
        pGhost->m_flTickRate = g_ReplaySystem.m_pPlaybackReplay->GetTickInterval();
        pGhost->SetPlaybackReplay(g_ReplaySystem.m_pPlaybackReplay);
        pGhost->m_RunData.m_iStartTickD = g_ReplaySystem.m_pPlaybackReplay->GetStartTick();
        g_ReplaySystem.m_pPlaybackReplay->SetRunEntity(pGhost);
    }

    return g_ReplaySystem.m_pPlaybackReplay;
#endif
    return nullptr;
}

