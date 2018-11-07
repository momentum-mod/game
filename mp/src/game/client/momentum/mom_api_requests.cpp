#include "cbase.h"

#include "mom_api_requests.h"
#include "util/jsontokv.h"
#include "fmtstr.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

#define BASE_API_URL "http://localhost:3002/api/"
#define API_REQ(url) BASE_API_URL url

static MAKE_TOGGLE_CONVAR(mom_log_api_requests, "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, all API requests will be logged to console.\n");

CAPIRequests::CAPIRequests() : CAutoGameSystemPerFrame("CAPIRequests"), 
m_hAuthTicket(k_HAuthTicketInvalid), m_bufAuthBuffer(nullptr),
m_iAuthActualSize(0), m_pAPIKey(nullptr)
{
    SetDefLessFunc(m_mapAPICalls);
}

bool CAPIRequests::SendAuthTicket(CallbackFunc func)
{
    HTTPRequestHandle handle;
    if (CreateAPIRequest(handle, API_REQ("auth/steam/user"), k_EHTTPMethodPOST, false))
    {
        uint64 id = SteamUser()->GetSteamID().ConvertToUint64();
        SteamHTTP()->SetHTTPRequestHeaderValue(handle, "id", CFmtStr("%llu", id).Get());
        SteamHTTP()->SetHTTPRequestRawPostBody(handle, "application/octet-stream", m_bufAuthBuffer, m_iAuthActualSize);

        return SendAPIRequest(handle, func, __FUNCTION__);
    }

    return false;
}

bool CAPIRequests::GetMaps(KeyValues *pKvFilters, CallbackFunc func)
{
    HTTPRequestHandle handle;
    if (CreateAPIRequest(handle, API_REQ("maps"), k_EHTTPMethodGET))
    {
        if (pKvFilters && !pKvFilters->IsEmpty())
        {
            FOR_EACH_VALUE(pKvFilters, pKvFilter)
            {
                SteamHTTP()->SetHTTPRequestGetOrPostParameter(handle, pKvFilter->GetName(),
                                                              pKvFilter->GetString());
            }
        }

        return SendAPIRequest(handle, func, __FUNCTION__);
    }

    return false;
}

bool CAPIRequests::GetTop10MapTimes(const char* pMapName, CallbackFunc func)
{
    AssertMsg(0, "%s Needs implementation!\n", __FUNCTION__);
    return false;
}

bool CAPIRequests::GetMapInfo(const char* pMapName, CallbackFunc func)
{
    AssertMsg(0, "%s Needs implementation!\n", __FUNCTION__);
    return false;
}


void CAPIRequests::PostInit()
{
    DoAuth();
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

    // This also cancels any outstanding API requests
    m_mapAPICalls.PurgeAndDeleteElements();
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
    KeyValues *pData = pResponse->FindKey("data");
    KeyValues *pErr = pResponse->FindKey("error");
    if (pData)
    {
        uint32 tokenLength = pData->GetInt("length");

        if (tokenLength)
        {
            m_pAPIKey = new char[tokenLength + 1];
            Q_strncpy(m_pAPIKey, pData->GetString("token"), tokenLength + 1);
        }
    }
    else if (pErr)
    {
        Warning("Error when trying to get an API key!\n");
        // MOM_TODO display the error here in the console
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

    m_bufAuthBuffer = new byte[1024];
    m_hAuthTicket = SteamUser()->GetAuthSessionTicket(m_bufAuthBuffer, 1024, &m_iAuthActualSize);
    if (m_hAuthTicket == k_HAuthTicketInvalid)
        Warning("Initial call to get the ticket failed!\n");
}

void CAPIRequests::OnHTTPResp(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    // Firstly, let's find the callback that corresponds to the API request we made
    uint16 callbackIndx = m_mapAPICalls.Find(pCallback->m_hRequest);
    if (callbackIndx != m_mapAPICalls.InvalidIndex())
    {
        // Okay cool, callback found
        APICallback *callback = m_mapAPICalls[callbackIndx];

        // Let's create our main KeyValues object to operate on and properly clean it up out of this scope
        KeyValues::AutoDelete response("response");

        // Secondly, let's set the code of the response. Even if it's an IO error.
        response->SetInt("code", pCallback->m_eStatusCode);

        // Thirdly, check if there are any errors early on. 
        // This populates the response with an appropriate "error" KeyValues object
        if (CheckAPIResponse(pCallback, bIOFailure, response))
        {
            KeyValues *pKvBodyData = new KeyValues("data");
            if (pCallback->m_unBodySize > 0)
            {
                // Fourthly, read the body properly
                uint8 *pData = new uint8[pCallback->m_unBodySize + 1];
                SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, pCallback->m_unBodySize);
                pData[pCallback->m_unBodySize] = 0; // Make sure to null terminate

                // Fifthly, parse this JSON and convert to KeyValues
                char *pDataPtr = reinterpret_cast<char *>(pData);
                if (CJsonToKeyValues::ConvertJsonToKeyValues(pDataPtr, pKvBodyData))
                {
                    // Log out the request if needed
                    if (mom_log_api_requests.GetBool())
                    {
                        CKeyValuesDumpContextAsDevMsg dump;
                        pKvBodyData->Dump(&dump);
                    }
                }
                else
                {
                    Warning("Failed to parse! %s\n", response->GetString("err_parse"));
                }

                // Sixthly, free our data buffer 
                delete[] pData;
                pData = nullptr;
                pDataPtr = nullptr;
            }
            // "else 0 body size" -- it's valid, but it'll be empty. Reading a 204 can still be done here
            response->AddSubKey(pKvBodyData);
        }
        else
        {
            // Errored out, either a server 4XX or 5XX error. Still valid, the callback func will want to know about it.
        }

        // Now actually call the callback. It should be reading the body by using `pKvResponse->FindKey("data")`
        // or any errors by using `pKvResponse->FindKey("error")`
        callback->callbackFunc(response);

        // And remove it from the map
        m_mapAPICalls.RemoveAt(callbackIndx);
        // And delete it (no memory leek pls)
        delete callback;
        // And delete the response KeyValu- oh right the AutoDelete handles that here (out of scope)
    }
    else
    {
        // Should never happen... but you never know. It's 2018.
        Warning("Somehow this class caught an unmapped HTTP request!\n");
    }

    // And finally free the request
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

bool CAPIRequests::CreateAPIRequest(HTTPRequestHandle &handle, const char* pszURL, EHTTPMethod kMethod, bool bAuth /* = true*/)
{
    if (!SteamHTTP())
        return false;

    if (bAuth && !m_pAPIKey)
        return false;

    handle = SteamHTTP()->CreateHTTPRequest(kMethod, pszURL);

    // Add the API key
    if (bAuth)
        SteamHTTP()->SetHTTPRequestHeaderValue(handle, "Authorization", CFmtStr1024("Bearer %s", m_pAPIKey).Get());

    return true;
}

bool CAPIRequests::SendAPIRequest(HTTPRequestHandle hRequest, CallbackFunc func, const char* pRequest)
{
    SteamAPICall_t apiHandle;
    if (SteamHTTP()->SendHTTPRequest(hRequest, &apiHandle))
    {
        APICallback *callback = new APICallback();
        callback->handle = hRequest;
        callback->callbackFunc = func;
        callback->callResult = new CCallResult<CAPIRequests, HTTPRequestCompleted_t>();
        callback->callResult->Set(apiHandle, this, &CAPIRequests::OnHTTPResp);
        m_mapAPICalls.Insert(hRequest, callback);
        return true;
    }

    Warning("%s --- Failed to send HTTP Request!\n", pRequest);
    SteamHTTP()->ReleaseHTTPRequest(hRequest); // GC
    return false;
}

bool CAPIRequests::CheckAPIResponse(HTTPRequestCompleted_t* pCallback, bool bIOFailure, KeyValues *pKvOut)
{
    if (pCallback->m_eStatusCode >= k_EHTTPStatusCode200OK &&
        pCallback->m_eStatusCode < k_EHTTPStatusCode300MultipleChoices)
    {
        return true; // Exit early
    }
    KeyValues *pKvErrStatus = nullptr;
    if (bIOFailure || !pCallback->m_bRequestSuccessful)
    {
        DevWarning(2, "bIOFailure!\n");
        pKvErrStatus = new KeyValues("error");
        pKvErrStatus->SetString("msg", "IO Failure");
    }
    // MOM_TODO: Parse the body here and stuff the error object in it?
    else if (pCallback->m_eStatusCode >= k_EHTTPStatusCode400BadRequest)
    {
        DevWarning(2, "Error in Request!\n");
        pKvErrStatus = new KeyValues("error");
        if (pCallback->m_eStatusCode >= k_EHTTPStatusCode500InternalServerError)
            pKvErrStatus->SetString("msg", "Internal Server Error");
        else
            pKvErrStatus->SetString("msg", "Error 4XX");
    }
    
    if (pKvErrStatus)
    {
        pKvOut->AddSubKey(pKvErrStatus);
        return false;
    }

    return true;
}

CAPIRequests s_APIRequests;
CAPIRequests *g_pAPIRequests = &s_APIRequests;