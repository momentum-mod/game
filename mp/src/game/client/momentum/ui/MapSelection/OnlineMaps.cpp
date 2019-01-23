#include "cbase.h"

#include "OnlineMaps.h"
#include "MapSelectorDialog.h"
#include "CMapListPanel.h"
#include "MapContextMenu.h"

#include "util/mom_util.h"
#include "mom_api_requests.h"

#include "mom_map_cache.h"

#include <OfflineMode.h>
#include "fmtstr.h"

#include "tier0/memdbgon.h"


using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//			NOTE:	m_Servers can not use more than 96 sockets, else it will
//					cause internet explorer to Stop working under win98 SE!
//-----------------------------------------------------------------------------
COnlineMaps::COnlineMaps(vgui::Panel *parent, const char *panelName) : CBaseMapsPage(parent, panelName)
{
    m_bRequireUpdate = true;
    m_bOfflineMode = !IsSteamGameServerBrowsingEnabled();
    m_iCurrentPage = 0;
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
COnlineMaps::~COnlineMaps()
{
}


void COnlineMaps::MapsQueryCallback(KeyValues *pKvResponse)
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

            MapSelectorDialog().ApplyFiltersToCurrentTab();
        }
        else
            RefreshComplete(eNoMapsReturned);
    }
}

void COnlineMaps::FillMapList()
{
    CUtlVector<MapData*> vecMaps;
    g_pMapCache->GetMapList(vecMaps, MAP_LIST_BROWSE);

    FOR_EACH_VEC(vecMaps, i)
        AddMapToList(vecMaps[i]);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COnlineMaps::PerformLayout()
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
void COnlineMaps::OnPageShow()
{
    GetNewMapList();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COnlineMaps::GetNewMapList()
{
    UpdateStatus();

    m_bRequireUpdate = false;

    m_iCurrentPage = 0;
    FillMapList();

    StartRefresh();
}

void COnlineMaps::RefreshComplete(EMapQueryOutputs eResponse)
{
    SetRefreshing(false);
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

void COnlineMaps::StartRefresh()
{
    if (SteamHTTP())
    {
        SetRefreshing(true);
        g_pAPIRequests->GetMaps(GetFilters(), UtlMakeDelegate(this, &COnlineMaps::MapsQueryCallback));
    }
}


//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void COnlineMaps::OnOpenContextMenu(int itemID)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu(m_pMapList);
    menu->ShowMenu(this, true, true);
}