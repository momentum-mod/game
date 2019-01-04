#include "cbase.h"

#include "LeaderboardsHeader.h"

#include <vgui_controls/Label.h>

#include <vgui/ISurface.h>

// Lastly
#include "tier0/memdbgon.h"

using namespace vgui;

CLeaderboardsHeader::CLeaderboardsHeader(Panel* pParent) : BaseClass(pParent, "CLeaderboardsHeader")
{
    m_bMapInfoLoaded = false;

    LoadControlSettings("resource/ui/leaderboards/header.res");

    m_pMapName = FindControl<Label>("MapName", true);
    m_pMapAuthor = FindControl<Label>("MapAuthor", true);
    m_pMapDetails = FindControl<Label>("MapDetails", true);
}

void CLeaderboardsHeader::LoadData(const char *pMapName, bool bFullUpdate)
{
    if (m_pMapName && pMapName)
    {
        m_pMapName->SetText(pMapName);
        // Set the author label to be at the end of this label
        // MOM_TODO: use pinning to put it here instead of this

        int wide, tall;
        m_pMapName->GetContentSize(wide, tall);
        m_pMapAuthor->SetPos(m_pMapName->GetXPos() + wide + GetScaledVal(4),
                             m_pMapName->GetYPos() + tall - GetScaledVal(surface()->GetFontTall(m_pMapAuthor->GetFont())));
    }

    if (bFullUpdate && !m_bMapInfoLoaded)
    {
        // MOM_TODO: actual API call for map info here
        /*char requrl[MAX_PATH];
        Q_snprintf(requrl, MAX_PATH, "%s/getmapinfo/%s", MOM_APIDOMAIN, g_pGameRules->MapName());
        g_pMomentumUtil->CreateAndSendHTTPReq(requrl, &cbGetMapInfoCallback, &CClientTimesDisplay::GetMapInfoCallback, this);*/
    }
}

void CLeaderboardsHeader::OnGetPlayerDataForMap(KeyValues* pKv)
{
    /*Warning("%s - Callback received.\n", __FUNCTION__);
    if (bIOFailure)
        return;

    uint32 size;
    SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        Warning("%s - size is 0!\n", __FUNCTION__);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    JsonValue val; // Outer object
    JsonAllocator alloc;
    char *pDataPtr = reinterpret_cast<char *>(pData);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
        {
            KeyValues *pResponse = CJsonToKeyValues::ConvertJsonToKeyValues(val.toNode());
            KeyValues::AutoDelete ad(pResponse);

            int mrank = -1;
            int mtotal = -1;

            int grank = -1;
            int gtotal = -1;
            int gexp = -1;

            float seconds = 0.0f;

            KeyValues *pRun = pResponse->FindKey("run");
            if (pRun)
            {
                mrank = static_cast<int>(pRun->GetFloat("rank"));
                seconds = pRun->GetFloat("time");
            }

            KeyValues *pMap = pResponse->FindKey("mapranking");
            if (pMap)
            {
                mtotal = static_cast<int>(pMap->GetFloat("total", -1.0f));
            }

            KeyValues *pExperience = pResponse->FindKey("globalranking");
            if (pExperience)
            {
                grank = static_cast<int>(pExperience->GetFloat("rank"));
                gtotal = static_cast<int>(pExperience->GetFloat("total"));
                gexp = static_cast<int>(pExperience->GetFloat("experience"));
            }

            if (mrank > -1 && mtotal > -1)
            {
                char p_sMapRank[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcMapRank, "MOM_MapRank", p_sMapRank);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sMapRank, mrank, mtotal);
                m_pPlayerMapRank->SetText(p_sLocalized);
            }
            if (seconds > 0.0f)
            {
                char p_sPersonalBestTime[BUFSIZETIME];
                char p_sPersonalBest[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                g_pMomentumUtil->FormatTime(seconds, p_sPersonalBestTime);
                LOCALIZE_TOKEN(p_wcPersonalBest, "MOM_PersonalBestTime", p_sPersonalBest);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %s", p_sPersonalBest, p_sPersonalBestTime);
                m_pPlayerPersonalBest->SetText(p_sLocalized);
            }

            if (grank > -1 && gtotal > -1)
            {
                char p_sGlobalRank[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sGlobalRank, grank, gtotal);
                m_pPlayerGlobalRank->SetText(p_sLocalized);

                char p_sExperience[BUFSIZELOCL];
                char p_sLocalized2[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcExperience, "MOM_ExperiencePoints", p_sExperience);
                Q_snprintf(p_sLocalized2, BUFSIZELOCL, "%s: %i", p_sExperience, gexp);
                m_pPlayerExperience->SetText(p_sLocalized2);
            }
            m_fLastHeaderUpdate = gpGlobals->curtime;
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    alloc.deallocate();
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);*/
}

void CLeaderboardsHeader::OnGetMapInfo(KeyValues* pKv)
{
    /*Warning("%s - Callback received.\n", __FUNCTION__);
    if (bIOFailure)
    {
        UpdateMapInfoLabel(); // Default param is nullptr, so it hides it
        Warning("%s - bIOFailure is true!\n", __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode409Conflict ||
        pCallback->m_eStatusCode == k_EHTTPStatusCode404NotFound)
    {
        char locl[BUFSIZELOCL];
        LOCALIZE_TOKEN(staged, "MOM_API_Unavailable", locl);
        UpdateMapInfoLabel(locl);
        Warning("%s - Map not found on server!\n", __FUNCTION__);
        return;
    }

    uint32 size;
    SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        UpdateMapInfoLabel();
        Warning("%s - size is 0!\n", __FUNCTION__);
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    JsonValue val; // Outer object
    JsonAllocator alloc;
    char *pDataPtr = reinterpret_cast<char *>(pData);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
        {
            KeyValues *pResponse = CJsonToKeyValues::ConvertJsonToKeyValues(val.toNode());
            KeyValues::AutoDelete ad(pResponse);
            if (pResponse)
            {
                const char *author = pResponse->GetString("submitter", "Unknown");
                const int tier = pResponse->GetInt("difficulty", -1);
                const int bonus = pResponse->GetInt("bonus", -1);
                char layout[BUFSIZELOCL];
                if (pResponse->GetBool("linear", false))
                {
                    LOCALIZE_TOKEN(linear, "MOM_Linear", layout);
                }
                else
                {
                    Q_snprintf(layout, BUFSIZELOCL, "%i STAGES", pResponse->GetInt("zones", -1));
                }

                UpdateMapInfoLabel(author, tier, layout, bonus);
                m_bMapInfoLoaded = true; // Stop this info from being fetched again
            }
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    alloc.deallocate();
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);*/
}

void CLeaderboardsHeader::UpdateMapInfoLabel(const char* text)
{
    if (m_pMapDetails)
    {
        m_pMapDetails->SetText(text);
        if (text == nullptr)
        {
            m_pMapDetails->SetVisible(false);
        }
    }
}

void CLeaderboardsHeader::UpdateMapInfoLabel(const char* author, const int tier, const char* layout, const int bonus)
{
    if (m_pMapDetails && m_pMapAuthor)
    {
        char mapAuthor[MAX_PLAYER_NAME_LENGTH + 3];
        Q_snprintf(mapAuthor, MAX_PLAYER_NAME_LENGTH + 3, "By %s", author);
        m_pMapAuthor->SetText(mapAuthor);

        char mapDetails[BUFSIZ];
        Q_snprintf(mapDetails, BUFSIZ, "TIER %i - %s - %i BONUS", tier, layout, bonus);
        UpdateMapInfoLabel(mapDetails);
    }
}
