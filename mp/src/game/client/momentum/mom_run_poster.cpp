#include "cbase.h"

#include "filesystem.h"
#include "mom_run_poster.h"
#include "mom_shareddefs.h"

#include <tier0/memdbgon.h>

extern IFileSystem *filesystem;

#define ENABLE_HTTP_LEADERBOARDS 0

CRunPoster::CRunPoster(): m_hCurrentLeaderboard(0)
{
}

CRunPoster::~CRunPoster() {}

void CRunPoster::PostInit()
{
    // We need to listen for "replay_save"
    ListenForGameEvent("replay_save");
}

void CRunPoster::LevelInitPostEntity()
{
    const char *pMapName = MapName();
    if (pMapName)
    {
        CHECK_STEAM_API(SteamUserStats());
        SteamAPICall_t findCall = SteamUserStats()->FindOrCreateLeaderboard(pMapName, k_ELeaderboardSortMethodAscending, k_ELeaderboardDisplayTypeTimeMilliSeconds);
        m_cLeaderboardFindResult.Set(findCall, this, &CRunPoster::OnLeaderboardFind);
    }
}

void CRunPoster::LevelShutdownPreClearSteamAPIContext()
{
    m_hCurrentLeaderboard = 0;
}

void CRunPoster::FireGameEvent(IGameEvent *pEvent)
{
    if (pEvent->GetBool("save"))
    {
        CHECK_STEAM_API(SteamUserStats());

        if (!m_hCurrentLeaderboard)
        {
            Warning("Could not upload run: leaderboard doesn't exist!\n");
            // MOM_TODO: Make the run_posted event here with the above message?
            return;
        }

        // Upload the score
        int runTime = pEvent->GetInt("time"); // Time in milliseconds
        if (!runTime)
        {
           Warning("Could not upload run: time is 0 milliseconds!\n");
           // MOM_TODO: Make the run_posted event here with the above message?
           return;
        }

        // Save the name and path for uploading in the callback of the score
        Q_strncpy(m_szFileName, pEvent->GetString("filename"), MAX_PATH);
        Q_strncpy(m_szFilePath, pEvent->GetString("filepath"), MAX_PATH);

        // Set our score
        SteamAPICall_t uploadScore = SteamUserStats()->UploadLeaderboardScore(m_hCurrentLeaderboard, 
            k_ELeaderboardUploadScoreMethodKeepBest, runTime, nullptr, 0);
        m_cLeaderboardScoreUploaded.Set(uploadScore, this, &CRunPoster::OnLeaderboardScoreUploaded);


#if ENABLE_HTTP_LEADERBOARDS
        CUtlBuffer buf;
        if (SteamHTTP() && filesystem->ReadFile(filePath, "MOD", buf))
        {
            char szURL[MAX_PATH] = "http://momentum-mod.org/postscore/";
            HTTPRequestHandle handle = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodPOST, szURL);
            int size = buf.Size();
            char *data = new char[size];
            buf.Get(data, size);
            SteamHTTP()->SetHTTPRequestGetOrPostParameter(handle, "file", data);

            SteamAPICall_t apiHandle;

            if (SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
            {
                Warning("Run sent.\n");
                (&cbPostTimeCallback)->Set(apiHandle, this, &CRunPoster::PostTimeCallback);
            }
            else
            {
                Warning("Failed to send HTTP Request to send run!\n");
                SteamHTTP()->ReleaseHTTPRequest(handle); // GC
            }
        }
#endif
    }
}

void CRunPoster::OnLeaderboardFind(LeaderboardFindResult_t* pResult, bool bIOFailure)
{
    if (bIOFailure)
    {
        Warning("Failed to create leaderboard for map %s!\n", MapName());
        return;
    }

    m_hCurrentLeaderboard = pResult->m_hSteamLeaderboard;
}

void CRunPoster::OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t* pResult, bool bIOFailure)
{
    IGameEvent *pEvent = gameeventmanager->CreateEvent("run_upload");

    bool bSuccess = true;
    if (bIOFailure || !pResult->m_bSuccess)
    {
        bSuccess = false;
        // MOM_TODO: If it didn't upload, hijack the run_upload event with a message here?
    }

    if (pEvent)
    {
        pEvent->SetBool("run_posted", bSuccess);

        if (gameeventmanager->FireEvent(pEvent))
        {
            if (bSuccess)
            {
                // Now we can (try to) upload this replay file to the Steam Cloud for attaching to this new leaderboard score
                CUtlBuffer fileBuffer;
                if (filesystem->ReadFile(m_szFilePath, "MOD", fileBuffer))
                {
                    SteamAPICall_t write = SteamRemoteStorage()->FileWriteAsync(m_szFileName, fileBuffer.Base(), fileBuffer.TellPut());
                    m_cFileUploaded.Set(write, this, &CRunPoster::OnFileUploaded);
                }
                else
                {
                    DevWarning("Couldn't read replay file %s!\n", m_szFilePath);
                }

                ConColorMsg(Color(0, 255, 0, 255), "Uploaded run to the leaderboards, check it out!\n");
            }
            else
            {
                Warning("Could not upload your leaderboard score, sorry!\n");
            }
        }
    }
}

void CRunPoster::OnLeaderboardUGCSet(LeaderboardUGCSet_t* pResult, bool bIOFailure)
{
    bool bSuccess = true;
    if (bIOFailure || pResult->m_eResult != k_EResultOK)
    {
        bSuccess = false;
        Warning("Failed to upload replay file to leaderboard! Result: %i\n", pResult->m_eResult);
    }

    // Either way we need to delete the file from Steam Cloud now, don't use quota
    if (SteamRemoteStorage()->FileDelete(m_szFileName))
    {
        DevLog("Successfully deleted the uploaded run on the Steam Cloud at %s\n", m_szFileName);
    }

    // Clear out the paths here
    m_szFileName[0] = 0;
    m_szFilePath[0] = 0;

    if (bSuccess)
        ConColorMsg(Color(0, 255, 0, 255), "Uploaded replay file to leaderboards, check it out!\n");
}

void CRunPoster::OnFileUploaded(RemoteStorageFileWriteAsyncComplete_t* pResult, bool bIOFailure)
{
    if (pResult->m_eResult != k_EResultOK || bIOFailure)
    {
        Warning("Could not upload steam cloud file! Result: %i\n", pResult->m_eResult);
        return;
    }

    SteamAPICall_t UGCcall = SteamRemoteStorage()->FileShare(m_szFileName);
    m_cFileShared.Set(UGCcall, this, &CRunPoster::OnFileShared);
}

void CRunPoster::OnFileShared(RemoteStorageFileShareResult_t* pResult, bool bIOFailure)
{
    if (bIOFailure || pResult->m_eResult != k_EResultOK)
    {
        Warning("Could not upload user replay file! Result %i\n", pResult->m_eResult);
        return;
    }

    // Now we attach to the leaderboard
    SteamAPICall_t UGCLeaderboardCall = SteamUserStats()->AttachLeaderboardUGC(m_hCurrentLeaderboard, pResult->m_hFile);
    m_cLeaderboardUGCSet.Set(UGCLeaderboardCall, this, &CRunPoster::OnLeaderboardUGCSet);
}

#if ENABLE_HTTP_LEADERBOARDS
void CRunPoster::PostTimeCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    // if (bIOFailure)
    //    return;
    // uint32 size;
    // SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    // if (size == 0)
    //{
    //    Warning("MomentumUtil::PostTimeCallback: 0 body size!\n");
    //    return;
    //}
    // DevLog("Size of body: %u\n", size);
    // uint8 *pData = new uint8[size];
    // SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    // IGameEvent *runUploadedEvent = gameeventmanager->CreateEvent("run_upload");

    // JsonValue val; // Outer object
    // JsonAllocator alloc;
    // char *pDataPtr = reinterpret_cast<char *>(pData);
    // DevLog("pDataPtr: %s\n", pDataPtr);
    // char *endPtr;
    // int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    // if (status == JSON_OK)
    //{
    //    DevLog("JSON Parsed!\n");
    //    if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
    //    {
    //        // toNode() returns the >>payload<< of the JSON object !!!

    //        DevLog("Outer is JSON OBJECT!\n");
    //        JsonNode *node = val.toNode();
    //        DevLog("Outer has key %s with value %s\n", node->key, node->value);

    //        // MOM_TODO: This doesn't work, even if node has tag 'true'. Something is wrong with the way we are
    //        parsing
    //        // the JSON
    //        if (node && node->value.getTag() == JSON_TRUE)
    //        {
    //            DevLog("RESPONSE WAS TRUE!\n");
    //            // Necessary so that the leaderboards and hud_mapfinished update appropriately
    //            if (runUploadedEvent)
    //            {
    //                runUploadedEvent->SetBool("run_posted", true);
    //                // MOM_TODO: Once the server updates this to contain more info, parse and do more with the
    //                response
    //                gameeventmanager->FireEvent(runUploadedEvent);
    //            }
    //        }
    //    }
    //}
    // else
    //{
    //    Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    //}
    //// Last but not least, free resources
    // alloc.deallocate();
    // if (pDataPtr)
    //{
    //    delete[] pDataPtr;
    //}
    // pDataPtr = nullptr;
    // SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}
#endif

static CRunPoster s_momRunposter;
CRunPoster *g_MOMRunPoster = &s_momRunposter;