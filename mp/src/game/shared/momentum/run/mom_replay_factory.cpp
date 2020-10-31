#include "cbase.h"
#include "mom_replay_factory.h"
#include "filesystem.h"
#include "mom_replay_versions.h"
#ifdef GAME_DLL
#include "momentum/mom_replay_entity.h"
#endif
#include "util/mom_util.h"

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
static MAKE_TOGGLE_CONVAR(mom_replay_debug, "0", FCVAR_ARCHIVE, "If 1, prints out debug info when loading replays.");
#endif

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

CMomReplayBase* CMomReplayFactory::LoadReplayFile(const char *pFileName, bool bFullLoad, const char *pPathID)
{
    bool bLogReplay = IsReplayDebugEnabled();

    if (bLogReplay)
        Log("Loading a replay from '%s'...\n", pFileName);

    CUtlBuffer reader;
    bool bFile = filesystem->ReadFile(pFileName, pPathID, reader);

    if (!bFile)
    {
        Log("Replay file not found: %s\n", pFileName);
        return nullptr;
    }

    return LoadReplayFromBuffer(reader, bFullLoad);
}

CMomReplayBase* CMomReplayFactory::LoadReplayFromBuffer(CUtlBuffer &reader, bool bFullLoad)
{
    bool bLogReplay = IsReplayDebugEnabled();

    if (bLogReplay)
        Log("Loading a replay from a buffer...\n");

    if (!reader.IsValid())
    {
        Warning("Attempted to read a replay from an invalid buffer!\n");
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

    if (bLogReplay)
        Log("Loading replay of version '%d'...\n", version);

    // MOM_TODO: Verify that replay parsing was successful.
    CMomReplayBase *toReturn = CreateReplay(version, reader, bFullLoad);
    char hash[41];
    if (MomUtil::GetSHA1Hash(reader, hash, sizeof(hash)))
        toReturn->SetRunHash(hash);

    if (bLogReplay)
        Log("Successfully loaded replay.\n");

    return toReturn;
}

CMomReplayFactory g_ReplayFactory;