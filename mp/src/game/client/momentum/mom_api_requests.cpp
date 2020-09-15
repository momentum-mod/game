#include "cbase.h"

#include "mom_api_requests.h"
#include "util/jsontokv.h"
#include "fmtstr.h"
#include "mom_shareddefs.h"
#include "filesystem.h"

#include "MessageboxPanel.h"

#include "tier0/memdbgon.h"

static MAKE_TOGGLE_CONVAR(mom_api_log_requests, "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, API requests will be logged to console.\n");
static MAKE_TOGGLE_CONVAR(mom_api_log_requests_sensitive, "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, API requests that are sensitive will also be logged to console.\n"
"!!!!!!! DANGER! Only set this if you know what you are doing! This could potentially expose an API key! !!!!!!!");
static ConVar mom_api_base_url("mom_api_base_url", "https://momentum-mod.org", FCVAR_ARCHIVE | FCVAR_REPLICATED, "The base URL for the API requests.\n");

#define API_REQ(url) CFmtStr1024("%s/api/%s", mom_api_base_url.GetString(), (url)).Get()
#define AUTH_REQ(url) CFmtStr1024("%s%s", mom_api_base_url.GetString(), (url)).Get()

#define NOT_IMPL AssertMsg(0, "API Request %s is not implemented yet!", __FUNCTION__); return false;

static const char* const s_pHTTPMethods[] = {
    "INVALID",
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "OPTIONS",
    "PATCH",
};

CAPIRequests::CAPIRequests() : CAutoGameSystem("CAPIRequests"), 
m_hAuthTicket(k_HAuthTicketInvalid), m_bufAuthBuffer(nullptr),
m_iAuthActualSize(0), m_pAPIKey(nullptr)
{
    m_szAPIKeyHeader[0] = '\0';
    SetDefLessFunc(m_mapAPICalls);
    SetDefLessFunc(m_mapDownloadCalls);
}

bool CAPIRequests::SendAuthTicket(CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, AUTH_REQ("/auth/steam/user"), k_EHTTPMethodPOST, false, true))
    {
        uint64 id = SteamUser()->GetSteamID().ConvertToUint64();
        SteamHTTP()->SetHTTPRequestHeaderValue(req->handle, "id", CFmtStr("%llu", id).Get());
        SteamHTTP()->SetHTTPRequestRawPostBody(req->handle, "application/octet-stream", m_bufAuthBuffer, m_iAuthActualSize);

        return SendAPIRequest(req, func, __FUNCTION__);
    }

    delete req;
    return false;
}

bool CAPIRequests::GetMaps(KeyValues *pKvFilters, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ("maps"), k_EHTTPMethodGET))
    {
        if (pKvFilters && !pKvFilters->IsEmpty())
        {
            FOR_EACH_VALUE(pKvFilters, pKvFilter)
                SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, pKvFilter->GetName(), pKvFilter->GetString());
        }

        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "expand", 
                                                      "info,thumbnail,credits,inLibrary,inFavorites,personalBest,worldRecord");

        return SendAPIRequest(req, func, __FUNCTION__);
    }

    delete req;
    return false;
}

bool CAPIRequests::GetTop10MapTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters/* = nullptr*/)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/ranks", mapID).Get()), k_EHTTPMethodGET))
    {
        if (pKvFilters)
        {
            FOR_EACH_VALUE(pKvFilters, pKvFilter)
                SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, pKvFilter->GetName(), pKvFilter->GetString());
        }
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetFriendsTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters/* = nullptr*/)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/ranks/friends", mapID).Get()), k_EHTTPMethodGET))
    {
        if (pKvFilters)
        {
            FOR_EACH_VALUE(pKvFilters, pKvFilter)
                SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, pKvFilter->GetName(), pKvFilter->GetString());
        }
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetAroundTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters/* = nullptr*/)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/ranks/around", mapID).Get()), k_EHTTPMethodGET))
    {
        if (pKvFilters)
        {
            FOR_EACH_VALUE(pKvFilters, pKvFilter)
                SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, pKvFilter->GetName(), pKvFilter->GetString());
        }
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetMapInfo(uint32 mapID, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u", mapID).Get()), k_EHTTPMethodGET))
    {
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "expand", "info,credits,inLibrary,inFavorites,submitter,images,personalBest,worldRecord");
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetMapByName(const char *pMapName, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ("maps"), k_EHTTPMethodGET))
    {
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "search", pMapName);
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "limit", "1");
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetMapZones(uint32 uMapID, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/zones", uMapID).Get()), k_EHTTPMethodGET))
    {
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetUserMapLibrary(CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ("user/maps/library"), k_EHTTPMethodGET))
    {
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "expand", "info,thumbnail,inFavorites,personalBest,worldRecord");
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "limit", "0");

        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::SetMapInLibrary(uint32 mapID, bool bAddToLibrary, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("user/maps/library/%u", mapID).Get()), bAddToLibrary ? k_EHTTPMethodPUT : k_EHTTPMethodDELETE))
    {
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetUserMapFavorites(CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ("user/maps/favorites"), k_EHTTPMethodGET))
    {
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "limit", "0");
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "expand", "info,inLibrary,worldRecord,personalBest");

        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::SetMapInFavorites(uint32 mapID, bool bAddToFavs, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("user/maps/favorites/%u", mapID).Get()), bAddToFavs ? k_EHTTPMethodPUT : k_EHTTPMethodDELETE))
    {
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::InvalidateRunSession(uint32 mapID, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/session", mapID).Get()), k_EHTTPMethodDELETE))
    {
        return SendAPIRequest(req, func, __FUNCTION__);
    }

    delete req;
    return false;
}

bool CAPIRequests::CreateRunSession(uint32 mapID, uint8 trackNum, uint8 zoneNum, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/session", mapID).Get()), k_EHTTPMethodPOST))
    {
        CFmtStr body("{\"trackNum\": %u, \"zoneNum\": %u}", trackNum, zoneNum);
        SteamHTTP()->SetHTTPRequestRawPostBody(req->handle, "application/json", (uint8*)body.Access(), body.Length());
        return SendAPIRequest(req, func, __FUNCTION__);
    }

    delete req;
    return false;
}

bool CAPIRequests::AddRunSessionTimestamp(uint32 mapID, uint64 sessionID, uint8 zoneNum, uint32 tick, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/session/%lld", mapID, sessionID).Get()), k_EHTTPMethodPOST))
    {
        CFmtStr body("{\"zoneNum\": %u, \"tick\": %u}", zoneNum, tick);
        SteamHTTP()->SetHTTPRequestRawPostBody(req->handle, "application/json", (uint8*) body.Access(), body.Length());
        return SendAPIRequest(req, func, __FUNCTION__);
    }

    delete req;
    return false;
}

bool CAPIRequests::EndRunSession(uint32 mapID, uint64 sessionID, const CUtlBuffer &replayBuf, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("maps/%u/session/%lld/end", mapID, sessionID).Get()), k_EHTTPMethodPOST))
    {
        SteamHTTP()->SetHTTPRequestRawPostBody(req->handle, "application/octet-stream", (uint8*) replayBuf.Base(), replayBuf.TellPut());
        return SendAPIRequest(req, func, __FUNCTION__, true);
    }

    delete req;
    return false;
}

bool CAPIRequests::GetUserStats(uint64 profileID, CallbackFunc func)
{
    return GetUserStatsAndMapRank(profileID, 0, func);
}

bool CAPIRequests::GetUserStatsAndMapRank(uint64 profileID, uint32 mapID, CallbackFunc func)
{
    APIRequest *req = new APIRequest;
    const auto pReqStr = profileID == 0 ? "user" : "users";
    if (CreateAPIRequest(req, API_REQ(pReqStr), k_EHTTPMethodGET))
    {
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "expand", "userStats");

        if (profileID != 0)
            SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "playerID", CFmtStr("%llu", profileID).Get());

        if (mapID != 0)
            SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "mapRank", CFmtStr("%u", mapID).Get());
        
        return SendAPIRequest(req, func, __FUNCTION__);
    }
    delete req;
    return false;
}

bool CAPIRequests::GetUserRunHistory(uint32 userID, CallbackFunc func, KeyValues *pKvFilters /*= nullptr*/)
{
    const auto req = new APIRequest;
    if (CreateAPIRequest(req, API_REQ(CFmtStr("users/%i/runs", userID).Get()), k_EHTTPMethodGET))
    {
        SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, "expand", "map");

        if (pKvFilters)
        {
            FOR_EACH_VALUE(pKvFilters, pKvFilter)
                SteamHTTP()->SetHTTPRequestGetOrPostParameter(req->handle, pKvFilter->GetName(), pKvFilter->GetString());
        }

        return SendAPIRequest(req, func, __FUNCTION__);
    }

    delete req;
    return false;
}

HTTPRequestHandle CAPIRequests::DownloadFile(const char* pszURL, CallbackFunc size, CallbackFunc prog, CallbackFunc end, 
                                             const char *pFileName, const char *pFilePathID /* = "GAME"*/, bool bAuth /*= false*/)
{
    HTTPRequestHandle handle = INVALID_HTTPREQUEST_HANDLE;
    APIRequest *req = new APIRequest;
    if (CreateAPIRequest(req, pszURL, k_EHTTPMethodGET, bAuth))
    {
        handle = req->handle;
        SteamAPICall_t apiHandle;
        if (SteamHTTP()->SendHTTPRequestAndStreamResponse(handle, &apiHandle))
        {
            DownloadRequest *callback = new DownloadRequest();
            callback->handle = handle;
            callback->sizeFunc = size;
            callback->progressFunc = prog;
            callback->completeFunc = end;
            callback->m_dSentTime = Plat_FloatTime();
            if (pFileName == nullptr)
            {
                callback->m_bSaveToFile = false;
            }
            else
            {
                V_FixupPathName(callback->m_szFileName, sizeof(callback->m_szFileName), pFileName);
                Q_strncpy(callback->m_szFilePathID, pFilePathID, sizeof(callback->m_szFilePathID));
            }
            callback->completeResult = new CCallResult<CAPIRequests, HTTPRequestCompleted_t>();
            callback->completeResult->Set(apiHandle, this, &CAPIRequests::OnDownloadHTTPComplete);
            m_mapDownloadCalls.Insert(handle, callback);
        }
        else
        {
            Warning("%s --- Failed to send HTTP request for downloading!\n", __FUNCTION__);
            SteamHTTP()->ReleaseHTTPRequest(handle); // GC
            handle = INVALID_HTTPREQUEST_HANDLE;
        }
    }

    delete req;
    return handle;
}

bool CAPIRequests::CancelDownload(HTTPRequestHandle handle)
{
    const auto downloadCallbackIndx = m_mapDownloadCalls.Find(handle);
    if (downloadCallbackIndx != m_mapDownloadCalls.InvalidIndex())
    {
        HTTPRequestCompleted_t mock;
        mock.m_hRequest = handle;
        mock.m_bRequestSuccessful = false;
        mock.m_eStatusCode = k_EHTTPStatusCode410Gone;

        OnDownloadHTTPComplete(&mock, true);
        return true;
    }
    return false;
}

bool CAPIRequests::Init()
{
    DoAuth();

    return true;
}

void CAPIRequests::Shutdown()
{
    if (m_hAuthTicket != k_HAuthTicketInvalid)
    {
        SteamUser()->CancelAuthTicket(m_hAuthTicket);
        delete[] m_bufAuthBuffer;
    }

    if (m_pAPIKey)
    {
        delete[] m_pAPIKey;
    }

    // This also cancels any outstanding API/download requests
    m_mapAPICalls.PurgeAndDeleteElements();
    m_mapDownloadCalls.PurgeAndDeleteElements();
}

void CAPIRequests::OnAuthTicket(GetAuthSessionTicketResponse_t* pParam)
{
    if (pParam->m_eResult == k_EResultOK)
    {
        if (pParam->m_hAuthTicket == m_hAuthTicket)
        {
            // Send it to the site
            SendAuthTicket(UtlMakeDelegate(this, &CAPIRequests::OnAuthHTTP));
        }
    }
}

void CAPIRequests::OnAuthHTTP(KeyValues *pResponse)
{
    IGameEvent *pEvent = gameeventmanager->CreateEvent("site_auth");
    bool bSuccess = false;

    KeyValues *pData = pResponse->FindKey("data");
    KeyValues *pErr = pResponse->FindKey("error");
    if (pData)
    {
        uint32 tokenLength = pData->GetInt("length");

        if (tokenLength)
        {
            m_pAPIKey = new char[tokenLength + 1];
            Q_strncpy(m_pAPIKey, pData->GetString("token"), tokenLength + 1);
            Q_snprintf(m_szAPIKeyHeader, sizeof(m_szAPIKeyHeader), "Bearer %s", m_pAPIKey);
            bSuccess = true;
        }
    }
    else if (pErr)
    {
        Warning("Error when trying to get an API key!\n");
        const auto iCode = pResponse->GetInt("code");
        if (iCode == 403)
        {
            const auto pErrorObj = pErr->FindKey("error");
            if (pErrorObj)
            {
                g_pMessageBox->CreateMessagebox("Error Authenticating", pErrorObj->GetString("message"));
            }
        }
    }

    if (pEvent)
    {
        pEvent->SetBool("success", bSuccess);
        gameeventmanager->FireEventClientSide(pEvent);
    }
}

void CAPIRequests::DoAuth()
{
    if (m_pAPIKey)
        delete[] m_pAPIKey;
    m_pAPIKey = nullptr;

    if (m_bufAuthBuffer)
        delete[] m_bufAuthBuffer;
    m_bufAuthBuffer = nullptr;

    m_szAPIKeyHeader[0] = '\0';

    CHECK_STEAM_API(SteamUser());

    m_bufAuthBuffer = new byte[1024];
    m_hAuthTicket = SteamUser()->GetAuthSessionTicket(m_bufAuthBuffer, 1024, &m_iAuthActualSize);
    if (m_hAuthTicket == k_HAuthTicketInvalid)
        Warning("Initial call to get the ticket failed!\n");
}

bool CAPIRequests::IsAuthenticated() const
{
    return m_pAPIKey != nullptr;
}

void CAPIRequests::OnDownloadHTTPHeader(HTTPRequestHeadersReceived_t* pCallback)
{
    const uint16 downloadCallbackIndx = m_mapDownloadCalls.Find(pCallback->m_hRequest);
    if (downloadCallbackIndx != m_mapDownloadCalls.InvalidIndex())
    {
        uint32 size;
        if (SteamHTTP()->GetHTTPResponseHeaderSize(pCallback->m_hRequest, "Content-Length", &size))
        {
            // Now read the actual data
            uint8 *pData = new uint8[size + 1];
            if (SteamHTTP()->GetHTTPResponseHeaderValue(pCallback->m_hRequest, "Content-Length", pData, size))
            {
                // Null-terminate
                pData[size] = 0;
                const uint64 fileSize = Q_atoui64(reinterpret_cast<const char *>(pData));

                if (fileSize)
                {
                    KeyValuesAD headers("Headers");
                    headers->SetUint64("request", pCallback->m_hRequest);
                    headers->SetUint64("size", fileSize);

                    DownloadRequest *call = m_mapDownloadCalls[downloadCallbackIndx];
                    call->m_bufFileData.EnsureCapacity(fileSize);
                    call->sizeFunc(headers);
                }
            }

            delete[] pData;
            pData = nullptr;
        }
    }
}

void CAPIRequests::OnDownloadHTTPData(HTTPRequestDataReceived_t* pCallback)
{
    const uint16 downloadCallbackIndx = m_mapDownloadCalls.Find(pCallback->m_hRequest);
    if (downloadCallbackIndx != m_mapDownloadCalls.InvalidIndex())
    {
        uint8 *pDataTemp = new uint8[pCallback->m_cBytesReceived];
        if (SteamHTTP()->GetHTTPStreamingResponseBodyData(pCallback->m_hRequest, pCallback->m_cOffset, pDataTemp, pCallback->m_cBytesReceived))
        {
            DownloadRequest *call = m_mapDownloadCalls[downloadCallbackIndx];
            // Add the data to the download buffer
            call->m_bufFileData.Put(pDataTemp, pCallback->m_cBytesReceived);

            KeyValuesAD prog("Progress");
            prog->SetUint64("request", pCallback->m_hRequest);
            float percent = 0.0f;
            if (SteamHTTP()->GetHTTPDownloadProgressPct(pCallback->m_hRequest, &percent))
                prog->SetFloat("percent", percent);
            prog->SetInt("offset", pCallback->m_cOffset);
            prog->SetInt("size", pCallback->m_cBytesReceived);
            call->progressFunc(prog);
        }

        delete[] pDataTemp;
        pDataTemp = nullptr;
    }
}

void CAPIRequests::OnDownloadHTTPComplete(HTTPRequestCompleted_t* pCallback, bool bIO)
{
    const uint16 downloadCallbackIndx = m_mapDownloadCalls.Find(pCallback->m_hRequest);
    if (downloadCallbackIndx != m_mapDownloadCalls.InvalidIndex())
    {
        DownloadRequest *call = m_mapDownloadCalls[downloadCallbackIndx];

        KeyValuesAD comp("Complete");
        comp->SetUint64("request", pCallback->m_hRequest);
        comp->SetFloat("duration", Plat_FloatTime() - call->m_dSentTime);
        if (bIO || !pCallback->m_bRequestSuccessful || pCallback->m_eStatusCode != k_EHTTPStatusCode200OK)
        {
            comp->SetBool("error", true);
            comp->SetInt("code", pCallback->m_eStatusCode);
            comp->SetBool("bIO", bIO);
        }
        else if (call->m_bSaveToFile)
        {
            bool bWrote = g_pFullFileSystem->WriteFile(call->m_szFileName, call->m_szFilePathID, call->m_bufFileData);
            comp->SetBool("error", !bWrote);
        }
        else
        {
            comp->SetPtr("buf", &call->m_bufFileData);
        }
        call->completeFunc(comp);
        m_mapDownloadCalls.RemoveAt(downloadCallbackIndx);
        delete call;
    }

    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void CAPIRequests::OnHTTPResp(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    // Firstly, let's find the callback that corresponds to the API request we made
    const auto callbackIndx = m_mapAPICalls.Find(pCallback->m_hRequest);
    if (callbackIndx != m_mapAPICalls.InvalidIndex())
    {
        // Okay cool, callback found
        APIRequest *req = m_mapAPICalls[callbackIndx];

        // Let's create our main KeyValues object to operate on and properly clean it up out of this scope
        KeyValuesAD response(req->m_szCallingFunc);
        response->UsesEscapeSequences(true);

        // Secondly, let's set the code, method, URL, and ping of the response. Even if it's an IO error.
        response->SetInt("code", pCallback->m_eStatusCode);
        response->SetString("method", req->m_szMethod);
        response->SetString("URL", req->m_szURL);
        response->SetString("ping", CFmtStr("%.3f ms", (Plat_FloatTime() - req->m_dSentTime) * 1000.0f));

        // Thirdly, check if there are any errors
        bool bRequestOK = CheckAPIResponse(pCallback, bIOFailure);

        // Fourthly, knowing if there's an error or not, create the proper data
        KeyValues *pKvBodyData = new KeyValues(bRequestOK ? "data" : "error");
        if (pCallback->m_unBodySize > 0)
        {
            // Fourthly-A, read the body properly
            uint8 *pData = new uint8[pCallback->m_unBodySize + 1];
            SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, pCallback->m_unBodySize);
            pData[pCallback->m_unBodySize] = 0; // Make sure to null terminate

            // Fourthly-B, parse this JSON and convert to KeyValues
            char *pDataPtr = reinterpret_cast<char*>(pData);
            if (!CJsonToKeyValues::ConvertJsonToKeyValues(pDataPtr, pKvBodyData))
            {
                pKvBodyData->SetName("error"); // Ensure it's passed as an error
                Warning("Failed to parse! %s\n", pKvBodyData->GetString("err_parse"));
            }

            // Fourthly-C, free our data buffer 
            delete[] pData;
            pData = nullptr;
            pDataPtr = nullptr;
        } // "else 0 body size" -- it's valid, but it'll be empty. Reading a 204 can still be done here

        // Fifthly, add our new body data
        response->AddSubKey(pKvBodyData);

        // Log out the response if desired
        if (mom_api_log_requests.GetBool())
        {
            CKeyValuesDumpContextAsDevMsg dump(0);

            if (req->m_bSensitive && !mom_api_log_requests_sensitive.GetBool())
            {
                // If a request is sensitive, we censor the data in the log
                KeyValuesAD sensitive(response->MakeCopy());
                // Only need to clear data, not errors
                KeyValues *pData = sensitive->FindKey("data");
                if (pData)
                {
                    pData->Clear();
                    pData->SetString("Censored", "for your own sake");
                    pData->SetString("To", "uncensor, use the command \"mom_api_log_requests_sensitive 1\"");
                }

                sensitive->Dump(&dump);
            }
            else
            {
                // Log it like normal
                response->Dump(&dump);
            }
        }

        // Sixthly, actually call the callback. It should be reading the body by using `pKvResponse->FindKey("data")`
        // or any errors by using `pKvResponse->FindKey("error")`
        req->callbackFunc(response);

        // And remove it from the map
        m_mapAPICalls.RemoveAt(callbackIndx);
        // And delete it (no memory leak pls)
        delete req;
        // And delete the response KeyValu- oh right the AutoDelete handles that here (out of scope)
    }
    else
    {
        // Should never happen... but you never know. It's <current year>.
        Warning("Somehow APIRequests caught an unmapped HTTP request!\n");
    }

    // And finally free the request
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

bool CAPIRequests::CreateAPIRequest(APIRequest *request, const char* pszURL, EHTTPMethod kMethod, bool bAuth /* = true*/, bool bSensitive /* = false*/)
{
    if (!SteamHTTP() || !request)
        return false;

    if (!pszURL)
        return false;

    if (bAuth && !m_pAPIKey)
        return false;

    Q_strncpy(request->m_szMethod, s_pHTTPMethods[kMethod], sizeof(request->m_szMethod));
    Q_strncpy(request->m_szURL, pszURL, sizeof(request->m_szURL));
    request->handle = SteamHTTP()->CreateHTTPRequest(kMethod, pszURL);
    request->m_bSensitive = bSensitive;

    // Add the API key
    if (bAuth)
        SteamHTTP()->SetHTTPRequestHeaderValue(request->handle, "Authorization", m_szAPIKeyHeader);

    return request->handle != INVALID_HTTPREQUEST_HANDLE;
}

bool CAPIRequests::SendAPIRequest(APIRequest *req, CallbackFunc func, const char* pCallingFunc, bool bPrioritize /*= false*/)
{
    SteamAPICall_t apiHandle;
    if (SteamHTTP()->SendHTTPRequest(req->handle, &apiHandle))
    {
        if (bPrioritize)
            SteamHTTP()->PrioritizeHTTPRequest(req->handle);

        req->m_dSentTime = Plat_FloatTime();
        Q_strncpy(req->m_szCallingFunc, pCallingFunc, sizeof(req->m_szCallingFunc));
        req->callbackFunc = func;
        req->callResult = new CCallResult<CAPIRequests, HTTPRequestCompleted_t>();
        req->callResult->Set(apiHandle, this, &CAPIRequests::OnHTTPResp);
        m_mapAPICalls.Insert(req->handle, req);
        return true;
    }

    Warning("%s --- Failed to send HTTP Request!\n", pCallingFunc);
    SteamHTTP()->ReleaseHTTPRequest(req->handle); // GC
    delete req;
    return false;
}

bool CAPIRequests::CheckAPIResponse(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    return pCallback->m_eStatusCode >= k_EHTTPStatusCode200OK &&
        pCallback->m_eStatusCode < k_EHTTPStatusCode300MultipleChoices && 
        !bIOFailure && pCallback->m_bRequestSuccessful;
}

CAPIRequests s_APIRequests;
CAPIRequests *g_pAPIRequests = &s_APIRequests;