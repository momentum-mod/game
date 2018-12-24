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

CMomReplayBase *CMomReplayFactory::CreateReplay(uint8 version, CUtlBuffer &reader, bool bFullLoad)
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

    CUtlBuffer reader;
    bool bFile = filesystem->ReadFile(pFileName, pPathID, reader);

    if (!bFile)
    {
        Log("Replay file not found: %s\n", pFileName);
        return nullptr;
    }

    uint32 magic = reader.GetUnsignedInt();

    if (magic != REPLAY_MAGIC_LE && magic != REPLAY_MAGIC_BE)
    {
        Warning("Not a replay file!\n");
        return nullptr;
    }

    if (magic == REPLAY_MAGIC_BE)
        reader.ActivateByteSwapping(true);

    uint8 version = reader.GetUnsignedChar();

    Log("Loading replay '%s' of version '%d'...\n", pFileName, version);

    // MOM_TODO: Verify that replay parsing was successful.
    CMomReplayBase * toReturn = CreateReplay(version, reader, bFullLoad);

    Log("Successfully loaded replay.\n");

    return toReturn;
}

CMomReplayFactory g_ReplayFactory;