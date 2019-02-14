#include "cbase.h"

#include "BrowseMaps.h"
#include "MapSelectorDialog.h"
#include "CMapListPanel.h"

#include "util/mom_util.h"
#include "mom_api_requests.h"

#include "mom_map_cache.h"

#include "fmtstr.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define MAP_SEARCH_INTERVAL 3.0f // Every 3 seconds

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBrowseMaps::CBrowseMaps(Panel *parent) : CBaseMapsPage(parent, "BrowseMaps")
{
    m_fPrevSearchTime = 0.0f;
    m_pMapList->SetSortColumnEx(HEADER_DATE_CREATED, HEADER_DIFFICULTY, true);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBrowseMaps::~CBrowseMaps()
{
}


void CBrowseMaps::MapsQueryCallback(KeyValues *pKvResponse)
{
    KeyValues *pKvData = pKvResponse->FindKey("data");
    KeyValues *pKvErr = pKvResponse->FindKey("error");
    if (pKvErr)
    {
        // MOM_TODO: error handle
        RefreshComplete(pKvResponse->GetInt("code") == 0 ? eNoServerReturn : eServerBadReturn);
        return;
    }

    if (pKvData)
    {
        KeyValues *pMaps = pKvData->FindKey("maps");
        if (g_pMapCache->AddMapsToCache(pMaps, MODEL_FROM_SEARCH_API_CALL))
        {
            GetNewMapList();
            
            RefreshComplete(eSuccess);
        }
        else
            RefreshComplete(eNoMapsReturned);
    }
}

void CBrowseMaps::GetSearchFilters(KeyValues* pInto)
{
    MapFilters_t filters = GetFilters();

    // Name should be search
    if (Q_strlen(filters.m_szMapName))
        pInto->SetString("search", filters.m_szMapName);

    // Difficulty range
    if (filters.m_iDifficultyLow > 0)
        pInto->SetInt("difficulty_low", filters.m_iDifficultyLow);
    if (filters.m_iDifficultyHigh > 0)
        pInto->SetInt("difficulty_high", filters.m_iDifficultyHigh);

    // Layout
    int layout = filters.m_iMapLayout;
    if (layout == 1)
        pInto->SetBool("isLinear", false);
    else if (layout == 2)
        pInto->SetBool("isLinear", true);

    // Type
    int type = filters.m_iGameMode;
    if (type > 0)
        pInto->SetInt("type", type);
}

void CBrowseMaps::SearchMaps()
{
    if (SteamHTTP())
    {
        KeyValuesAD kvSearchFilters("SearchFilters");
        GetSearchFilters(kvSearchFilters);
        if (g_pAPIRequests->GetMaps(kvSearchFilters, UtlMakeDelegate(this, &CBrowseMaps::MapsQueryCallback)))
            m_fPrevSearchTime = gpGlobals->curtime;
    }
}

void CBrowseMaps::ApplyFilters(MapFilters_t filters)
{
    // We do a little redirecting here, we need to search with these filters, and also apply to results
    if (!(m_PreviousFilters == filters) && gpGlobals->curtime - m_fPrevSearchTime > MAP_SEARCH_INTERVAL)
    {
        SearchMaps();
        m_PreviousFilters = filters;
    }

    // We still apply these filters regardless, to our maps in the cache
    BaseClass::ApplyFilters(filters);
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh if needed
//-----------------------------------------------------------------------------
void CBrowseMaps::OnTabSelected()
{
    GetNewMapList();
    ApplyFilters(GetFilters());
}

void CBrowseMaps::OnGetNewMapList()
{
    OnApplyFilters(GetFilters());
}

void CBrowseMaps::RefreshComplete(EMapQueryOutputs eResponse)
{
    switch (eResponse)
    {
        case eNoMapsReturned: 
            m_pMapList->SetEmptyListText("#ServerBrowser_NoInternetGames"); 
            break;
        case eNoServerReturn: 
        case eServerBadReturn:
            m_pMapList->SetEmptyListText("#ServerBrowser_MasterServerNotResponsive");
            break;
        case eSuccess: 
        default:
            break;
    }

    UpdateStatus();
}
