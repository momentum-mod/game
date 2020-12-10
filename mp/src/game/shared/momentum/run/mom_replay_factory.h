#pragma once

#define REPLAY_MAGIC_LE 0x524D4F4D
#define REPLAY_MAGIC_BE 0x4D4F4D52
#define REPLAY_HASH_LENGTH 40

class CMomReplayBase;

#ifdef CLIENT_DLL
#include "filesystem.h"
#include "mom_shareddefs.h"
#include "utldelegate.h"

enum AsyncReplayStatus_t
{
    ASYNC_REPLAYS_READY,            // Everything was loaded correctly and is ready to be read
    ASYNC_REPLAYS_INPROGRESS,       // Filesystem is still busy
    ASYNC_REPLAYS_CANCELLED,        // The request was manually cancelled
    ASYNC_REPLAYS_NONEFOUND,        // There are no replays for the specified map
    ASYNC_REPLAYS_BADREQUEST,       // The request was ill-formed
    ASYNC_REPLAYS_FSERROR           // Every file failed due to a filesystem error
};

DECLARE_POINTER_HANDLE(MomReplayAsyncHandle);
typedef CUtlDelegate<void(MomReplayAsyncHandle)> MomLocalReplayCallback_t;

#endif

class CMomReplayFactory
{
  public:
    CMomReplayFactory();
    // ~CMomReplayFactory();

    // Pass 0 as a version number to get whatever the latest version is.
    CMomReplayBase *CreateEmptyReplay(uint8 version);
    CMomReplayBase *CreateReplay(uint8 version, CUtlBuffer &reader, bool bFullLoad);

    // Returns a replay file and constructs a versioned replay object.
    CMomReplayBase *LoadReplayFile(const char *pFileName, bool bFullLoad = true, const char *pPathID = "MOD");
    CMomReplayBase *LoadReplayFromBuffer(CUtlBuffer &reader, bool bFullLoad = true);

  private:
    bool IsReplayDebugEnabled();

    uint8 m_ucCurrentVersion;

    /* Asynchronous replay loading
     * The interaction should look like this:
     * call AsyncLoadReplaysByFilename/ForMap -> have the callback send a message to recipient (e.g. a panel)
     * -> get replay files by calling AsyncGetReplays (e.g. in a message_func)
     * If you have the callback use AsyncGetReplays directly keep in mind you're blocking the IO thread
     */
    // Client-only for now
#ifdef CLIENT_DLL
  public:
    AsyncReplayStatus_t AsyncLoadReplaysByFilename(const CUtlStringList &filenames, const char *pathID,
                                                   const MomLocalReplayCallback_t &callback,
                                                   MomReplayAsyncHandle &outHandle);
    AsyncReplayStatus_t AsyncLoadReplaysForMap(const char *mapName, const MomLocalReplayCallback_t &callback,
                                               MomReplayAsyncHandle &outHandle);

    // Returns all loaded files if finished. Otherwise, do nothing.
    AsyncReplayStatus_t AsyncGetReplays(MomReplayAsyncHandle handle, CUtlVector<CMomReplayBase*> &vecToReturn);

    void AsyncCleanup(const MomReplayAsyncHandle& handle);

  private:
    // NOTE: We can afford to use a fast mutex because the way fs invokes callbacks, we don't expect contention there
    // It's mostly so we don't delete everything while having a callback running
    struct MomReplayLoadRequestInternal
    {
        MomReplayAsyncHandle                handle;
        MomLocalReplayCallback_t            callback;
        CUtlVector<FSAsyncControl_t>        vecControls;        // We keep these so we can abort the requests if needed
        CInterlockedInt                     controls_pending;   // How many files have we not heard back from yet?
        CUtlVector<FileAsyncRequest_t>      vecFSRequests;
        CUtlVector<CMomReplayBase*>         replays;
    };

    CUtlVector<MomReplayLoadRequestInternal> m_vecRequests;
    CThreadMutex m_requestMutex; // While the documentation is sparse, this is a recursive mutex

    static void FilesystemCallbackForward(const FileAsyncRequest_t &request, int nBytesRead, FSAsyncStatus_t status);
    void FilesystemCallback(const FileAsyncRequest_t &request, int nBytesRead, FSAsyncStatus_t status);

    int FindRequestByHandle(MomReplayAsyncHandle handle);
#endif

};

extern CMomReplayFactory g_ReplayFactory;