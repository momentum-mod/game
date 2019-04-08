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
    SetSize(10, 10);

    m_pMapName = new Label(this, "MapName", "<Map Name Here>");
    m_pMapAuthor = new Label(this, "MapAuthor", "");
    m_pMapDetails = new Label(this, "MapDetails", "");

    LoadControlSettings("resource/ui/leaderboards/header.res");
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
        CUtlString authorsString("By ");
        if (!pData->GetCreditString(&authorsString, CREDIT_AUTHOR))
            authorsString.Clear();
            
        m_pMapAuthor->SetText(authorsString.Get());
    }

    CFmtStr layout("%s (%i zone%s)", pData->m_MainTrack.m_bIsLinear ? "LINEAR" : "STAGED", pData->m_MainTrack.m_iNumZones, pData->m_MainTrack.m_iNumZones > 2 ? "s" : "");
    CFmtStr bonuses(" - %i BONUS%s", pData->m_Info.m_iNumTracks - 1, pData->m_Info.m_iNumTracks - 1 ? "ES" : "");

    char mapDetails[BUFSIZ];
    Q_snprintf(mapDetails, BUFSIZ, "TIER %i - %s%s", pData->m_MainTrack.m_iDifficulty, layout.Get(), pData->m_Info.m_iNumTracks > 1 ? bonuses.Get() : "");

    if (m_pMapDetails)
    {
        m_pMapDetails->SetText(mapDetails);
    }
}