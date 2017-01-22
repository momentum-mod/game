#include "cbase.h"

#include "filesystem.h"
#include "mom_run_poster.h"

extern IFileSystem *filesystem;

CRunPoster::CRunPoster() {}

CRunPoster::~CRunPoster() {}

void CRunPoster::Init()
{
    // We need to listen for "replay_save"
    ListenForGameEvent("replay_save");
}

void CRunPoster::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp("replay_save", pEvent->GetName()) && pEvent->GetBool("save"))
    {
        char filePath[MAX_PATH];
        const char *filename = pEvent->GetString("filename");
        Q_ComposeFileName(RECORDING_PATH, filename, filePath, MAX_PATH);
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
    }
}

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

static CRunPoster s_momRunposter;
CRunPoster *g_MOMRunPoster = &s_momRunposter;