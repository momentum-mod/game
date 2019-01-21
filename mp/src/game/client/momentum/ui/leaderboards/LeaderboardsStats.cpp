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

    m_pPlayerAvatar = new CAvatarImagePanel(this, "PlayerAvatar");

    LoadControlSettings("resource/ui/leaderboards/stats.res");

    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);

    m_pPlayerName = FindControl<Label>("PlayerName");
    m_pPlayerMapRank = FindControl<Label>("PlayerMapRank");
    m_pPlayerPersonalBest = FindControl<Label>("PlayerPersonalBest");
    m_pPlayerGlobalRank = FindControl<Label>("PlayerGlobalRank");
    m_pPlayerRankXP = FindControl<Label>("RankXP");
    m_pPlayerCosXP = FindControl<Label>("CosXP");
    m_pMapsCompleted = FindControl<Label>("MapsCompleted");
    m_pRunsSubmitted = FindControl<Label>("RunsSubmitted");
    m_pTotalJumps = FindControl<Label>("TotalJumps");
    m_pTotalStrafes = FindControl<Label>("TotalStrafes");

    m_pPlayerAvatar->SetDefaultAvatar(scheme()->GetImage("default_steam", false));
    m_pPlayerAvatar->SetShouldScaleImage(true);
    m_pPlayerAvatar->SetShouldDrawFriendIcon(false);
}

void CLeaderboardsStats::LoadData(bool bFullUpdate)
{
    SetVisible(false); // Hidden so it is not seen being changed

    UpdatePlayerAvatarStandalone();

    player_info_t pi;
    char newName[MAX_PLAYER_NAME_LENGTH];
    engine->GetPlayerInfo(engine->GetLocalPlayer(), &pi);
    UTIL_MakeSafeName(pi.name, newName, MAX_PLAYER_NAME_LENGTH);
    m_pPlayerName->SetText(newName);

    float flLastUp = gpGlobals->curtime - m_flLastUpdate;
    if (bFullUpdate && ((g_pMapCache->GetCurrentMapID() && flLastUp >= UPDATE_INTERVAL) || m_bNeedsUpdate))
    {
        wchar_t *waiting = g_pVGuiLocalize->Find("MOM_API_WaitingForResponse");

        m_pPlayerMapRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapRank"), waiting));
        m_pPlayerGlobalRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_GlobalRank"), waiting));
        m_pPlayerPersonalBest->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_PersonalBestTime"), waiting));
        m_pPlayerRankXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RankXP"), waiting));
        m_pPlayerCosXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_CosXP"), waiting));
        m_pMapsCompleted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapsCompleted"), waiting));
        m_pRunsSubmitted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RunsSubmitted"), waiting));
        m_pTotalJumps->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalJumps"), waiting));
        m_pTotalStrafes->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalStrafes"), waiting));

        uint64 uID = SteamUser()->GetSteamID().ConvertToUint64();
        g_pAPIRequests->GetUserStatsAndMapRank(uID, g_pMapCache->GetCurrentMapID(), UtlMakeDelegate(this, &CLeaderboardsStats::OnPlayerStats));
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
        int mrank = -1;
        // int mtotal = -1; // MOM_TODO

        int grank = -1; // MOM_TODO
        int gtotal = -1; // MOM_TODO

        float seconds = 0.0f;

        KeyValues *pMapRank = pData->FindKey("mapRanks");
        if (pMapRank)
        {
            pMapRank = pMapRank->FindKey("1");
            if (pMapRank)
            {
                mrank = pMapRank->GetInt("rank");

                KeyValues *pRun = pMapRank->FindKey("run");
                if (pRun)
                {
                    seconds = pRun->GetFloat("time");
                }
            }
        }

        KeyValues *pUserStats = pData->FindKey("stats");
        if (pUserStats)
        {
            // MOM_TODO: fill in these
            // grank = static_cast<int>(pExperience->GetFloat("rank"));
            // gtotal = static_cast<int>(pExperience->GetFloat("total"));

            m_pPlayerRankXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RankXP"), pUserStats->GetInt("rankXP")));
            m_pPlayerCosXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_CosXP"), pUserStats->GetInt("cosXP")));
            m_pMapsCompleted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapsCompleted"), pUserStats->GetInt("mapsCompleted")));
            m_pRunsSubmitted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RunsSubmitted"), pUserStats->GetInt("runsSubmitted")));
            m_pTotalJumps->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalJumps"), pUserStats->GetInt("totalJumps")));
            m_pTotalStrafes->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalStrafes"), pUserStats->GetInt("totalStrafes")));
        }

        if (mrank > -1)
        {
            m_pPlayerMapRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapRank"), mrank));
        }
        else
        {
            m_pPlayerMapRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapRank"), g_pVGuiLocalize->Find("MOM_NotApplicable")));
        }
        if (seconds > 0.0f)
        {
            char p_sPersonalBestTime[BUFSIZETIME];
            wchar_t w_PB[BUFSIZETIME];
            g_pMomentumUtil->FormatTime(seconds, p_sPersonalBestTime);
            ANSI_TO_UNICODE(p_sPersonalBestTime, w_PB);
            m_pPlayerPersonalBest->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_PersonalBestTime"), w_PB));
        }
        else
        {
            m_pPlayerPersonalBest->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_PersonalBestTime"), g_pVGuiLocalize->Find("MOM_NotApplicable")));
        }
        if (grank > -1 && gtotal > -1)
        {
            char p_sGlobalRank[BUFSIZELOCL];
            char p_sLocalized[BUFSIZELOCL];
            LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
            Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sGlobalRank, grank, gtotal);
            m_pPlayerGlobalRank->SetText(p_sLocalized);
        }
    }
    else if (pErr)
    {
        // MOM_TODO: Handle errors
    }
}

void CLeaderboardsStats::UpdatePlayerAvatarStandalone()
{
    CHECK_STEAM_API(SteamUser());
    // Update their avatar only if need be
    if (!m_bLoadedLocalPlayerAvatar)
    {
        CSteamID steamIDForPlayer = SteamUser()->GetSteamID();

        m_pPlayerAvatar->SetPlayer(steamIDForPlayer, k_EAvatarSize184x184);

        m_bLoadedLocalPlayerAvatar = true;
    }
}