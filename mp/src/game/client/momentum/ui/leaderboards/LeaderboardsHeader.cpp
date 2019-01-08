#include "cbase.h"

#include "LeaderboardsHeader.h"

#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>

#include "fmtstr.h"

#include "mom_api_requests.h"
#include "mom_run_poster.h"
#include "mom_shareddefs.h"

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
    }

    if (bFullUpdate && !m_bMapInfoLoaded)
    {
        // MOM_TODO: Use something better than the run poster to store map ID
        if (g_pRunPoster->m_iMapID)
        {
            g_pAPIRequests->GetMapInfo(g_pRunPoster->m_iMapID, UtlMakeDelegate(this, &CLeaderboardsHeader::OnGetMapInfo));
            wchar_t *waiting = g_pVGuiLocalize->Find("MOM_API_WaitingForResponse");
            m_pMapAuthor->SetText(waiting);
            m_pMapDetails->SetText(waiting);
        }
        else
        {
            m_bMapInfoLoaded = true;
            m_pMapAuthor->SetVisible(false);
            m_pMapDetails->SetVisible(false);
        }
    }
}

void CLeaderboardsHeader::Reset()
{
    m_bMapInfoLoaded = false;
    m_pMapAuthor->SetVisible(true);
    m_pMapDetails->SetVisible(true);
    m_pMapAuthor->SetText("");
    m_pMapDetails->SetText("");
}

void CLeaderboardsHeader::OnGetMapInfo(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        int tier = 0, numBonus = 0, numZones = 0;
        bool isLinear = true;
        CUtlStringList authors;

        KeyValues *pInfo = pData->FindKey("info");
        if (pInfo)
        {
            tier = pInfo->GetInt("difficulty");
            numBonus = pInfo->GetInt("numBonuses");
            numZones = pInfo->GetInt("numZones");
            isLinear = pInfo->GetBool("isLinear");
        }

        KeyValues *pCredits = pData->FindKey("credits");
        if (pCredits)
        {
            FOR_EACH_SUBKEY(pCredits, pCredit)
            {
                KeyValues *pUser = pCredit->FindKey("user");
                if (pCredit->GetInt("type", -1) == CREDIT_AUTHOR && pUser)
                {
                    authors.CopyAndAddToTail(pUser->GetString("alias", "<NULL>"));
                }
            }
        }

        UpdateMapInfoLabel(authors, tier, isLinear, numZones, numBonus);
        m_bMapInfoLoaded = true; // Stop this info from being fetched again
    }
    else if (pErr)
    {
        int code = pKv->GetInt("code");
        if (code == k_EHTTPStatusCode404NotFound)
        {
            // Map not found, don't update again
            m_pMapAuthor->SetVisible(false);
            m_pMapDetails->SetVisible(false);
        } 
        else
        {
            // Some other error, may be worth retrying
            m_bMapInfoLoaded = false;
        }
    }
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

void CLeaderboardsHeader::UpdateMapInfoLabel(CUtlVector<char*>& vecAuthors, int tier, bool bIsLinear, int numZones, int numBonuses)
{
    if (m_pMapAuthor)
    {
        if (vecAuthors.IsEmpty())
        {
            m_pMapAuthor->SetText("");
        }
        else
        {
            CUtlString authorsString("By ");
            int count = vecAuthors.Count();
            for (int i = 0; i < count; i++)
            {
                authorsString.Append(vecAuthors[i]);
                if (i < count - 2)
                    authorsString.Append(", ", 2);
            }

            m_pMapAuthor->SetText(authorsString.Get());
        }
    }

    CFmtStr layout("%s (%i zone%s)", bIsLinear ? "LINEAR" : "STAGED", numZones, numZones > 1 ? "s" : "");
    CFmtStr bonuses(" - %i BONUS%s", numBonuses, numBonuses > 1 ? "ES" : "");

    char mapDetails[BUFSIZ];
    Q_snprintf(mapDetails, BUFSIZ, "TIER %i - %s%s", tier, layout.Get(), numBonuses ? bonuses.Get() : "");
    UpdateMapInfoLabel(mapDetails);
}