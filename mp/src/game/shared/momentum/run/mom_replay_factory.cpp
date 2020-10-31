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

    const auto pReplay = LoadReplayFromBuffer(reader, bFullLoad);
    if (pReplay)
        pReplay->SetFilePath(pFileName);
    return pReplay;
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
    char hash[REPLAY_HASH_LENGTH + 1];
    if (MomUtil::GetSHA1Hash(reader, hash, sizeof(hash)))
        toReturn->SetRunHash(hash);

    if (bLogReplay)
        Log("Successfully loaded replay.\n");

    return toReturn;
}

bool CMomReplayFactory::IsReplayDebugEnabled()
{
#ifdef CLIENT_DLL
    return mom_replay_debug.GetBool();
#else
    static ConVarRef replayDebug("mom_replay_debug");
    return replayDebug.GetBool();
#endif
}

#ifdef CLIENT_DLL
AsyncReplayStatus_t CMomReplayFactory::AsyncLoadReplaysByFilename(const CUtlStringList &filenames, const char *pathID,
                                                                 const MomLocalReplayCallback_t &callback,
                                                                 MomReplayAsyncHandle &outHandle)
{
    bool bLogReplay = IsReplayDebugEnabled();

    if (bLogReplay)
        Log("Loading replays asynchronously...\n");

    int idx = m_vecRequests.AddToTail();
    auto &internalRequest = m_vecRequests[idx];

    // This belongs in a constructor but CUtlVector doesn't have emplace_back
    size_t count = filenames.Size();
    internalRequest.vecControls.EnsureCount(count);
    internalRequest.vecFSRequests.EnsureCount(count);
    internalRequest.controls_pending = count;

    outHandle = internalRequest.handle;

    internalRequest.callback = callback;

    FOR_EACH_VEC(filenames, i)
    {
        internalRequest.vecFSRequests[i].pszFilename = filenames[i];
        internalRequest.vecFSRequests[i].pfnCallback = FilesystemCallbackForward;
        internalRequest.vecFSRequests[i].pszPathID = pathID;
        internalRequest.vecFSRequests[i].pContext = internalRequest.handle;
    }

    if (filesystem->AsyncReadMultiple(internalRequest.vecFSRequests.Base(), count, internalRequest.vecControls.Base())
        != FSASYNC_OK)
    {
        Warning("Bad async replay load request!\n");
        AsyncCleanup(internalRequest.handle);
        return ASYNC_REPLAYS_FSERROR;
    }

    return ASYNC_REPLAYS_INPROGRESS;

}

AsyncReplayStatus_t CMomReplayFactory::AsyncLoadReplaysForMap(const char *mapName,
                                                              const MomLocalReplayCallback_t &callback,
                                                              MomReplayAsyncHandle &outHandle)
{
    char path[MAX_PATH];
    Q_snprintf(path, MAX_PATH, "%s/%s-*%s", RECORDING_PATH, mapName, EXT_RECORDING_FILE);
    Q_FixSlashes(path);

    FileFindHandle_t found;
    CUtlStringList filenames;
    const char *pFoundFile = filesystem->FindFirstEx(path, "MOD", &found);

    if (!pFoundFile)
    {
        filesystem->FindClose(found);
        return ASYNC_REPLAYS_NONEFOUND;
    }

    char pReplayPath[MAX_PATH];

    do
    {
        Q_ComposeFileName(RECORDING_PATH, pFoundFile, pReplayPath, MAX_PATH);
        filenames.CopyAndAddToTail(pReplayPath);

        pFoundFile = filesystem->FindNext(found);
    }
    while (pFoundFile);

    filesystem->FindClose(found);

    return AsyncLoadReplaysByFilename(filenames, "MOD", callback, outHandle);
}

void CMomReplayFactory::FilesystemCallbackForward(const FileAsyncRequest_t &request, int nBytesRead,
                                                  FSAsyncStatus_t status)
{
    g_ReplayFactory.FilesystemCallback(request, nBytesRead, status);
}

void CMomReplayFactory::FilesystemCallback(const FileAsyncRequest_t &request, int nBytesRead, FSAsyncStatus_t status)
{
    auto handle = reinterpret_cast<MomReplayAsyncHandle>(request.pContext);
    CAutoLock lock(m_requestMutex);
    int idx = FindRequestByHandle(handle);

    if (idx == -1)
    {
        return;
    }

    auto &intReq = m_vecRequests[idx];

    if (status != FSASYNC_OK)
    {
        // This shouldn't happen, we just cancel the whole operation
        AsyncCleanup(handle);
        return;
    }

    // Build Buffer and create replay

    CUtlBuffer reader(request.pData, nBytesRead, CUtlBuffer::READ_ONLY);
    reader.SeekPut(CUtlBuffer::SEEK_TAIL, 0); // The hash function will break otherwise!

    auto replay = LoadReplayFromBuffer(reader, false);

    if (replay)
    {
        replay->SetFilePath(request.pszFilename);
        intReq.replays.AddToTail(replay);
    }

    if (--(intReq.controls_pending) == 0)
    {
        // We are done here and ready to call back
        intReq.callback(handle);
    }
}

AsyncReplayStatus_t CMomReplayFactory::AsyncGetReplays(MomReplayAsyncHandle handle, CUtlVector<CMomReplayBase*> &vecToReturn)
{
    bool bLogReplay = IsReplayDebugEnabled();

    CAutoLock lock(m_requestMutex);
    int idx = FindRequestByHandle(handle);

    if (idx == -1)
    {
        return ASYNC_REPLAYS_BADREQUEST;
    }

    auto &intReq = m_vecRequests[idx];

    if (intReq.controls_pending != 0)
    {
        return ASYNC_REPLAYS_INPROGRESS;
    }

    vecToReturn.AddVectorToTail(intReq.replays);

    if (bLogReplay)
        Log("Asynchronous replays retrieved successfully!\n");

    AsyncCleanup(handle);

    return ASYNC_REPLAYS_READY;
}

void CMomReplayFactory::AsyncCleanup(const MomReplayAsyncHandle& handle)
{
    CAutoLock lock(m_requestMutex);
    int idx = FindRequestByHandle(handle);

    if (idx == -1)
    {
        return;
    }

    auto &intReq = m_vecRequests[idx];

    FOR_EACH_VEC(intReq.vecControls, i)
    {
        filesystem->AsyncAbort(intReq.vecControls[i]);
    }

    m_vecRequests.Remove(idx);
}

int CMomReplayFactory::FindRequestByHandle(MomReplayAsyncHandle handle)
{
    CAutoLock lock(m_requestMutex);
    FOR_EACH_VEC(m_vecRequests, i)
    {
        if (m_vecRequests[i].handle == handle)
        {
            return i;
        }
    }
    return -1;
}
#endif

CMomReplayFactory g_ReplayFactory;