#include "cbase.h"

#include "LeaderboardsStats.h"

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "vgui_avatarimage.h"
#include "vgui/ILocalize.h"

#include "mom_shareddefs.h"
#include "mom_map_cache.h"
#include "mom_api_requests.h"
#include "util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define UPDATE_INTERVAL 25.0f // 25 seconds between updates

CLeaderboardsStats::CLeaderboardsStats(Panel* pParent) : BaseClass(pParent, "CLeaderboardsStats")
{
    m_bLoadedLocalPlayerAvatar = false;
    m_flLastUpdate = 0.0f;
    m_bNeedsUpdate = true;

    SetSize(10, 10);

    m_pPlayerAvatar = new CAvatarImagePanel(this, "PlayerAvatar");

    m_pPlayerName = new Label(this, "PlayerName", "");
    m_pPlayerMapRank = new Label(this, "PlayerMapRank", "");
    m_pPlayerLevel = new Label(this, "PlayerLevel", "");
    m_pPlayerRankXP = new Label(this, "RankXP", "");
    m_pPlayerCosXP = new Label(this, "CosXP", "");
    m_pMapsCompleted = new Label(this, "MapsCompleted", "");
    m_pRunsSubmitted = new Label(this, "RunsSubmitted", "");
    m_pTotalJumps = new Label(this, "TotalJumps", "");
    m_pTotalStrafes = new Label(this, "TotalStrafes", "");

    LoadControlSettings("resource/ui/leaderboards/stats.res");

    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);

    m_pPlayerAvatar->SetDefaultAvatar(scheme()->GetImage("default_steam", false));
    m_pPlayerAvatar->SetShouldScaleImage(true);
    m_pPlayerAvatar->SetShouldDrawFriendIcon(false);
}

void CLeaderboardsStats::LoadData(bool bFullUpdate)
{
    SetVisible(false); // Hidden so it is not seen being changed

    CHECK_STEAM_API(SteamUser());

    UpdatePlayerAvatarStandalone();

    player_info_t pi;
    char newName[MAX_PLAYER_NAME_LENGTH];
    engine->GetPlayerInfo(engine->GetLocalPlayer(), &pi);
    UTIL_MakeSafeName(pi.name, newName, MAX_PLAYER_NAME_LENGTH);
    m_pPlayerName->SetText(newName);

    const float flLastUp = gpGlobals->curtime - m_flLastUpdate;
    if (bFullUpdate && ((g_pMapCache->GetCurrentMapID() && flLastUp >= UPDATE_INTERVAL) || m_bNeedsUpdate))
    {
        wchar_t *waiting = g_pVGuiLocalize->Find("MOM_API_WaitingForResponse");
        const wchar_t *unavail = L" - ";

        const bool bSuccess = g_pAPIRequests->GetUserStatsAndMapRank(0,
                                                               g_pMapCache->GetCurrentMapID(),
                                                               UtlMakeDelegate(this, &CLeaderboardsStats::OnPlayerStats));

        KeyValuesAD dummyObj("dummy");
        dummyObj->SetWString("mRank", bSuccess ? waiting : unavail);
        dummyObj->SetWString("time", bSuccess ? waiting : unavail);
        dummyObj->SetWString("level", bSuccess ? waiting : unavail);
        dummyObj->SetWString("gRank", bSuccess ? waiting : unavail);
        dummyObj->SetWString("gRank", bSuccess ? waiting : unavail);
        dummyObj->SetWString("rankXP", bSuccess ? waiting : unavail);
        dummyObj->SetWString("cosXP", bSuccess ? waiting : unavail);
        dummyObj->SetWString("mapsCompleted", bSuccess ? waiting : unavail);
        dummyObj->SetWString("runsSubmitted", bSuccess ? waiting : unavail);
        dummyObj->SetWString("totalJumps", bSuccess ? waiting : unavail);
        dummyObj->SetWString("totalStrafes", bSuccess ? waiting : unavail);
        KeyValues *pDumDum = dummyObj;

        m_pPlayerLevel->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_Level"), pDumDum));
        m_pPlayerMapRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapRank"), pDumDum));
        m_pPlayerRankXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RankXP"), pDumDum));
        m_pPlayerCosXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_CosXP"), pDumDum));
        m_pMapsCompleted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapsCompleted"), pDumDum));
        m_pRunsSubmitted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RunsSubmitted"), pDumDum));
        m_pTotalJumps->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalJumps"), pDumDum));
        m_pTotalStrafes->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalStrafes"), pDumDum));
        
        if (bSuccess)
            m_flLastUpdate = gpGlobals->curtime;

        m_bNeedsUpdate = false;
    }

    SetVisible(true); // And seen again!
}

void CLeaderboardsStats::OnPlayerStats(KeyValues* kv)
{
    KeyValues *pData = kv->FindKey("data");
    KeyValues *pErr = kv->FindKey("error");
    if (pData)
    {
        // int mtotal = -1; // MOM_TODO

        // int grank = -1; // MOM_TODO
        // int gtotal = -1; // MOM_TODO


        KeyValues *pMapRank = pData->FindKey("mapRank");
        if (pMapRank)
        {
            int iMapRank = pMapRank->GetInt("rank", -1);
            if (iMapRank == -1)
                pMapRank->SetWString("mRank", g_pVGuiLocalize->Find("MOM_NotApplicable"));
            else
                pMapRank->SetInt("mRank", iMapRank);

            const auto iRankXP = pMapRank->GetInt("rankXP", -1);
            if (iRankXP == -1)
                pMapRank->SetWString("rankXP", g_pVGuiLocalize->Find("MOM_NotApplicable"));
            else
                pMapRank->SetInt("rankXP", iRankXP);

            pMapRank->SetWString("time", g_pVGuiLocalize->Find("MOM_NotApplicable"));
            KeyValues *pRun = pMapRank->FindKey("run");
            if (pRun)
            {
                float seconds = pRun->GetFloat("time");
                if (seconds > 0.0f)
                {
                    char sPersonalBestTime[BUFSIZETIME];
                    MomUtil::FormatTime(seconds, sPersonalBestTime);
                    pMapRank->SetString("time", sPersonalBestTime);
                }
            }
        }

        KeyValues *pUserStats = pData->FindKey("stats");
        if (pUserStats)
        {
            // MOM_TODO: fill in these
            // grank = static_cast<int>(pExperience->GetFloat("rank"));
            // gtotal = static_cast<int>(pExperience->GetFloat("total"));

            m_pPlayerLevel->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_Level"), pUserStats));
            m_pPlayerCosXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_CosXP"), pUserStats));
            m_pMapsCompleted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapsCompleted"), pUserStats));
            m_pRunsSubmitted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RunsSubmitted"), pUserStats));
            m_pTotalJumps->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalJumps"), pUserStats));
            m_pTotalStrafes->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalStrafes"), pUserStats));
        }

        m_pPlayerMapRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapRank"), pMapRank));
        m_pPlayerRankXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RankXP"), pMapRank));
    }
    else if (pErr)
    {
        // MOM_TODO: Handle errors
    }
}

void CLeaderboardsStats::UpdatePlayerAvatarStandalone()
{
    // Update their avatar only if need be
    if (!m_bLoadedLocalPlayerAvatar)
    {
        CSteamID steamIDForPlayer = SteamUser()->GetSteamID();

        m_pPlayerAvatar->SetPlayer(steamIDForPlayer, k_EAvatarSize184x184);

        m_bLoadedLocalPlayerAvatar = true;
    }
}