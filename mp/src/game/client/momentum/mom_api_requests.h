#pragma once

#include "igamesystem.h"
#include "steam/steam_api.h"
#include "utldelegate.h"

typedef CUtlDelegate<void (KeyValues *pKv)> CallbackFunc;

class CAPIRequests : public CAutoGameSystemPerFrame
{
public:
    CAPIRequests();

    // === HTTP API METHODS ===
    // The way to call these is to create a delegate of a callback function, and pass that in.
    // Example:
    //      `g_pAPIRequests->GetMaps(UtlMakeDelegate(this, &SomeClass::SomeCallbackFunc);`
    //      where SomeCallbackFunc is a method of SomeClass that takes a KeyValues pointer as the only parameter.
    // API requests will pass a KeyValues object (that will auto delete itself, no worries!) with:
    //      "code" -- The integer HTTP status code returned. 0 if it's an IO error
    //      "data" -- The response data, parsed JSON represented as KeyValues
    //      "error" -- An error object represented as KeyValues
    //      "err_parse" -- If any parsing issue happens with JSON, it will be logged here as a string
    //
    // All API requests return `true` if the call succeeded in sending, else `false`.

    // ===== Auth ======
    // Sends an auth ticket to the server to get an API key to use for requests. 
    // This should be the first API call to make, and is done automatically on game boot.
    bool SendAuthTicket(CallbackFunc func);

    // ===== Maps ======
    bool GetMaps(CallbackFunc func);
    bool GetTop10MapTimes(const char* pMapName, CallbackFunc func);
    bool GetMapInfo(const char* pMapName, CallbackFunc func);


protected:
    // CAutoGameSystemPerFrame
    void PostInit() override;
    void Shutdown() override;

    // Auth ticket impl
    STEAM_CALLBACK(CAPIRequests, OnAuthTicket, GetAuthSessionTicketResponse_t);
    void OnAuthHTTP(KeyValues *pResponse);
    void DoAuth();

    // Base HTTP response method, the CallbackFunc is passed the JSON object here
    void OnHTTPResp(HTTPRequestCompleted_t *pParam, bool bIOFailure);
private:
    // Should be called after the HTTP request is prepared by the API calls (above)
    bool SendHTTPRequest(HTTPRequestHandle hRequest, CallbackFunc func, const char *pRequest);
    // Check the response for errors, insert error objects in here
    bool CheckAPIResponse(HTTPRequestCompleted_t *pCallback, bool bIOFailure, KeyValues *pKvOut);

    struct APICallback
    {
        APICallback() : handle(INVALID_HTTPREQUEST_HANDLE), callResult(nullptr) {}
        ~APICallback()
        {
            if (callResult)
                delete callResult; // Should call cancel if still in progress
        }
        HTTPRequestHandle handle;
        CallbackFunc callbackFunc;
        CCallResult<CAPIRequests, HTTPRequestCompleted_t> *callResult;
        bool operator==(const APICallback &other) const
        {
            return handle == other.handle;
        }
    };
    CUtlMap<HTTPRequestHandle, APICallback*> m_mapAPICalls;
    
    // Auth ticket impl
    HAuthTicket m_hAuthTicket;
    byte* m_bufAuthBuffer;
    uint32 m_iAuthActualSize;
    // API key for requests
    char *m_pAPIKey;
};

extern CAPIRequests *g_pAPIRequests;