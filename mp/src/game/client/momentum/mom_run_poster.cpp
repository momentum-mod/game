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
        SteamAPICall_t findCall = steamapicontext->SteamUserStats()->FindLeaderboard(pMapName);
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

        SteamAPICall_t uploadScore = steamapicontext->SteamUserStats()->UploadLeaderboardScore(m_hCurrentLeaderboard, 
            k_ELeaderboardUploadScoreMethodKeepBest, runTime, nullptr, 0);
        m_cLeaderboardScoreUploaded.Set(uploadScore, this, &CRunPoster::OnLeaderboardScoreUploaded);

        // MOM_TODO: Append the replay file to the leaderboard entry
        /*char filePath[MAX_PATH];
        const char *filename = pEvent->GetString("filename");
        Q_ComposeFileName(RECORDING_PATH, filename, filePath, MAX_PATH);

        // Get the full path to the recording
        char fullPath[MAX_PATH];
        g_pFullFileSystem->RelativePathToFullPath_safe(filePath, "MOD", fullPath);

        // Upload this bad boy
        //steamapicontext->SteamUGC()->CreateItem(steamapicontext->SteamUtils()->GetAppID(), k_EWorkshopFileTypeGameManagedItem);
        // OR
        SteamAPICall_t UGCcall = steamapicontext->SteamRemoteStorage()->FileShare(fullPath); // Not sure if this is right...
        m_cUGCUploaded.Set(UGCcall, this, &OnUGCUploaded);*/

#if ENABLE_HTTP_LEADERBOARDS
        CUtlBuffer buf;
        if (steamapicontext && steamapicontext->SteamHTTP() && filesystem->ReadFile(filePath, "MOD", buf))
        {
            char szURL[MAX_PATH] = "http://momentum-mod.org/postscore/";
            HTTPRequestHandle handle = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodPOST, szURL);
            int size = buf.Size();
            char *data = new char[size];
            buf.Get(data, size);
            steamapicontext->SteamHTTP()->SetHTTPRequestGetOrPostParameter(handle, "file", data);

            SteamAPICall_t apiHandle;

            if (steamapicontext->SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
            {
                Warning("Run sent.\n");
                (&cbPostTimeCallback)->Set(apiHandle, this, &CRunPoster::PostTimeCallback);
            }
            else
            {
                Warning("Failed to send HTTP Request to send run!\n");
                steamapicontext->SteamHTTP()->ReleaseHTTPRequest(handle); // GC
            }
        }
#endif
    }
}

void CRunPoster::OnLeaderboardFind(LeaderboardFindResult_t* pResult, bool bIOFailure)
{
    if (bIOFailure || !pResult->m_bLeaderboardFound)
    {
        Warning("No leaderboard found for map %s!\n", MapName());
        return;
    }

    m_hCurrentLeaderboard = pResult->m_hSteamLeaderboard;
}

void CRunPoster::OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t* pResult, bool bIOFailure)
{
    IGameEvent *pEvent = gameeventmanager->CreateEvent("run_upload");

    if (bIOFailure || !pResult->m_bSuccess)
    {
        pEvent->SetBool("run_posted", false);
        // MOM_TODO: If it didn't upload, hijack the run_upload event with a message here?
        return;
    }

    pEvent->SetBool("run_posted", true);
    if (gameeventmanager->FireEvent(pEvent))
        ConColorMsg(Color(0, 255, 0, 255), "Uploaded run to the leaderboards, check it out!\n");
    // Still need to upload UGC... (already taken care of in FireEvent)
}

void CRunPoster::OnLeaderboardUGCSet(LeaderboardUGCSet_t* pResult, bool bIOFailure)
{
    if (bIOFailure || pResult->m_eResult != k_EResultOK)
    {
        Warning("Failed to upload replay file to leaderboard! Result: %i\n", pResult->m_eResult);
        return;
    }

    // MOM_TODO: This is chronologically the last step to uploading a run. Fire the event here?
    //ConColorMsg(Color(0, 255, 0, 255), "Uploaded replay file to leaderboards, check it out!\n");
}

void CRunPoster::OnUGCUploaded(RemoteStorageFileShareResult_t* pResult, bool bIOFailure)
{
    if (bIOFailure || pResult->m_eResult != k_EResultOK)
    {
        Warning("Could not upload user replay file! Result %i\n", pResult->m_eResult);
        return;
    }

    // Now we attach to the leaderboard
    //SteamAPICall_t UGCLeaderboardCall = steamapicontext->SteamUserStats()->AttachLeaderboardUGC(m_hCurrentLeaderboard, pResult->m_hFile);
}

#if ENABLE_HTTP_LEADERBOARDS
void CRunPoster::PostTimeCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    // if (bIOFailure)
    //    return;
    // uint32 size;
    // steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    // if (size == 0)
    //{
    //    Warning("MomentumUtil::PostTimeCallback: 0 body size!\n");
    //    return;
    //}
    // DevLog("Size of body: %u\n", size);
    // uint8 *pData = new uint8[size];
    // steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

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
    // steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}
#endif

static CRunPoster s_momRunposter;
CRunPoster *g_MOMRunPoster = &s_momRunposter;