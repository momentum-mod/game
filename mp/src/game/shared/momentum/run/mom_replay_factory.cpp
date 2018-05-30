#include "cbase.h"
#include "mom_replay_factory.h"
#include "filesystem.h"
#include "mom_replay_versions.h"
#ifdef GAME_DLL
#include "momentum/mom_replay_entity.h"
#endif

#include "tier0/memdbgon.h"

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

CMomReplayFactory g_ReplayFactory;