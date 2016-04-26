#include "cbase.h"
#include "mom_util.h"
#include "filesystem.h"
#include "momentum/mom_shareddefs.h"
#include "mom_player_shared.h"
#include "tier0/memdbgon.h"

extern IFileSystem* filesystem;

#ifdef GAME_DLL
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

    IGameEvent *mapFinishedEvent = gameeventmanager->CreateEvent("run_save");

    JsonValue val;//Outer object
    JsonAllocator alloc;
    char* pDataPtr = reinterpret_cast<char*>(pData);
    DevLog("pDataPtr: %s\n", pDataPtr);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT)//Outer should be a JSON Object
        {
            //toNode() returns the >>payload<< of the JSON object !!!

            DevLog("Outer is JSON OBJECT!\n");
            JsonNode *node = val.toNode();
            DevLog("Outer has key %s with value %s\n", node->key, node->value.toString());

            if (node && node->value.getTag() == JSON_TRUE)
            {
                DevLog("RESPONSE WAS TRUE!\n");
                // Necesary so TimeDisplay scoreboard knows it has to update;
                if (mapFinishedEvent)
                {
                    mapFinishedEvent->SetBool("run_posted", true);
                    gameeventmanager->FireEvent(mapFinishedEvent);
                }
                    

                //MOM_TODO: Once the server updates this to contain more info, parse and do more with the response
            }
            //  @tuxx: this bit of code SHOULD be changing the bool for run_posted only, but for some reason it seems to 
            //  change the bool for run_saved as well. I can guess it's because they are in the same event, and the value was changed for
            //  run_saved but not fired. Either way, still really annoying. 

            //else if (mapFinishedEvent)
            //{
            //    mapFinishedEvent->SetBool("run_posted", false);
            //    gameeventmanager->FireEvent(mapFinishedEvent);
            //}
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    //Last but not least, free resources
    alloc.deallocate();
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

void MomentumUtil::GetRemoteRepoModVersion()
{
    CreateAndSendHTTPReq("http://raw.githubusercontent.com/momentum-mod/game/master/version.txt", &cbVersionCallback, &MomentumUtil::VersionCallback);
}

void MomentumUtil::VersionCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    if (bIOFailure) return;
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    char* pDataPtr = reinterpret_cast<char*>(pData);
    const char separator[2] = ".";
    CSplitString storedVersion = CSplitString(MOM_CURRENT_VERSION, separator);
    CSplitString repoVersion = CSplitString(pDataPtr, separator);

    char versionValue[15];
    Q_snprintf(versionValue, 15, "%s.%s.%s", repoVersion.Element(0), repoVersion.Element(1), repoVersion.Element(2));
    if (Q_atoi(repoVersion.Element(0)) > Q_atoi(storedVersion.Element(0)))
    {       
        ConVarRef("cl_showversionwarnpanel").SetValue(versionValue);
    }
    else if (Q_atoi(repoVersion.Element(1)) > Q_atoi(storedVersion.Element(1)))
    {
        ConVarRef("cl_showversionwarnpanel").SetValue(versionValue);
    }
    else if (Q_atoi(repoVersion.Element(2)) > Q_atoi(storedVersion.Element(2)))
    {
        ConVarRef("cl_showversionwarnpanel").SetValue(versionValue);
    }
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

#endif

void MomentumUtil::FormatTime(float m_flSecondsTime, char *pOut, int precision)
{
    //float m_flSecondsTime = ticks * rate;
    m_flSecondsTime = abs(m_flSecondsTime);

    int hours = m_flSecondsTime / (60.0f * 60.0f);
    int minutes = fmod(m_flSecondsTime / 60.0f, 60.0f);
    int seconds = fmod(m_flSecondsTime, 60.0f);
    int millis = fmod(m_flSecondsTime, 1.0f) * 1000.0f;
    int hundredths = millis / 10;
    int tenths = millis / 100;

    switch (precision)
    {
    case 0:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d:%02d", hours, minutes, seconds);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d", minutes, seconds);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%d", seconds);
        break;
    case 1:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d:%02d.%d", hours, minutes, seconds, tenths);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d.%d", minutes, seconds, tenths);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%d.%d", seconds, tenths);
        break;
    case 2:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d:%02d.%02d", hours, minutes, seconds, hundredths);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d.%02d", minutes, seconds, hundredths);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%d.%02d", seconds, hundredths);
        break;
    case 3:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d:%02d.%03d", hours, minutes, seconds, millis);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%d:%02d.%03d", minutes, seconds, millis);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%d.%03d", seconds, millis);
        break;
    }
    
}

Color MomentumUtil::GetColorFromVariation(float variation, float deadZone, Color normalcolor, Color increasecolor, Color decreasecolor)
{
    //variation is current velocity minus previous velocity. 
    Color pFinalColor = normalcolor;
    deadZone = abs(deadZone);

    if (variation < -deadZone)    //our velocity decreased 
        pFinalColor = decreasecolor;
    else if (variation > deadZone) //our velocity increased
        pFinalColor = increasecolor;

    return pFinalColor;
}


KeyValues* MomentumUtil::GetBestTime(KeyValues *kvMap, const char* szMapName, float tickrate, int flags)
{
    if (kvMap && szMapName)
    {
        char path[MAX_PATH], mapName[MAX_PATH];
        Q_snprintf(mapName, MAX_PATH, "%s.tim", szMapName);
        V_ComposeFileName("maps", mapName, path, MAX_PATH);

        if (kvMap->LoadFromFile(filesystem, path, "MOD"))
        {
            if (!kvMap->IsEmpty())
            {
                CUtlSortVector<KeyValues*, CTimeSortFunc> sortedTimes;

                FOR_EACH_SUBKEY(kvMap, kv)
                {
                    int kvflags = kv->GetInt("flags");
                    float kvrate = kv->GetFloat("rate");
                    if (kvflags == flags && FloatEquals(kvrate, tickrate, 0.001f)){
                        sortedTimes.InsertNoSort(kv);
                    }
                }
                if (!sortedTimes.IsEmpty())
                {
                    sortedTimes.RedoSort();
                    KeyValues *toReturn = kvMap->FindKey(sortedTimes[0]->GetName());
                    sortedTimes.Purge();
                    return toReturn;
                }           
            }
        }
    }
    return nullptr;
}

void MomentumUtil::GetBestStageTimes(KeyValues *pKvBestTime, CUtlVector<float> *vecInto)
{
    if (pKvBestTime && vecInto)
    {
        for (KeyValues *kv = pKvBestTime->GetFirstSubKey(); kv; kv = kv->GetNextKey())
        {
            if (!Q_strnicmp(kv->GetName(), "stage", strlen("stage")))
            {
                vecInto->AddToTail(kv->GetFloat("time"));
            }
        }
    }
}

void MomentumUtil::GetBestStageSpeeds(KeyValues *pKvBestTime, CUtlVector<float> *vecInto)
{
    if (pKvBestTime && vecInto)
    {
        FOR_EACH_SUBKEY(pKvBestTime, kv)
        {
            if (!Q_strnicmp(kv->GetName(), "stage", strlen("stage")))
            {
                vecInto->AddToTail(kv->GetFloat("stage_enter_vel"));
            }
        }
    }
}


static MomentumUtil s_momentum_util;
MomentumUtil *mom_UTIL = &s_momentum_util;