#include "cbase.h"
#include "mom_util.h"

#include "memdbgon.h"



void MomentumUtil::DownloadCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    if (bIOFailure) return;

    FileHandle_t file;
    //MOM_TODO: Read the MOM_TODO DownloadMap(), we're going to need to save the zone files too
    file = filesystem->Open("testmapdownload.bsp", "w+b", "MOD");
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    //write the file
    filesystem->Write(pData, size, file);
    //save the file
    filesystem->Close(file);
    DevLog("Successfully written file\n");

    //Free resources
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void MomentumUtil::PostTimeCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    if (bIOFailure) return;
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    DevLog("PostTimeCallback response: %s\n", reinterpret_cast<char*>(pData));
    //MOM_TODO: Once the server updates this to contain more info, parse and do more with the response

    //Last but not least, free resources
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}


void MomentumUtil::PostTime(const char* szURL)
{
    CreateAndSendHTTPReq(szURL, &cbPostTimeCallback, &MomentumUtil::PostTimeCallback);
}

void MomentumUtil::DownloadMap(const char* szMapname)
{
    if (!steamapicontext->SteamHTTP())
    {
        Warning("Failed to download map, cannot access HTTP!\n");
        return;
    }
    //MOM_TODO: 
    //This should only be called if the user has the outdated map version or
    //doesn't have the map at all

    //The two different URLs:
    //cdn.momentum-mod.org/maps/MAPNAME/MAPNAME.bsp
    //and
    //cdn.momentum-mod.org/maps/MAPNAME/MAPNAME.zon
    //We're going to need to build requests for and download both of these files

    //Uncomment the following when we build the URLS (MOM_TODO)
    //CreateAndSendHTTPReq(mapfileURL, &cbDownloadCallback, &MomentumUtil::DownloadCallback);
    //CreateAndSendHTTPReq(zonFileURL, &cbDownloadCallback, &MomentumUtil::DownloadCallback);
}


void MomentumUtil::CreateAndSendHTTPReq(const char* szURL, CCallResult<MomentumUtil, HTTPRequestCompleted_t>* callback,
    CCallResult<MomentumUtil, HTTPRequestCompleted_t>::func_t func)
{
    HTTPRequestHandle handle = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, szURL);
    SteamAPICall_t apiHandle;
    if (steamapicontext->SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
    {
        callback->Set(apiHandle, this, func);
    }
    else
    {
        Warning("Failed to send HTTP Request to post scores online!\n");
        steamapicontext->SteamHTTP()->ReleaseHTTPRequest(handle);//GC
    }
}

MomentumUtil mom_UTIL;