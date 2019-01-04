#include "cbase.h"

#include "LeaderboardsStats.h"

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "vgui_avatarimage.h"
#include "vgui/ILocalize.h"

#include "mom_shareddefs.h"
#include "mom_api_requests.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define UPDATE_INTERVAL 15.0f // 15 seconds between updates

CLeaderboardsStats::CLeaderboardsStats(Panel* pParent) : BaseClass(pParent, "CLeaderboardsStats")
{
    m_bLoadedLocalPlayerAvatar = false;
    m_flLastUpdate = 0.0f;
    m_bNeedsUpdate = true;

    LoadControlSettings("resource/ui/leaderboards/stats.res");

    m_pPlayerAvatar = FindControl<ImagePanel>("PlayerAvatar", true);
    m_pPlayerName = FindControl<Label>("PlayerName", true);
    m_pPlayerMapRank = FindControl<Label>("PlayerMapRank", true);
    m_pPlayerPersonalBest = FindControl<Label>("PlayerPersonalBest", true);
    m_pPlayerGlobalRank = FindControl<Label>("PlayerGlobalRank", true);
    m_pPlayerExperience = FindControl<Label>("PlayerExperience", true);
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

    // What this if is:
    // We want to do a full update if (we ask for it with fullUpdate boolean AND (the minimum time has passed OR it is
    // the first update)) OR the maximum time has passed
    float flLastUp = gpGlobals->curtime - m_flLastUpdate;
    if (bFullUpdate && (flLastUp >= UPDATE_INTERVAL || m_bNeedsUpdate))
    {
        char p_sCalculating[BUFSIZELOCL];
        char p_sWaitingResponse[BUFSIZELOCL];
        LOCALIZE_TOKEN(p_wcCalculating, "MOM_Calculating", p_sCalculating);
        LOCALIZE_TOKEN(p_wcWaitingResponse, "MOM_API_WaitingForResponse", p_sWaitingResponse);

        char p_sMapRank[BUFSIZELOCL];
        char p_sGlobalRank[BUFSIZELOCL];
        char p_sPersonalBest[BUFSIZELOCL];
        char p_sExperiencePoints[BUFSIZELOCL];

        char mrLocalized[BUFSIZELOCL];
        char grLocalized[BUFSIZELOCL];
        char pbLocalized[BUFSIZELOCL];
        char xpLocalized[BUFSIZELOCL];

        LOCALIZE_TOKEN(p_wcMapRank, "MOM_MapRank", p_sMapRank);
        LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
        LOCALIZE_TOKEN(p_wcPersonalBest, "MOM_PersonalBestTime", p_sPersonalBest);
        LOCALIZE_TOKEN(p_wcExperiencePoints, "MOM_ExperiencePoints", p_sExperiencePoints);

        Q_snprintf(mrLocalized, BUFSIZELOCL, "%s: %s", p_sMapRank, p_sCalculating);
        Q_snprintf(grLocalized, BUFSIZELOCL, "%s: %s", p_sGlobalRank, p_sCalculating);
        Q_snprintf(pbLocalized, BUFSIZELOCL, "%s: %s", p_sPersonalBest, p_sWaitingResponse);
        Q_snprintf(xpLocalized, BUFSIZELOCL, "%s: %s", p_sExperiencePoints, p_sWaitingResponse);

        m_pPlayerMapRank->SetText(mrLocalized);
        m_pPlayerGlobalRank->SetText(grLocalized);
        m_pPlayerPersonalBest->SetText(pbLocalized);
        m_pPlayerExperience->SetText(xpLocalized);

        // MOM_TODO: Get player map rank info for this map
        // g_pAPIRequests->GetPlayerStats()
        m_flLastUpdate = gpGlobals->curtime;
        /*char requrl[MAX_PATH];
        // Mapname, tickrate, rank, radius
        Q_snprintf(requrl, MAX_PATH, "%s/getusermaprank/%s/%llu", MOM_APIDOMAIN, g_pGameRules->MapName(),
                   GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64());
        g_pMomentumUtil->CreateAndSendHTTPReq(requrl, &cbGetPlayerDataForMapCallback, &CClientTimesDisplay::GetPlayerDataForMapCallback, this);
        m_fLastHeaderUpdate = gpGlobals->curtime;*/
    }

    SetVisible(true); // And seen again!
}

void CLeaderboardsStats::UpdatePlayerAvatarStandalone()
{
    // Update their avatar
    if (SteamUser())
    {
        if (!m_bLoadedLocalPlayerAvatar)
        {
            CSteamID steamIDForPlayer = SteamUser()->GetSteamID();

            CAvatarImage *pImage = new CAvatarImage();
            // 64 is enough up to full HD resolutions.
            pImage->SetAvatarSteamID(steamIDForPlayer, k_EAvatarSize64x64);

            pImage->SetDrawFriend(false);
            pImage->SetAvatarSize(64, 64); // Deliberately non scaling, the ImagePanel does that for us

            // Get rid of the other image if it was there
            m_pPlayerAvatar->EvictImage();

            m_pPlayerAvatar->SetImage(pImage);
            m_bLoadedLocalPlayerAvatar = true;
        }
    }
    else
    {
        m_pPlayerAvatar->SetImage("default_steam");
    }
}
