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
    return false;
}

bool CAPIRequests::GetMaps(KeyValues *pKvFilters, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::GetTop10MapTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters/* = nullptr*/)
{
    return false;
}

bool CAPIRequests::GetFriendsTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters/* = nullptr*/)
{
    return false;
}

bool CAPIRequests::GetAroundTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters/* = nullptr*/)
{
    return false;
}

bool CAPIRequests::GetMapInfo(uint32 mapID, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::GetMapByName(const char *pMapName, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::GetMapZones(uint32 uMapID, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::GetMapTrickData(uint32 uMapID, CallbackFunc func)
{
    NOT_IMPL;
}

bool CAPIRequests::GetUserMapLibrary(CallbackFunc func)
{
    return false;
}

bool CAPIRequests::SetMapInLibrary(uint32 mapID, bool bAddToLibrary, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::GetUserMapFavorites(CallbackFunc func)
{
    return false;
}

bool CAPIRequests::SetMapInFavorites(uint32 mapID, bool bAddToFavs, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::InvalidateRunSession(uint32 mapID, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::CreateRunSession(uint32 mapID, uint8 trackNum, uint8 zoneNum, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::AddRunSessionTimestamp(uint32 mapID, uint64 sessionID, uint8 zoneNum, uint32 tick, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::EndRunSession(uint32 mapID, uint64 sessionID, const CUtlBuffer &replayBuf, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::GetUserStats(uint64 profileID, CallbackFunc func)
{
    return GetUserStatsAndMapRank(profileID, 0, func);
}

bool CAPIRequests::GetUserStatsAndMapRank(uint64 profileID, uint32 mapID, CallbackFunc func)
{
    return false;
}

bool CAPIRequests::GetUserRunHistory(uint32 userID, CallbackFunc func, KeyValues *pKvFilters /*= nullptr*/)
{
    return false;
}

HTTPRequestHandle CAPIRequests::DownloadFile(const char* pszURL, CallbackFunc size, CallbackFunc prog, CallbackFunc end, 
                                             const char *pFileName, const char *pFilePathID /* = "GAME"*/, bool bAuth /*= false*/)
{
    return INVALID_HTTPREQUEST_HANDLE;
}

bool CAPIRequests::CancelDownload(HTTPRequestHandle handle)
{
    return false;
}

bool CAPIRequests::Init()
{
    return true;
}

void CAPIRequests::Shutdown()
{
}

void CAPIRequests::OnAuthTicket(GetAuthSessionTicketResponse_t* pParam)
{
}

void CAPIRequests::OnAuthHTTP(KeyValues *pResponse)
{

}

void CAPIRequests::DoAuth()
{

}

bool CAPIRequests::IsAuthenticated() const
{
    return false;
}

void CAPIRequests::OnDownloadHTTPHeader(HTTPRequestHeadersReceived_t* pCallback)
{
}

void CAPIRequests::OnDownloadHTTPData(HTTPRequestDataReceived_t* pCallback)
{
}

void CAPIRequests::OnDownloadHTTPComplete(HTTPRequestCompleted_t* pCallback, bool bIO)
{
}

void CAPIRequests::OnHTTPResp(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{

}

bool CAPIRequests::CreateAPIRequest(APIRequest *request, const char* pszURL, EHTTPMethod kMethod, bool bAuth /* = true*/, bool bSensitive /* = false*/)
{
    return false;
}

bool CAPIRequests::SendAPIRequest(APIRequest *req, CallbackFunc func, const char* pCallingFunc, bool bPrioritize /*= false*/)
{
    return false;
}

bool CAPIRequests::CheckAPIResponse(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    return false;
}

CAPIRequests s_APIRequests;
CAPIRequests *g_pAPIRequests = &s_APIRequests;