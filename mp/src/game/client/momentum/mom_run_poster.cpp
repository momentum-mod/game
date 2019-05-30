#include "cbase.h"

#include "filesystem.h"
#include "mom_run_poster.h"
#include "mom_api_requests.h"
#include "mom_map_cache.h"
#include "icommandline.h"

#include <tier0/memdbgon.h>

CRunPoster::CRunPoster()
{
#if ENABLE_STEAM_LEADERBOARDS
    m_hCurrentLeaderboard = 0;
#endif
    ResetSession();
}

CRunPoster::~CRunPoster() {}

void CRunPoster::PostInit()
{
    ListenForGameEvent("replay_save");
    ListenForGameEvent("timer_event");
    ListenForGameEvent("zone_enter");
    m_bIsMappingMode = CommandLine()->FindParm("-mapping") != 0;
}

void CRunPoster::LevelInitPostEntity()
{
#if ENABLE_STEAM_LEADERBOARDS
    const char *pMapName = MapName();
    if (pMapName)
    {
        CHECK_STEAM_API(SteamUserStats());
        SteamAPICall_t findCall = SteamUserStats()->FindOrCreateLeaderboard(pMapName, k_ELeaderboardSortMethodAscending, k_ELeaderboardDisplayTypeTimeMilliSeconds);
        m_cLeaderboardFindResult.Set(findCall, this, &CRunPoster::OnLeaderboardFind);

    }
#endif
    if (!m_bIsMappingMode)
    {
        const auto pMapData = g_pMapCache->GetCurrentMapData();
        if (pMapData)
        {
            g_pAPIRequests->InvalidateRunSession(pMapData->m_uID, UtlMakeDelegate(this, &CRunPoster::InvalidateSessionCallback));
        }
        ResetSession();
    }
}

void CRunPoster::LevelShutdownPostEntity()
{
#if ENABLE_STEAM_LEADERBOARDS
    m_hCurrentLeaderboard = 0;
#endif
}

void CRunPoster::PreRender()
{
    if (m_uRunSessionID)
    {
        static ConVarRef cheats("sv_cheats");
        if (cheats.GetBool())
        {
            g_pAPIRequests->InvalidateRunSession(g_pMapCache->GetCurrentMapID(), UtlMakeDelegate(this, &CRunPoster::InvalidateSessionCallback));
            ResetSession();
        }
    }
}

void CRunPoster::FireGameEvent(IGameEvent *pEvent)
{
    if (FStrEq(pEvent->GetName(), "replay_save"))
    {
        if (pEvent->GetBool("save"))
        {
#if ENABLE_STEAM_LEADERBOARDS
            CHECK_STEAM_API(SteamUserStats());

            if (!m_hCurrentLeaderboard)
            {
                Warning("Could not upload run: leaderboard doesn't exist!\n");
                // MOM_TODO: Make the run_posted event here with the above message?
                return;
            }

            // Upload the score
            int runTime = pEvent->GetInt("time"); // Time in milliseconds
            if (!runTime)
            {
                Warning("Could not upload run: time is 0 milliseconds!\n");
                // MOM_TODO: Make the run_posted event here with the above message?
                return;
            }

            // Save the name and path for uploading in the callback of the score
            Q_strncpy(m_szFileName, pEvent->GetString("filename"), MAX_PATH);
            Q_strncpy(m_szFilePath, pEvent->GetString("filepath"), MAX_PATH);

            // Set our score
            SteamAPICall_t uploadScore = SteamUserStats()->UploadLeaderboardScore(m_hCurrentLeaderboard,
                                                                                  k_ELeaderboardUploadScoreMethodKeepBest, runTime, nullptr, 0);
            m_cLeaderboardScoreUploaded.Set(uploadScore, this, &CRunPoster::OnLeaderboardScoreUploaded);
#endif

            if (ShouldSubmitRun())
            {
                CUtlBuffer buf;
                if (g_pFullFileSystem->ReadFile(pEvent->GetString("filepath"), "MOD", buf))
                {
                    if (g_pAPIRequests->EndRunSession(g_pMapCache->GetCurrentMapID(), m_uRunSessionID, buf, UtlMakeDelegate(this, &CRunPoster::EndSessionCallback)))
                    {
                        DevLog(2, "Run submitted!\n");
                    }
                    else
                    {
                        Warning("Failed to submit run; API call returned false!\n");
                    }
                    ResetSession();
                }
                else
                {
                    Warning("Failed to submit run: could not read file %s from %s !\n", pEvent->GetString("filename"), pEvent->GetString("filepath"));
                }
            }
            else
            {
                Warning("Not submitting the run!\n");
            }
        }
    }
    else if (FStrEq(pEvent->GetName(), "timer_event"))
    {
        const auto iMapID = g_pMapCache->GetCurrentMapID();
        static ConVarRef cheats("sv_cheats");
        if (iMapID == 0 || m_bIsMappingMode || cheats.GetBool())
            return;

        if (pEvent->GetInt("ent") == engine->GetLocalPlayer())
        {
            const auto iType = pEvent->GetInt("type", -1);
            if (iType == TIMER_EVENT_STARTED)
            {
                // MOM_TODO allow different track/zones (0.9.0)
                // C_MomentumPlayer::GetLocalMomPlayer()->GetRunEntData()->m_iCurrentTrack and m_iCurrentZone
                g_pAPIRequests->CreateRunSession(iMapID, 0, 0, UtlMakeDelegate(this, &CRunPoster::CreateSessionCallback));
            }
            else if (iType == TIMER_EVENT_STOPPED && m_uRunSessionID)
            {
                g_pAPIRequests->InvalidateRunSession(iMapID, UtlMakeDelegate(this, &CRunPoster::InvalidateSessionCallback));
                ResetSession();
            }
        }
    }
    else if (FStrEq(pEvent->GetName(), "zone_enter"))
    {
        const auto iMapID = g_pMapCache->GetCurrentMapID();
        if (iMapID == 0 || m_uRunSessionID == 0 || m_bIsMappingMode)
            return;
        
        if (pEvent->GetInt("ent") == engine->GetLocalPlayer())
        {
            const auto iZone = pEvent->GetInt("num");
            if (iZone > 1 && m_iZoneEnterTicks[iZone] == 0)
            {
                m_iZoneEnterTicks[iZone] = gpGlobals->tickcount;
                g_pAPIRequests->AddRunSessionTimestamp(iMapID, m_uRunSessionID, 
                                                       iZone, gpGlobals->tickcount, 
                                                       UtlMakeDelegate(this, &CRunPoster::UpdateSessionCallback));
            }
        }
    }
}

#if ENABLE_STEAM_LEADERBOARDS
void CRunPoster::OnLeaderboardFind(LeaderboardFindResult_t* pResult, bool bIOFailure)
{
    if (bIOFailure)
    {
        Warning("Failed to create leaderboard for map %s!\n", MapName());
        return;
    }

    m_hCurrentLeaderboard = pResult->m_hSteamLeaderboard;
}

void CRunPoster::OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t* pResult, bool bIOFailure)
{
    IGameEvent *pEvent = gameeventmanager->CreateEvent("run_upload");

    bool bSuccess = true;
    if (bIOFailure || !pResult->m_bSuccess)
    {
        bSuccess = false;
        // MOM_TODO: If it didn't upload, hijack the run_upload event with a message here?
    }

    if (pEvent)
    {
        pEvent->SetBool("run_posted", bSuccess);

        if (gameeventmanager->FireEvent(pEvent))
        {
            if (bSuccess)
            {
                // Now we can (try to) upload this replay file to the Steam Cloud for attaching to this new leaderboard score
                CUtlBuffer fileBuffer;
                if (filesystem->ReadFile(m_szFilePath, "MOD", fileBuffer))
                {
                    SteamAPICall_t write = SteamRemoteStorage()->FileWriteAsync(m_szFileName, fileBuffer.Base(), fileBuffer.TellPut());
                    m_cFileUploaded.Set(write, this, &CRunPoster::OnFileUploaded);
                }
                else
                {
                    DevWarning("Couldn't read replay file %s!\n", m_szFilePath);
                }

                ConColorMsg(Color(0, 255, 0, 255), "Uploaded run to the leaderboards, check it out!\n");
            }
            else
            {
                Warning("Could not upload your leaderboard score, sorry!\n");
            }
        }
    }
}

void CRunPoster::OnLeaderboardUGCSet(LeaderboardUGCSet_t* pResult, bool bIOFailure)
{
    bool bSuccess = true;
    if (bIOFailure || pResult->m_eResult != k_EResultOK)
    {
        bSuccess = false;
        Warning("Failed to upload replay file to leaderboard! Result: %i\n", pResult->m_eResult);
    }

    // Either way we need to delete the file from Steam Cloud now, don't use quota
    if (SteamRemoteStorage()->FileDelete(m_szFileName))
    {
        DevLog("Successfully deleted the uploaded run on the Steam Cloud at %s\n", m_szFileName);
    }

    // Clear out the paths here
    m_szFileName[0] = 0;
    m_szFilePath[0] = 0;

    if (bSuccess)
        ConColorMsg(Color(0, 255, 0, 255), "Uploaded replay file to leaderboards, check it out!\n");
}

void CRunPoster::OnFileUploaded(RemoteStorageFileWriteAsyncComplete_t* pResult, bool bIOFailure)
{
    if (pResult->m_eResult != k_EResultOK || bIOFailure)
    {
        Warning("Could not upload steam cloud file! Result: %i\n", pResult->m_eResult);
        return;
    }

    SteamAPICall_t UGCcall = SteamRemoteStorage()->FileShare(m_szFileName);
    m_cFileShared.Set(UGCcall, this, &CRunPoster::OnFileShared);
}

void CRunPoster::OnFileShared(RemoteStorageFileShareResult_t* pResult, bool bIOFailure)
{
    if (bIOFailure || pResult->m_eResult != k_EResultOK)
    {
        Warning("Could not upload user replay file! Result %i\n", pResult->m_eResult);
        return;
    }

    // Now we attach to the leaderboard
    SteamAPICall_t UGCLeaderboardCall = SteamUserStats()->AttachLeaderboardUGC(m_hCurrentLeaderboard, pResult->m_hFile);
    m_cLeaderboardUGCSet.Set(UGCLeaderboardCall, this, &CRunPoster::OnLeaderboardUGCSet);
}
#endif

void CRunPoster::InvalidateSessionCallback(KeyValues *pKv)
{
    // Note: Never has any body data due to being a 204
    KeyValues *pErr = pKv->FindKey("error");
    if (pErr)
    {
        Warning("Error when invalidating run session!\n");
    }
}

void CRunPoster::CreateSessionCallback(KeyValues *pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        m_uRunSessionID = pData->GetUint64("id");
        ConColorMsg(2, COLOR_GREEN, "Got the run session ID! %lld\n", m_uRunSessionID);
    }
    else if (pErr)
    {
        Warning("Error when creating the run session!\n");
    }
}

void CRunPoster::UpdateSessionCallback(KeyValues *pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        // MOM_TODO We may get incremental XP in the future here (0.9.0+)
    }
    else if (pErr)
    {
        Warning("Error when updating the run session!\n");
    }
}

void CRunPoster::EndSessionCallback(KeyValues* pKv)
{
    IGameEvent *runUploadedEvent = gameeventmanager->CreateEvent("run_upload");
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        // Necessary so that the leaderboards and hud_mapfinished update appropriately
        if (runUploadedEvent)
        {
            runUploadedEvent->SetBool("run_posted", true);

            const auto pPlayer = C_BasePlayer::GetLocalPlayer();
            if (pPlayer)
            {
                if (pData->GetBool("isNewWorldRecord"))
                {
                    pPlayer->EmitSound("Momentum.AchievedWR");
                }
                else if (pData->GetBool("isNewPersonalBest"))
                {
                    pPlayer->EmitSound("Momentum.AchievedPB");
                }
            }

            KeyValues *pXP = pData->FindKey("xp");
            if (pXP)
            {
                // MOM_TODO: Each of these have more info, add them to the event if/when needed (0.9.0+)
                const auto pCosXP = pXP->FindKey("cosXP");
                if (pCosXP)
                {
                    runUploadedEvent->SetInt("lvl_gain", pCosXP->GetInt("gainLvl"));
                    runUploadedEvent->SetInt("cos_xp", pCosXP->GetInt("gainXP"));
                    // oldXP: <int>
                }
                const auto pRankXP = pXP->FindKey("rankXP");
                if (pRankXP)
                {
                    runUploadedEvent->SetInt("rank_xp", pRankXP->GetInt("rankXP"));
                    // top10: <int>
                    // formula: <int>
                    // group: {
                    //   groupXP: <int>
                    //   groupNum: <int>
                    // }
                }
            }
            gameeventmanager->FireEvent(runUploadedEvent);
        }
    }
    else if (pErr)
    {
        if (runUploadedEvent)
        {
            runUploadedEvent->SetBool("run_posted", false);
            // MOM_TODO: send an error here
            gameeventmanager->FireEvent(runUploadedEvent);
        }
    }
}

bool CRunPoster::ShouldSubmitRun()
{
    return !m_bIsMappingMode && CheckCurrentMap() && m_uRunSessionID != 0;
}

bool CRunPoster::CheckCurrentMap()
{
    const auto pData = g_pMapCache->GetCurrentMapData();
    if (pData && pData->m_uID)
    {
        // Now check if the status is alright
        const auto status = pData->m_eMapStatus;
        if (status == MAP_APPROVED || status == MAP_PRIVATE_TESTING || status == MAP_PUBLIC_TESTING)
        {
            return true;
        }
    }
    return false;
}

void CRunPoster::ResetSession()
{
    m_uRunSessionID = 0;
    memset(m_iZoneEnterTicks, 0, MAX_ZONES * sizeof(m_iZoneEnterTicks[0]));
}

static CRunPoster s_momRunposter;
CRunPoster *g_pRunPoster = &s_momRunposter;