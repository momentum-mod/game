#include "cbase.h"

#include "filesystem.h"
#include "mom_player_shared.h"
#include "mom_util.h"
#include "momentum/mom_shareddefs.h"
#include "tier0/memdbgon.h"
#include "run/mom_replay_factory.h"

extern IFileSystem *filesystem;

inline void CleanupRequest(HTTPRequestCompleted_t *pCallback, uint8 *pData)
{
    if (pData)
    {
        delete[] pData;
    }
    pData = nullptr;
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void MomentumUtil::DownloadCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    if (bIOFailure)
        return;

    FileHandle_t file;
    // MOM_TODO: Read the MOM_TODO DownloadMap(), we're going to need to save the zone files too
    file = filesystem->Open("testmapdownload.bsp", "w+b", "MOD");
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    if (size == 0)
    {
        Warning("MomentumUtil::DownloadCallback: 0 body size!\n");
        return;
    }
    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    // write the file
    filesystem->Write(pData, size, file);
    // save the file
    filesystem->Close(file);
    DevLog("Successfully written file\n");

    // Free resources
    CleanupRequest(pCallback, pData);
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

void MomentumUtil::CreateAndSendHTTPReq(const char *szURL, CCallResult<MomentumUtil, HTTPRequestCompleted_t> *callback,
                                        CCallResult<MomentumUtil, HTTPRequestCompleted_t>::func_t func)
{
    if (steamapicontext && steamapicontext->SteamHTTP())
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
    else
    {
        Warning("Steampicontext failure.\n");
        Warning("Could not find Steam Api Context active\n");
    }
}

bool MomentumUtil::CreateAndSendHTTPReqWithPost(const char *szURL,
                                                CCallResult<MomentumUtil, HTTPRequestCompleted_t> *callback,
                                                CCallResult<MomentumUtil, HTTPRequestCompleted_t>::func_t func,
                                                KeyValues *params)
{
    bool bSuccess = false;
    if (steamapicontext && steamapicontext->SteamHTTP())
    {
        HTTPRequestHandle handle = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodPOST, szURL);
        FOR_EACH_VALUE(params, p_value)
        {
            steamapicontext->SteamHTTP()->SetHTTPRequestGetOrPostParameter(handle, p_value->GetName(),
                                                                           p_value->GetString());
        }

        SteamAPICall_t apiHandle;

        if (steamapicontext->SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            Warning("Report sent.\n");
            callback->Set(apiHandle, this, func);
            bSuccess = true;
        }
        else
        {
            Warning("Failed to send HTTP Request to report bug online!\n");
            steamapicontext->SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("Steamapicontext is not online!\n");
    }
    return bSuccess;
}

#ifdef CLIENT_DLL
void MomentumUtil::GetRemoteRepoModVersion()
{
    CreateAndSendHTTPReq("http://raw.githubusercontent.com/momentum-mod/game/master/version.txt", &cbVersionCallback,
                         &MomentumUtil::VersionCallback);
}

void MomentumUtil::GetRemoteChangelog()
{
    CreateAndSendHTTPReq("http://raw.githubusercontent.com/momentum-mod/game/master/changelog.txt", &cbChangeLog,
                         &MomentumUtil::ChangelogCallback);
}

void MomentumUtil::ChangelogCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    const char *pError = "Error loading changelog!";
    if (bIOFailure)
    {
        pError = "Error loading changelog due to bIOFailure!";
        changelogpanel->SetChangelog(pError);
        return;
    }
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    if (size == 0)
    {
        pError = "MomentumUtil::ChangelogCallback: 0 body size!\n";
        changelogpanel->SetChangelog(pError);
        return;
    }

    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    char *pDataPtr = reinterpret_cast<char *>(pData);

    changelogpanel->SetChangelog(pDataPtr);

    CleanupRequest(pCallback, pData);
}

void MomentumUtil::VersionCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    // 502 is usually returned if Steam is set to offline mode. Thanks .Enjoy for reporting this one!
    if (bIOFailure || (pCallback && pCallback->m_eStatusCode == k_EHTTPStatusCode502BadGateway))
        return;
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    if (size == 0)
    {
        Warning("MomentumUtil::VersionCallback: 0 body size!\n");
        return;
    }
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);
    char *pDataPtr = reinterpret_cast<char *>(pData);
    const char separator[2] = ".";
    CSplitString storedVersion(MOM_CURRENT_VERSION, separator);
    CSplitString repoVersion(pDataPtr, separator);
    // Above check for 502 fixes crash with Steam being offline, but just to be on the safe side, we double check we can get all the version numbers
    if (repoVersion.Count() >= 3)
    {
        char versionValue[15];
        Q_snprintf(versionValue, 15, "%s.%s.%s", repoVersion.Element(0), repoVersion.Element(1), repoVersion.Element(2));

        for (int i = 0; i < 3; i++)
        {
            int repo = Q_atoi(repoVersion.Element(i)), local = Q_atoi(storedVersion.Element(i));
            if (repo > local)
            {
                if (developer.GetInt() < 2) // If we're developers, we probably know what version we are at.
                {
                    changelogpanel->SetVersion(versionValue);
                    GetRemoteChangelog();
                    changelogpanel->Activate();
                }
                break;
            }
            if (repo < local)
            {
                // The local version is higher than the repo version, do not show this panel
                break;
            }
        }
    }
    CleanupRequest(pCallback, pData);
}

void MomentumUtil::GenerateBogusRunStats(CMomRunStats *pStatsOut)
{
    RandomSeed(Plat_FloatTime());
    for (int i = 0; i < MAX_STAGES; i++)
    {
        // Time
        pStatsOut->SetZoneTime(i, RandomFloat(25.0f, 250.0f));
        pStatsOut->SetZoneEnterTime(i, i == 1 ? 0.0f : RandomFloat(25.0f, 250.0f));

        // Velocity
        pStatsOut->SetZoneVelocityMax(i, RandomFloat(0.0f, 7000.0f), RandomFloat(0.0f, 4949.0f));
        pStatsOut->SetZoneVelocityAvg(i, RandomFloat(0.0f, 7000.0f), RandomFloat(0.0f, 4949.0f));
        pStatsOut->SetZoneExitSpeed(i, RandomFloat(0.0f, 7000.0f), RandomFloat(0.0f, 4949.0f));
        pStatsOut->SetZoneEnterSpeed(i, RandomFloat(0.0f, 7000.0f), RandomFloat(0.0f, 4949.0f));

        // Sync
        pStatsOut->SetZoneStrafeSyncAvg(i, RandomFloat(65.0f, 100.0f));
        pStatsOut->SetZoneStrafeSync2Avg(i, RandomFloat(65.0f, 100.0f));

        // Keypress
        pStatsOut->SetZoneJumps(i, RandomInt(3, 100));
        pStatsOut->SetZoneStrafes(i, RandomInt(40, 1500));
    }
}
#endif

void MomentumUtil::FormatTime(float m_flSecondsTime, char *pOut, const int precision, const bool fileName, const bool negativeTime) const
{
    // We want the absolute value to format! Negatives (if any) should be added post-format!
    m_flSecondsTime = fabs(m_flSecondsTime);
    char separator = fileName ? '-' : ':'; // MOM_TODO: Think of a better char?
    const char *negative = negativeTime ? "-" : "";
    int hours = static_cast<int>(m_flSecondsTime / (60.0f * 60.0f));
    int minutes = static_cast<int>(fmod(m_flSecondsTime / 60.0f, 60.0f));
    int seconds = static_cast<int>(fmod(m_flSecondsTime, 60.0f));
    int millis = static_cast<int>(fmod(m_flSecondsTime, 1.0f) * 1000.0f);
    int hundredths = millis / 10;
    int tenths = millis / 100;

    switch (precision)
    {
    case 0:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d", negative, hours, separator, minutes, separator, seconds);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d", negative, minutes, separator, seconds);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d", negative, seconds);
        break;
    case 1:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%d", negative, hours, separator, minutes, separator,
                       seconds, tenths);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%d", negative, minutes, separator, seconds, tenths);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%d", negative, seconds, tenths);
        break;
    case 2:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%02d", negative, hours, separator, minutes, separator,
                       seconds, hundredths);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%02d", negative, minutes, separator, seconds, hundredths);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%02d", negative, seconds, hundredths);
        break;
    case 3:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%03d", negative, hours, separator, minutes, separator,
                       seconds, millis);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%03d", negative, minutes, separator, seconds, millis);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%03d", negative, seconds, millis);
        break;
    case 4:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%04d", negative, hours, separator, minutes, separator,
                       seconds, millis);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%04d", negative, minutes, separator, seconds, millis);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%04d", negative, seconds, millis);
        break;
    }
}

Color MomentumUtil::GetColorFromVariation(const float variation, float deadZone, const Color &normalcolor, const Color &increasecolor,
                                          const Color &decreasecolor) const
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

Color *MomentumUtil::GetColorFromHex(const char *hexColor)
{
    long hex = strtol(hexColor, nullptr, 16);
    int length = Q_strlen(hexColor);
    if (length == 6)
    {
        int r = ((hex >> 16) & 0xFF); // extract RR byte
        int g = ((hex >> 8) & 0xFF);  // extract GG byte
        int b = ((hex)&0xFF);         // extract BB byte
        m_newColor.SetColor(r, g, b, 75);
        return &m_newColor;
    }
    Msg("Error: Color format incorrect! Use hex code in format \"RRGGBB\"\n");
    return nullptr;
}

inline bool CheckReplayB(CMomReplayBase *pFastest, CMomReplayBase *pCheck, float tickrate, uint32 flags)
{
    if (pCheck)
    {
        if (pCheck->GetRunFlags() == flags && g_pMomentumUtil->FloatEquals(tickrate, pCheck->GetTickInterval()))
        {
            if (pFastest)
            {
                return pCheck->GetRunTime() < pFastest->GetRunTime();
            }
            
            return true;
        }
    }

    return false;
}

//!!! NOTE: The value returned here MUST BE DELETED, otherwise you get a memory leak!
CMomReplayBase *MomentumUtil::GetBestTime(const char *szMapName, float tickrate, uint32 flags) const
{
    if (szMapName)
    {
        char path[MAX_PATH];
        Q_snprintf(path, MAX_PATH, "%s/%s*%s", RECORDING_PATH, szMapName, EXT_RECORDING_FILE);
        V_FixSlashes(path);

        CMomReplayBase *pFastest = nullptr;

        FileFindHandle_t found;
        const char *pFoundFile = filesystem->FindFirstEx(path, "MOD", &found);
        while (pFoundFile)
        {
            // NOTE: THIS NEEDS TO BE MANUALLY CLEANED UP!
            char pReplayPath[MAX_PATH];
            V_ComposeFileName(RECORDING_PATH, pFoundFile, pReplayPath, MAX_PATH);

            CMomReplayBase *pBase = g_ReplayFactory.LoadReplayFile(pReplayPath, false);
            assert(pBase != nullptr);
                
            if (CheckReplayB(pFastest, pBase, tickrate, flags))
            {
                pFastest = pBase;
            }
            else // Not faster, get rid of it
            {
                delete pBase;
            }

            pFoundFile = filesystem->FindNext(found);
        }

        filesystem->FindClose(found);
        return pFastest;
    }
    return nullptr;
}

bool MomentumUtil::GetRunComparison(const char *szMapName, const float tickRate, const int flags, RunCompare_t *into) const
{
    if (into && szMapName)
    {
        CMomReplayBase *bestRun = GetBestTime(szMapName, tickRate, flags);
        if (bestRun)
        {
            // MOM_TODO: this may not be a PB, for now it is, but we'll load times from online.
            // I'm thinking the name could be like "(user): (Time)"
            FillRunComparison("Personal Best", bestRun->GetRunStats(), into);
            delete bestRun;
            DevLog("Loaded run comparisons for %s !\n", into->runName);
            return true;
        }
    }
    return false;
}

void MomentumUtil::FillRunComparison(const char *compareName, CMomRunStats *pRun, RunCompare_t *into) const
{
    Q_strcpy(into->runName, compareName);
    pRun->FullyCopyStats(&into->runStatsData);
}

#define SAVE_3D_TO_KV(kvInto, pName, toSave)                                                                           \
    if (!kvInto || !pName)                                                                                             \
        return;                                                                                                        \
    char value[512];                                                                                                   \
    Q_snprintf(value, 512, "%f %f %f", toSave.x, toSave.y, toSave.z);                                                  \
    kvInto->SetString(pName, value);

#define LOAD_3D_FROM_KV(kvFrom, pName, into)                                                                           \
    if (!kvFrom || !pName)                                                                                             \
        return;                                                                                                        \
    sscanf(kvFrom->GetString(pName), "%f %f %f", &into.x, &into.y, &into.z);

void MomentumUtil::KVSaveVector(KeyValues *kvInto, const char *pName, const Vector &toSave)
{
    SAVE_3D_TO_KV(kvInto, pName, toSave);
}

void MomentumUtil::KVLoadVector(KeyValues *kvFrom, const char *pName, Vector &vecInto)
{
    LOAD_3D_FROM_KV(kvFrom, pName, vecInto);
}

void MomentumUtil::KVSaveQAngles(KeyValues *kvInto, const char *pName, const QAngle &toSave)
{
    SAVE_3D_TO_KV(kvInto, pName, toSave);
}

void MomentumUtil::KVLoadQAngles(KeyValues *kvFrom, const char *pName, QAngle &angInto)
{
    LOAD_3D_FROM_KV(kvFrom, pName, angInto);
}

static MomentumUtil s_momentum_util;
MomentumUtil *g_pMomentumUtil = &s_momentum_util;
