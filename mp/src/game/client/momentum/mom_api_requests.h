#pragma once

#include "igamesystem.h"
#include "steam/steam_api_common.h"
#include "steam/isteamhttp.h"
#include "steam/isteamuser.h"
#include "utldelegate.h"

typedef CUtlDelegate<void (KeyValues *pKv)> CallbackFunc;

class CAPIRequests;

struct APIRequest
{
    APIRequest() : handle(INVALID_HTTPREQUEST_HANDLE), callResult(nullptr)
    {
        m_szURL[0] = '\0';
        m_szMethod[0] = '\0';
        m_szCallingFunc[0] = '\0';
        m_bSensitive = false;
        m_dSentTime = -1;
    }
    ~APIRequest()
    {
        if (callResult)
            delete callResult; // Should call cancel if still in progress
    }
    char m_szCallingFunc[256];
    char m_szURL[256];
    char m_szMethod[12];
    bool m_bSensitive;
    double m_dSentTime;
    HTTPRequestHandle handle;
    CallbackFunc callbackFunc;
    CCallResult<CAPIRequests, HTTPRequestCompleted_t> *callResult;
    bool operator==(const APIRequest &other) const
    {
        return handle == other.handle;
    }
};

struct DownloadRequest
{
    DownloadRequest() : handle(INVALID_HTTPREQUEST_HANDLE), completeResult(nullptr), m_bSaveToFile(true)
    {
        m_dSentTime = -1;
        m_szFileName[0] = '\0';
        m_szFilePathID[0] = '\0';
        m_szURL[0] = '\0';
    }

    ~DownloadRequest()
    {
        if (completeResult)
            delete completeResult;
    }
    HTTPRequestHandle handle;
    CCallResult<CAPIRequests, HTTPRequestCompleted_t> *completeResult;

    // The first function called, when the headers are returned for the download request.
    // Data in the response KeyValues:
    //  "request"   (uint64)    The request handle that the download operates under
    //  "size"      (uint64)    The size of the download, in bytes.
    CallbackFunc sizeFunc;
    // The second function (repeatedly) called when data is transferred from the server to the client.
    // Data in the response KeyValues:
    //  "request"   (uint64)    The request handle that the download operates under
    //  "percent"   (float)     The percent of download completion (NOTE: not very reliable for progress, use offset and size!)
    //  "offset"    (uint32)    The offset (from 0) of the bytes being downloaded
    //  "size"      (uint32)    The size of the chunk of data being downloaded, in bytes
    CallbackFunc progressFunc;
    // The last function to be called when downloading a file.
    // Data in the response KeyValues:
    //  "request"   (uint64)    The request handle that the download operates under
    //  "error"     (bool)      If the request fails in any way, will be true, otherwise false
    //  "code"      (int)       The HTTP status code of the request if it failed, otherwise 0
    //  "duration"  (float)     The amount of time in seconds it took to download the file
    //  "buf"       (pointer)   If the request was created with a nullptr filename, a pointer to the buffer is passed here
    CallbackFunc completeFunc;

    char m_szURL[256];
    char m_szFileName[MAX_PATH];
    char m_szFilePathID[16];
    bool m_bSaveToFile;
    double m_dSentTime;
    CUtlBuffer m_bufFileData;

    bool operator==(const DownloadRequest &other) const
    {
        return handle == other.handle;
    }
};

class CAPIRequests : public CAutoGameSystem
{
public:
    CAPIRequests();

    // ==== HTTP API METHODS ====
    // The way to call these is to create a delegate of a callback function, and pass that in.
    // Example:
    //      `g_pAPIRequests->GetMaps(nullptr, UtlMakeDelegate(this, &SomeClass::SomeCallbackFunc));`
    //      where SomeCallbackFunc is a void of SomeClass that takes a KeyValues pointer as the only parameter.
    // API requests will pass a KeyValues object (that will auto delete itself, no worries!) with:
    //      "code"              The integer HTTP status code returned. 0 if it's an IO error
    //      "URL"               The URL of the request
    //      "ping"              The RTT time it took to send the request and get the response, in milliseconds
    //      "method"            The method used for the request (GET, POST, etc)
    //      "data"              The response data, parsed JSON represented as KeyValues
    //      "error"             An error object, parsed JSON represented as KeyValues
    //          "err_parse"     If any parsing issue happens with JSON, it will be logged here as a string, inside error
    //
    // All API requests return `true` if the call succeeded in sending, else `false`.

    // ==== Auth ====
    // Sends an auth ticket to the server to get an API key to use for requests. 
    // This should be the first API call to make, and is done automatically on game boot.
    bool SendAuthTicket(CallbackFunc func);
    // Returns true if authenticated with the server
    bool IsAuthenticated() const;

    // ==== Maps ====
    bool GetMaps(KeyValues *pKvFilters, CallbackFunc func);
    bool GetMapInfo(uint32 mapID, CallbackFunc func);
    bool GetMapByName(const char *pMapName, CallbackFunc func);
    bool GetMapZones(uint32 uMapID, CallbackFunc func);
    bool GetUserMapLibrary(CallbackFunc func);
    // bAddToLibrary being false means "remove from library"
    bool SetMapInLibrary(uint32 mapID, bool bAddToLibrary, CallbackFunc func);
    bool GetUserMapFavorites(CallbackFunc func);
    // bAddToFavs being false means "remove from favorites"
    bool SetMapInFavorites(uint32 mapID, bool bAddToFavs, CallbackFunc func);

    // ==== Leaderboards ====
    // pKvFilters are run category filters (track, zone, flags, etc). Null is entire main track, no flags.
    bool GetTop10MapTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters = nullptr);
    bool GetFriendsTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters = nullptr);
    bool GetAroundTimes(uint32 mapID, CallbackFunc func, KeyValues *pKvFilters = nullptr);

    // Run Sessions
    bool InvalidateRunSession(uint32 mapID, CallbackFunc func);
    bool CreateRunSession(uint32 mapID, uint8 trackNum, uint8 zoneNum, CallbackFunc func);
    bool AddRunSessionTimestamp(uint32 mapID, uint64 sessionID, uint8 zoneNum, uint32 tick, CallbackFunc func);
    bool EndRunSession(uint32 mapID, uint64 sessionID, const CUtlBuffer &replayBuf, CallbackFunc func);

    // ==== Stats ====
    bool GetUserStats(uint64 profileID, CallbackFunc func); // Just user stats
    /**
     * Gets a user's profile stats as well as their map rank for a particular map.
     * @param profileID The profile to get the stats for. 0 will get the local user's stuff.
     * @param mapID     The map to get the rank stats for. 0 will just get the user's stats.
     * @param func      The callback function
     */
    bool GetUserStatsAndMapRank(uint64 profileID, uint32 mapID, CallbackFunc func);
    bool GetUserRunHistory(uint32 userID, CallbackFunc func, KeyValues *pKvFilters = nullptr);

    // ==== File Downloading ====
    /**
     * @param pszURL        The URL to the file
     * @param size          The size function of the download, see DownloadRequest for more info
     * @param prog          The progress function of the download, see DownloadRequest for more info
     * @param end           The complete function of the download, see DownloadRequest for more info
     * @param pFileName     The file name (including any extra pathing) of where the file should be stored.
     *                      If nullptr, the file will be downloaded to the buffer and passed through to the 
     *                      end CallbackFunc, fetched by `->GetPtr("buf");`, and will not be saved to disk.
     * @param pFilePathID   (Optional) The pathID of where the file should be stored. Defaults to "GAME".
     * @param bAuth         (Optional) Whether this request should be authenticated. Defaults to false.
     * @return The handle of the request, will be an invalid handle if the request fails
     */
    HTTPRequestHandle DownloadFile(const char *pszURL, CallbackFunc size, CallbackFunc prog, CallbackFunc end,
                                   const char *pFileName, const char *pFilePathID = "GAME", bool bAuth = false);

    /**
     * @param handle    The handle of the request to cancel
     * @return true if the download was cancelled, otherwise false
     */
    bool CancelDownload(HTTPRequestHandle handle);

protected:
    // CAutoGameSystem
    bool Init() OVERRIDE;
    void Shutdown() OVERRIDE;

    // Auth ticket impl
    STEAM_CALLBACK(CAPIRequests, OnAuthTicket, GetAuthSessionTicketResponse_t);
    void OnAuthHTTP(KeyValues *pResponse);
    void DoAuth();

    STEAM_CALLBACK(CAPIRequests, OnDownloadHTTPHeader, HTTPRequestHeadersReceived_t);
    STEAM_CALLBACK(CAPIRequests, OnDownloadHTTPData, HTTPRequestDataReceived_t);
    void OnDownloadHTTPComplete(HTTPRequestCompleted_t *pParam, bool bIO);

    // Base HTTP response method, the CallbackFunc is passed the JSON object here
    void OnHTTPResp(HTTPRequestCompleted_t *pParam, bool bIOFailure);
private:
    // Creates an HTTP request with the proper authorization header added. 
    // If bAuth = true, it will add the API key to the request, and will also return false if the key isn't set
    bool CreateAPIRequest(APIRequest *request, const char *pszURL, EHTTPMethod kMethod, bool bAuth = true, bool bSensitive = false);
    // Should be called after the HTTP request is prepared by the API calls (above)
    bool SendAPIRequest(APIRequest *request, CallbackFunc func, const char *pCallingFunction, bool bPrioritize = false);
    // Check the response for errors
    bool CheckAPIResponse(HTTPRequestCompleted_t *pCallback, bool bIOFailure);

    CUtlMap<HTTPRequestHandle, APIRequest*> m_mapAPICalls;
    CUtlMap<HTTPRequestHandle, DownloadRequest*> m_mapDownloadCalls;

    // Auth ticket impl
    HAuthTicket m_hAuthTicket;
    byte* m_bufAuthBuffer;
    uint32 m_iAuthActualSize;
    // API key for requests
    char *m_pAPIKey;
    // The header string for authorizing requests
    char m_szAPIKeyHeader[1024];
};

extern CAPIRequests *g_pAPIRequests;