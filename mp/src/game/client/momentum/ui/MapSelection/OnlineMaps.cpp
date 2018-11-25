#include "pch_mapselection.h"
#include "util/jsontokv.h"

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

    LoadFilterSettings();
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
COnlineMaps::~COnlineMaps()
{
}


void COnlineMaps::MapsQueryCallback(HTTPRequestCompleted_t* pCallback, bool bIOFailure)
{
    if (bIOFailure)
    {
        RefreshComplete(eNoServerReturn);
        return;
    }

    if (pCallback->m_eStatusCode != k_EHTTPStatusCode200OK)
    {
        RefreshComplete(eServerBadReturn);
        return;
    }

    EMapQueryOutputs returnCode = eSuccess;
    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        Warning("%s - 0 body size!\n", __FUNCTION__);
        RefreshComplete(eNoServerReturn);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

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
            KeyValues *pRuns = pResponse->FindKey("maps");

            if (pRuns && !pRuns->IsEmpty())
            {
                FOR_EACH_SUBKEY(pRuns, pRun)
                {
                    mapdisplay_t map;
                    mapstruct_t m;
                    Q_strcpy(m.m_szMapName, pRun->GetString("map_name"));
                    m.m_bHasStages = pRun->GetBool("linear");
                    m.m_iDifficulty = pRun->GetInt("gamemode");
                    Q_strcpy(m.m_szBestTime, pRun->GetString("mapper_name"));
                    map.m_mMap = m;
                    KeyValues *kv = new KeyValues("map");;
 
                    kv->SetString(KEYNAME_MAP_NAME, m.m_szMapName);
                    kv->SetString(KEYNAME_MAP_LAYOUT, m.m_bHasStages ? "STAGED" : "LINEAR");
                    kv->SetInt(KEYNAME_MAP_DIFFICULTY, m.m_iDifficulty);
                    kv->SetString(KEYNAME_MAP_BEST_TIME, m.m_szBestTime);
                    kv->SetInt(KEYNAME_MAP_IMAGE, map.m_iMapImageIndex);
                    map.m_iListID = m_pMapList->AddItem(kv, 0, false, false);
                    m_vecMaps.AddToTail(map); 
                }
            }
            else
            {
                returnCode = eNoMapsReturned;
            }
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
        returnCode = eServerBadReturn;
    }

    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    RefreshComplete(returnCode);
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
        m_pStartMap->SetEnabled(false);
        m_pQueryMaps->SetEnabled(false);
        m_pQueryMapsQuick->SetEnabled(false);
        m_pFilter->SetEnabled(false);
    }

    BaseClass::PerformLayout();
    //m_pLocationFilter->SetEnabled(true);
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh if needed
//-----------------------------------------------------------------------------
void COnlineMaps::OnPageShow()
{
    GetNewMapList();
}


//-----------------------------------------------------------------------------
// Purpose: Called every frame, maintains sockets and runs refreshes
//-----------------------------------------------------------------------------
void COnlineMaps::OnTick()
{
    BaseClass::OnTick();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COnlineMaps::GetNewMapList()
{
    UpdateStatus();

    m_bRequireUpdate = false;

    m_pMapList->DeleteAllItems();
    m_vecMaps.RemoveAll();
    StartRefresh();
}

void COnlineMaps::RefreshComplete(EMapQueryOutputs eResponse)
{
    SetRefreshing(false);
    UpdateFilterSettings();
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


//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool COnlineMaps::SupportsItem(IMapList::InterfaceItem_e item)
{
    switch (item)
    {
        case FILTERS:
        case GETNEWLIST:
            return true;

        default:
            return false;
    }
}


void COnlineMaps::StartRefresh()
{
    if (steamapicontext && steamapicontext->SteamHTTP())
    {
        SetRefreshing(true);
        ClearMapList();
        char szUrl[BUFSIZ];
        Q_snprintf(szUrl, BUFSIZ, "%s/getmaps/1", MOM_APIDOMAIN);
        g_pMomentumUtil->CreateAndSendHTTPReq(szUrl, &cbMapsQuery, &COnlineMaps::MapsQueryCallback, this);
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


//-----------------------------------------------------------------------------
// Purpose: refreshes a single server
//-----------------------------------------------------------------------------
void COnlineMaps::OnRefreshServer(int serverID)
{
    BaseClass::OnRefreshServer(serverID);

    MapSelectorDialog().UpdateStatusText("#ServerBrowser_GettingNewServerList");
}
