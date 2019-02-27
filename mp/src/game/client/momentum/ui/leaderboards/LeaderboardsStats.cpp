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
    m_pPlayerPersonalBest = new Label(this, "PlayerPersonalBest", "");
    m_pPlayerGlobalRank = new Label(this, "PlayerGlobalRank", "");
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


            pMapRank->SetWString("time", g_pVGuiLocalize->Find("MOM_NotApplicable"));
            KeyValues *pRun = pMapRank->FindKey("run");
            if (pRun)
            {
                float seconds = pRun->GetFloat("time");
                if (seconds > 0.0f)
                {
                    char sPersonalBestTime[BUFSIZETIME];
                    g_pMomentumUtil->FormatTime(seconds, sPersonalBestTime);
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

            m_pPlayerRankXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RankXP"), pUserStats));
            m_pPlayerCosXP->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_CosXP"), pUserStats));
            m_pMapsCompleted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapsCompleted"), pUserStats));
            m_pRunsSubmitted->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_RunsSubmitted"), pUserStats));
            m_pTotalJumps->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalJumps"), pUserStats));
            m_pTotalStrafes->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_TotalStrafes"), pUserStats));
        }

        m_pPlayerMapRank->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_MapRank"), pMapRank));
        m_pPlayerPersonalBest->SetText(CConstructLocalizedString(g_pVGuiLocalize->Find("MOM_PersonalBestTime"), pMapRank));

        /*if (grank > -1 && gtotal > -1)
        {
            char p_sGlobalRank[BUFSIZELOCL];
            char p_sLocalized[BUFSIZELOCL];
            LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
            Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sGlobalRank, grank, gtotal);
            m_pPlayerGlobalRank->SetText(p_sLocalized);
        }*/
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