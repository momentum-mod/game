#include "cbase.h"
#include "filesystem.h"
#include "mom_player_shared.h"
#include "mom_util.h"
#include "momentum/mom_shareddefs.h"
#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

void MomentumUtil::DownloadCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    if (bIOFailure)
        return;

    FileHandle_t file;
    // MOM_TODO: Read the MOM_TODO DownloadMap(), we're going to need to save the zone files too
    file = filesystem->Open("testmapdownload.bsp", "w+b", "MOD");
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    // write the file
    filesystem->Write(pData, size, file);
    // save the file
    filesystem->Close(file);
    DevLog("Successfully written file\n");

    // Free resources
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void MomentumUtil::PostTimeCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    if (bIOFailure)
        return;
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    IGameEvent *runUploadedEvent = gameeventmanager->CreateEvent("run_upload");

    JsonValue val; // Outer object
    JsonAllocator alloc;
    char *pDataPtr = reinterpret_cast<char *>(pData);
    DevLog("pDataPtr: %s\n", pDataPtr);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
        {
            // toNode() returns the >>payload<< of the JSON object !!!

            DevLog("Outer is JSON OBJECT!\n");
            JsonNode *node = val.toNode();
            DevLog("Outer has key %s with value %s\n", node->key, node->value.toString());

            // MOM_TODO: This doesn't work, even if node has tag 'true'. Something is wrong with the way we are parsing
            // the JSON
            if (node && node->value.getTag() == JSON_TRUE)
            {
                DevLog("RESPONSE WAS TRUE!\n");
                // Necessary so that the leaderboards and hud_mapfinished update appropriately
                if (runUploadedEvent)
                {
                    runUploadedEvent->SetBool("run_posted", true);
                    // MOM_TODO: Once the server updates this to contain more info, parse and do more with the response
                    gameeventmanager->FireEvent(runUploadedEvent);
                }
            }
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    alloc.deallocate();
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void MomentumUtil::PostTime(const char *szURL)
{
    CreateAndSendHTTPReq(szURL, &cbPostTimeCallback, &MomentumUtil::PostTimeCallback);
}

void MomentumUtil::DownloadMap(const char *szMapname)
{
    if (!steamapicontext->SteamHTTP())
    {
        Warning("Failed to download map, cannot access HTTP!\n");
        return;
    }
    // MOM_TODO:
    // This should only be called if the user has the outdated map version or
    // doesn't have the map at all

    // The two different URLs:
    // cdn.momentum-mod.org/maps/MAPNAME/MAPNAME.bsp
    // and
    // cdn.momentum-mod.org/maps/MAPNAME/MAPNAME.zon
    // We're going to need to build requests for and download both of these files

    // Uncomment the following when we build the URLS (MOM_TODO)
    // CreateAndSendHTTPReq(mapfileURL, &cbDownloadCallback, &MomentumUtil::DownloadCallback);
    // CreateAndSendHTTPReq(zonFileURL, &cbDownloadCallback, &MomentumUtil::DownloadCallback);
}

void MomentumUtil::ReportBugCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{

}

void MomentumUtil::ReportBug(const char* email, const char* message)
{
    //CreateAndSendHTTPReq("", &cbReportBugCallback, &MomentumUtil::ReportBugCallback);
}

void MomentumUtil::CreateAndSendHTTPReq(const char *szURL, CCallResult<MomentumUtil, HTTPRequestCompleted_t> *callback,
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
        steamapicontext->SteamHTTP()->ReleaseHTTPRequest(handle); // GC
    }
}

void MomentumUtil::GetRemoteRepoModVersion()
{
    CreateAndSendHTTPReq("http://raw.githubusercontent.com/momentum-mod/game/master/version.txt", &cbVersionCallback,
                         &MomentumUtil::VersionCallback);
}

void MomentumUtil::VersionCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    if (bIOFailure)
        return;
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    char *pDataPtr = reinterpret_cast<char *>(pData);
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

void MomentumUtil::FormatTime(float m_flSecondsTime, char *pOut, int precision) const
{
    // We want the absolute value to format! Negatives (if any) should be added post-format!
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

Color MomentumUtil::GetColorFromVariation(float variation, float deadZone, Color normalcolor, Color increasecolor,
                                          Color decreasecolor) const
{
    // variation is current velocity minus previous velocity.
    Color pFinalColor = normalcolor;
    deadZone = abs(deadZone);

    if (variation < -deadZone) // our velocity decreased
        pFinalColor = decreasecolor;
    else if (variation > deadZone) // our velocity increased
        pFinalColor = increasecolor;

    return pFinalColor;
}

KeyValues *MomentumUtil::GetBestTime(KeyValues *kvMap, const char *szMapName, float tickrate, int flags)
{
    if (kvMap && szMapName)
    {
        char path[MAX_PATH], mapName[MAX_PATH];
        Q_snprintf(mapName, MAX_PATH, "%s%s", szMapName, EXT_TIME_FILE);
        V_ComposeFileName(MAP_FOLDER, mapName, path, MAX_PATH);
        if (kvMap->LoadFromFile(filesystem, path, "MOD"))
        {
            if (!kvMap->IsEmpty())
            {
                CUtlSortVector<KeyValues *, CTimeSortFunc> sortedTimes;

                FOR_EACH_SUBKEY(kvMap, kv)
                {
                    int kvflags = kv->GetInt("flags");
                    float kvrate = kv->GetFloat("rate");
                    if (kvflags == flags && FloatEquals(kvrate, tickrate, 0.001f))
                    {
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

bool MomentumUtil::GetRunComparison(const char *szMapName, float tickRate, int flags, RunCompare_t *into)
{
    bool toReturn = false;
    if (into && szMapName)
    {
        KeyValues *kvMap = new KeyValues(szMapName);
        KeyValues *bestRun = GetBestTime(kvMap, szMapName, tickRate, flags);
        if (bestRun)
        {
            FOR_EACH_SUBKEY(bestRun, kv)
            {
                if (!Q_strnicmp(kv->GetName(), "stage", strlen("stage"))) // MOM_TODO: or "checkpoint" (for linears)
                {
                    // MOM_TODO: this may not be a PB, for now it is, but we'll load times from online.
                    // I'm thinking the name could be like "(user): (Time)"
                    Q_strcpy(into->runName, "Personal Best");
                    // Splits
                    into->overallSplits.AddToTail(kv->GetFloat("enter_time"));
                    into->stageSplits.AddToTail(kv->GetFloat("time"));
                    // Keypress
                    into->stageJumps.AddToTail(kv->GetInt("num_jumps"));
                    into->stageStrafes.AddToTail(kv->GetInt("num_strafes"));
                    // Sync
                    into->stageAvgSync1.AddToTail(kv->GetFloat("avg_sync"));
                    into->stageAvgSync2.AddToTail(kv->GetFloat("avg_sync2"));
                    // Velocity (3D and Horizontal)
                    for (int i = 0; i < 2; i++)
                    {
                        bool horizontalVel = (i == 1);
                        into->stageAvgVels[i].AddToTail(kv->GetFloat(horizontalVel ? "avg_vel_2D" : "avg_vel"));
                        into->stageMaxVels[i].AddToTail(kv->GetFloat(horizontalVel ? "max_vel_2D" : "max_vel"));
                        into->stageEnterVels[i].AddToTail(
                            kv->GetFloat(horizontalVel ? "stage_enter_vel_2D" : "stage_enter_vel"));
                        into->stageExitVels[i].AddToTail(
                            kv->GetFloat(horizontalVel ? "stage_exit_vel_2D" : "stage_exit_vel"));
                    }
                }
            }
            DevLog("Loaded run comparisons for %s !\n", into->runName);
            toReturn = true;
        }

        kvMap->deleteThis();
    }
    return toReturn;
}

static MomentumUtil s_momentum_util;
MomentumUtil *mom_UTIL = &s_momentum_util;