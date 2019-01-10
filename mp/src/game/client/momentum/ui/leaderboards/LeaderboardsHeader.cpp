#include "cbase.h"

#include "LeaderboardsHeader.h"

#include <vgui_controls/Label.h>

#include "fmtstr.h"

#include "mom_shareddefs.h"
#include "mom_map_cache.h"

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
        LoadMapData();
    }
}

void CLeaderboardsHeader::LoadMapData()
{
    MapData *pData = g_pMapCache->GetCurrentMapData();
    if (pData)
    {
        UpdateMapInfoLabel(pData);
        m_bMapInfoLoaded = true; // Stop this info from being loaded again
    }
    else
    {
        m_bMapInfoLoaded = true;
        m_pMapAuthor->SetVisible(false);
        m_pMapDetails->SetVisible(false);
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

void CLeaderboardsHeader::UpdateMapInfoLabel(MapData* pData)
{
    if (m_pMapAuthor)
    {
        if (pData->m_vecCredits.IsEmpty())
        {
            m_pMapAuthor->SetText("");
        }
        else
        {
            CUtlString authorsString("By ");
            CUtlStringList authors;
            FOR_EACH_VEC(pData->m_vecCredits, i)
            {
                if (pData->m_vecCredits[i].m_eType == CREDIT_AUTHOR)
                {
                    authors.CopyAndAddToTail(pData->m_vecCredits[i].m_User.m_szAlias);
                }
            }
            int count = authors.Count();
            for (int i = 0; i < count; i++)
            {
                authorsString.Append(authors[i]);
                if (i < count - 2)
                    authorsString.Append(", ", 2);
            }

            m_pMapAuthor->SetText(authorsString.Get());
        }
    }

    CFmtStr layout("%s (%i zone%s)", pData->m_Info.m_bIsLinear ? "LINEAR" : "STAGED", pData->m_Info.m_iNumZones, pData->m_Info.m_iNumZones > 1 ? "s" : "");
    CFmtStr bonuses(" - %i BONUS%s", pData->m_Info.m_iNumBonuses, pData->m_Info.m_iNumBonuses > 1 ? "ES" : "");

    char mapDetails[BUFSIZ];
    Q_snprintf(mapDetails, BUFSIZ, "TIER %i - %s%s", pData->m_Info.m_iDifficulty, layout.Get(), pData->m_Info.m_iNumBonuses ? bonuses.Get() : "");

    if (m_pMapDetails)
    {
        m_pMapDetails->SetText(mapDetails);
    }
}