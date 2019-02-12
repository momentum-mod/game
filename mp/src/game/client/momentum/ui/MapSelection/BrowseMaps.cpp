#include "cbase.h"

#include "BrowseMaps.h"
#include "MapSelectorDialog.h"
#include "CMapListPanel.h"

#include "util/mom_util.h"
#include "mom_api_requests.h"

#include "mom_map_cache.h"

#include <OfflineMode.h>
#include "fmtstr.h"

#include "tier0/memdbgon.h"


using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBrowseMaps::CBrowseMaps(Panel *parent) : CBaseMapsPage(parent, "BrowseMaps")
{
    m_bRequireUpdate = true;
    m_bOfflineMode = !IsSteamGameServerBrowsingEnabled();
    m_iCurrentPage = 0;
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
            FillMapList();

            RefreshComplete(eSuccess);
        }
        else
            RefreshComplete(eNoMapsReturned);
    }
}

void CBrowseMaps::FillMapList()
{
    CUtlVector<MapData*> vecMaps;
    g_pMapCache->GetMapList(vecMaps, MAP_LIST_BROWSE);

    FOR_EACH_VEC(vecMaps, i)
        AddMapToList(vecMaps[i]);

    ApplyFilters(GetFilters());
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBrowseMaps::PerformLayout()
{
    if (!m_bOfflineMode && m_bRequireUpdate && MapSelectorDialog().IsVisible())
    {
        PostMessage(this, new KeyValues("GetNewServerList"), 0.1f);
        m_bRequireUpdate = false;
    }

    if (m_bOfflineMode)
    {
        m_pMapList->SetEmptyListText("#ServerBrowser_OfflineMode");
    }

    BaseClass::PerformLayout();
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh if needed
//-----------------------------------------------------------------------------
void CBrowseMaps::OnPageShow()
{
    GetNewMapList();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBrowseMaps::GetNewMapList()
{
    UpdateStatus();

    m_bRequireUpdate = false;

    m_iCurrentPage = 0;
    FillMapList();

    if (SteamHTTP())
    {
        g_pAPIRequests->GetMaps(GetFilters(), UtlMakeDelegate(this, &CBrowseMaps::MapsQueryCallback));
    }
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