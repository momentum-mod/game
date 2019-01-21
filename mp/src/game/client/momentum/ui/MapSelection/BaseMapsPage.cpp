#include "cbase.h"

#include "BaseMapsPage.h"
#include "CMapListPanel.h"
#include "MapSelectorDialog.h"
#include "MapFilterPanel.h"
#include "mom_map_cache.h"
#include "util/mom_util.h"

#include "fmtstr.h"

#include "vgui/ILocalize.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImageList.h"
#include "vgui_controls/ComboBox.h"

#include "tier0/memdbgon.h"

using namespace vgui; 

#undef wcscat

//Sort functions
static int __cdecl MapNameSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *string1 = item1.kv->GetString(KEYNAME_MAP_NAME);
    const char *string2 = item2.kv->GetString(KEYNAME_MAP_NAME);
    return Q_stricmp(string1, string2);
}

static int __cdecl MapCompletedSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *string1 = item1.kv->GetString(KEYNAME_MAP_BEST_TIME);
    const char *string2 = item2.kv->GetString(KEYNAME_MAP_BEST_TIME);
    return Q_stricmp(string1, string2);
}

static int __cdecl MapLayoutSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *i1 = item1.kv->GetString(KEYNAME_MAP_LAYOUT);
    const char *i2 = item2.kv->GetString(KEYNAME_MAP_LAYOUT);
    return Q_stricmp(i1, i2);
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBaseMapsPage::CBaseMapsPage(vgui::Panel *parent, const char *name) : PropertyPage(parent, name)
{
    SetSize(664, 294);
    
    m_iOnlineMapsCount = 0;

    m_hFont = NULL;

    // Init UI
    m_pMapList = new CMapListPanel(this, "MapList");
    m_pMapList->SetAllowUserModificationOfColumns(true);
    
    // Add the column headers
    m_pMapList->AddColumnHeader(HEADER_MAP_IMAGE, KEYNAME_MAP_IMAGE, "", GetScaledVal(120), ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_IMAGE_SIZETOFIT);
    m_pMapList->AddColumnHeader(HEADER_MAP_NAME, KEYNAME_MAP_NAME, "#MOM_MapSelector_Maps", GetScaledVal(150), GetScaledVal(150), 9001, ListPanel::COLUMN_RESIZEWITHWINDOW | ListPanel::COLUMN_UNHIDABLE);
    m_pMapList->AddColumnHeader(HEADER_MAP_LAYOUT, KEYNAME_MAP_LAYOUT, "#MOM_MapSelector_MapLayout", GetScaledVal(75), GetScaledVal(75), GetScaledVal(100), ListPanel::COLUMN_RESIZEWITHWINDOW);
    m_pMapList->AddColumnHeader(HEADER_DIFFICULTY, KEYNAME_MAP_DIFFICULTY, "#MOM_MapSelector_Difficulty", GetScaledVal(55), GetScaledVal(55), GetScaledVal(100), 0);
    m_pMapList->AddColumnHeader(HEADER_BESTTIME, KEYNAME_MAP_BEST_TIME, "#MOM_MapSelector_BestTime", GetScaledVal(90), GetScaledVal(90), 9001, ListPanel::COLUMN_RESIZEWITHWINDOW);
    
    //Tooltips
    //MOM_TODO: do we want tooltips?
    
    // Alignment
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_MAP_LAYOUT, Label::a_center);
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_DIFFICULTY, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_LAYOUT, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_DIFFICULTY, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_IMAGE, Label::a_center);

    // Sort Functions
    m_pMapList->SetSortFunc(HEADER_MAP_NAME, MapNameSortFunc);  
    m_pMapList->SetSortFunc(HEADER_BESTTIME, MapCompletedSortFunc);
    m_pMapList->SetSortFunc(HEADER_MAP_LAYOUT, MapLayoutSortFunc);

    // disable sort for certain columns
    m_pMapList->SetColumnSortable(HEADER_MAP_IMAGE, false);

    // Sort by map name by default
    m_pMapList->SetSortColumn(HEADER_MAP_NAME);

    m_bAutoSelectFirstItemInGameList = false;

    LoadControlSettings(CFmtStr("resource/ui/MapSelector/%sPage.res", name));
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBaseMapsPage::~CBaseMapsPage()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseMapsPage::GetInvalidMapListID()
{
    return m_pMapList->InvalidItemID();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::PerformLayout()
{
    BaseClass::PerformLayout();

    /*if (m_pMapList->GetSelectedItemsCount() < 1)
    {
        m_pStartMap->SetEnabled(false);
    }
    else
    {
        m_pStartMap->SetEnabled(true);
    }


    if (SupportsItem(IMapList::GETNEWLIST))
    {
        m_pQueryMapsQuick->SetVisible(true);
        m_pQueryMaps->SetText("#ServerBrowser_RefreshAll");
    }
    else
    {
        m_pQueryMapsQuick->SetVisible(false);
        m_pQueryMaps->SetVisible(false);//Because local maps won't be searching
        //m_pRefreshAll->SetText("#ServerBrowser_Refresh");
    }

    if (IsRefreshing())
    {
        m_pQueryMaps->SetText("#ServerBrowser_StopRefreshingList");
    }

    if (m_pMapList->GetItemCount() > 0)
    {
        m_pQueryMapsQuick->SetEnabled(true);
    }
    else
    {
        m_pQueryMapsQuick->SetEnabled(false);
    }
    m_pMapList->SetEmptyListText("#MOM_MapSelector_NoMaps");
#ifndef NO_STEAM
    //if (!SteamMatchmakingServers() || !SteamMatchmaking())
    {
        m_pQueryMapsQuick->SetEnabled(false);
        m_pStartMap->SetEnabled(false);
        m_pQueryMaps->SetEnabled(false);
        //m_pGameList->SetEmptyListText("#ServerBrowser_SteamRunning");
    }
#endif*/
    Repaint();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    // OnButtonToggled(m_pFilter, false);

    // Images
    ImageList *imageList = new ImageList(false);
    imageList->AddImage(scheme()->GetImage("maps/invalid_map", false)); // The ? banner at index 1
    m_pMapList->SetImageList(imageList, true);

    //Font
    m_hFont = pScheme->GetFont("MapListFont", IsProportional());
    if (!m_hFont)
        m_hFont = pScheme->GetFont("DefaultSmall", IsProportional());
    m_pMapList->SetFont(m_hFont);
}


//-----------------------------------------------------------------------------
// Purpose: loads filter settings (from disk) from the keyvalues
//-----------------------------------------------------------------------------
void CBaseMapsPage::LoadFilters()
{
    MapSelectorDialog().LoadTabFilterData(GetName());
}

//-----------------------------------------------------------------------------
// Purpose: applies only the game filter to the current list
//-----------------------------------------------------------------------------
void CBaseMapsPage::ApplyFilters(MapFilterPanel *pFilters)
{
    if (!IsVisible())
        return;

    // loop through all the maps checking filters
    FOR_EACH_VEC(m_vecMaps, i)
    {
        mapdisplay_t *map = &m_vecMaps[i];
        MapData* mapinfo = map->m_pMap;
        if (!pFilters->MapPassesFilters(mapinfo))
        {
            //Failed filters, remove the map
            map->m_bDoNotRefresh = true;
            if (m_pMapList->IsValidItemID(map->m_iListID))
            {
                m_pMapList->SetItemVisible(map->m_iListID, false);
            }
        }
        else if (BShowMap(*map))
        {
            map->m_bDoNotRefresh = false;
            if (!m_pMapList->IsValidItemID(map->m_iListID))
            {
                //DevLog("ADDING MAP TO LIST! %s\n ", mapinfo->m_szMapName);
                KeyValuesAD kv("Map");
                kv->SetString(KEYNAME_MAP_NAME, mapinfo->m_szMapName);
                kv->SetString("map", mapinfo->m_szMapName);//I think this is needed somewhere
                // kv->SetInt(KEYNAME_MAP_GAME_MODE, mapinfo->m_iGameMode);
                kv->SetInt(KEYNAME_MAP_DIFFICULTY, mapinfo->m_Info.m_iDifficulty);
                kv->SetString(KEYNAME_MAP_LAYOUT, mapinfo->m_Info.m_bIsLinear ? "LINEAR" : "STAGED");
                if (mapinfo->m_Rank.m_bValid)
                {
                    char szBestTime[BUFSIZETIME];
                    g_pMomentumUtil->FormatTime(mapinfo->m_Rank.m_Run.m_fTime, szBestTime);

                    kv->SetString(KEYNAME_MAP_BEST_TIME, szBestTime);
                }
                else
                {
                    kv->SetString(KEYNAME_MAP_BEST_TIME, "#MOM_NotApplicable");
                }

                kv->SetInt(KEYNAME_MAP_IMAGE, map->m_iMapImageIndex);

                map->m_iListID = m_pMapList->AddItem(kv, NULL, false, false);
            }
            // make sure the map is visible
            m_pMapList->SetItemVisible(map->m_iListID, true);
        }
    }

    UpdateStatus();
    m_pMapList->SortList();
    InvalidateLayout();
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Resets UI map count
//-----------------------------------------------------------------------------
void CBaseMapsPage::UpdateStatus()
{
    if (m_pMapList->GetItemCount() > 1)
    {
        m_pMapList->SetColumnHeaderText(HEADER_MAP_NAME, CConstructLocalizedString(g_pVGuiLocalize->Find("#MOM_MapSelector_MapCount"), m_pMapList->GetItemCount()));
    }
    else
    {
        m_pMapList->SetColumnHeaderText(HEADER_MAP_NAME, g_pVGuiLocalize->Find("#MOM_MapSelector_Maps"));
        m_pMapList->SetEmptyListText("#MOM_MapSelector_NoMaps");
    }
}

//-----------------------------------------------------------------------------
// Purpose: call to let the UI now whether the game list is currently refreshing
// MOM_TODO: Use this method for OnlineMaps
//-----------------------------------------------------------------------------
void CBaseMapsPage::SetRefreshing(bool state)
{
    //MOM_TODO: The OnlineMaps tab will have its own "search" using the filter
    // that is the same filter as the one in LocalMaps, asking the API (using filter data)
    // and populating the list with the map data from API.

    /*if (state)
    {
        MapSelectorDialog().UpdateStatusText("#MOM_MapSelector_SearchingForMaps");

        // clear message in panel
        m_pMapList->SetEmptyListText("");
    }
    else
    {
        MapSelectorDialog().UpdateStatusText(nullptr);
        if (SupportsItem(IMapList::GETNEWLIST))
        {
            m_pQueryMaps->SetText("#MOM_MapSelector_Search");
        }
        else
        {
            //MOM_TODO: hide the button? This else branch is specifically the LocalMaps one
            m_pQueryMaps->SetVisible(false);
            //m_pRefreshAll->SetText("#ServerBrowser_Refresh");
        }
        m_pQueryMaps->SetCommand("GetNewList");

        // 'refresh quick' button is only enabled if there are servers in the list
        if (m_pMapList->GetItemCount() > 0)
        {
            m_pQueryMapsQuick->SetEnabled(true);
        }
        else
        {
            m_pQueryMapsQuick->SetEnabled(false);
        }
    }*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnCommand(const char *command)
{
    if (!Q_stricmp(command, "StartMap"))
    {
        OnMapStart();
    }
    else if (!Q_stricmp(command, "stoprefresh"))
    {
        // cancel the existing refresh
        StopRefresh();
    }
    else if (!Q_stricmp(command, "refresh"))
    {
#ifndef NO_STEAM
        //if (SteamMatchmakingServers())
        //    SteamMatchmakingServers()->RefreshQuery(m_eMatchMakingType);
        SetRefreshing(true);
        m_iOnlineMapsCount = 0;
#endif
    }
    else if (!Q_stricmp(command, "GetNewList"))
    {
        GetNewMapList();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

//-----------------------------------------------------------------------------
// Purpose: called when a row gets selected in the list
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnItemSelected()
{
    
}

//-----------------------------------------------------------------------------
// Purpose: refreshes server list on F5
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnKeyCodePressed(vgui::KeyCode code)
{
    if (code == KEY_F5)
    {
        StartRefresh();
    }
    else
    {
        BaseClass::OnKeyCodePressed(code);
    }
}


//-----------------------------------------------------------------------------
// Purpose: Handle enter pressed in the games list page. Return true
// to intercept the message instead of passing it on through vgui.
//-----------------------------------------------------------------------------
bool CBaseMapsPage::OnGameListEnterPressed()
{
    return false;
}


//-----------------------------------------------------------------------------
// Purpose: Get the # items selected in the game list.
//-----------------------------------------------------------------------------
int CBaseMapsPage::GetSelectedItemsCount()
{
    return m_pMapList->GetSelectedItemsCount();
}


//-----------------------------------------------------------------------------
// Purpose: removes the server from the UI list
//-----------------------------------------------------------------------------
void CBaseMapsPage::RemoveMap(mapdisplay_t &map)
{
    if (m_pMapList->IsValidItemID(map.m_iListID))
    {
        // don't remove the server from list, just hide since this is a lot faster
        m_pMapList->SetItemVisible(map.m_iListID, false);

        // find the row in the list and kill
        //	m_pGameList->RemoveItem(server.listEntryID);
        //	server.listEntryID = GetInvalidServerListID();
    }

    UpdateStatus();
}


//-----------------------------------------------------------------------------
// Purpose: refreshes a single server
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnRefreshServer(int serverID)
{
    //MOM_TODO: for the mp/ port?
    /*
#ifndef NO_STEAM
    if (!SteamMatchmakingServers())
    return;

    // walk the list of selected servers refreshing them
    for (int i = 0; i < m_pGameList->GetSelectedItemsCount(); i++)
    {
    int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(i));

    // refresh this server
    SteamMatchmakingServers()->RefreshServer(m_eMatchMakingType, serverID);
    }

    SetRefreshing(IsRefreshing());
    #endif
    */
}


//-----------------------------------------------------------------------------
// Purpose: starts the servers refreshing
//-----------------------------------------------------------------------------
void CBaseMapsPage::StartRefresh()
{
    ClearMapList();
    SetRefreshing(true);
    
    m_iOnlineMapsCount = 0;
}


//-----------------------------------------------------------------------------
// Purpose: Remove all the maps we currently have
//-----------------------------------------------------------------------------
void CBaseMapsPage::ClearMapList()
{
    m_vecMaps.RemoveAll();
    m_pMapList->RemoveAll();
}


//-----------------------------------------------------------------------------
// Purpose: get a new list of maps from the backend
//-----------------------------------------------------------------------------
void CBaseMapsPage::GetNewMapList()
{
    StartRefresh();
}


//-----------------------------------------------------------------------------
// Purpose: stops current refresh/GetNewMapList()
//-----------------------------------------------------------------------------
void CBaseMapsPage::StopRefresh()
{
    // clear update states
    m_iOnlineMapsCount = 0;
#ifndef NO_STEAM
    // Stop the server list refreshing
    //MOM_TODO: implement the following by HTTP request for online maps?
    // SteamHTTP()->ReleaseHTTPRequest()

    //if (SteamMatchmakingServers())
    //    SteamMatchmakingServers()->CancelQuery(m_eMatchMakingType);
#endif
    // update UI
    //RefreshComplete(eServerResponded);
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the list is currently searching for maps
//-----------------------------------------------------------------------------
bool CBaseMapsPage::IsRefreshing()
{
    //MOM_TODO: 
    // SteamUtils()->IsAPICallCompleted() on the HTTP request for the online
    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnPageShow()
{
    if (IsVisible())
        StartRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: Called on page hide, stops any refresh
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnPageHide()
{
    StopRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: initiates map loading
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnMapStart()
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    // get the map
    //MOM_TODO: get the mapstruct_t data instead of KVs here
    KeyValues *kv = m_pMapList->GetItem(m_pMapList->GetSelectedItem(0));
    // Stop the current search (online maps)
    StopRefresh();
    
    //MOM_TODO: set global gamemode, which sets tick settings etc
    //TickSet::SetTickrate(MOMGM_BHOP);
    //engine->ServerCmd(VarArgs("mom_gamemode %i", MOMGM_BHOP));//MOM_TODO this is testing, replace with m.m_iGamemode

    // Start the map
    // Originally this was kv->GetString("map") but did not work for Online maps. WHy it was maps and not what it's now?
    engine->ExecuteClientCmd(VarArgs("map %s\n", kv->GetString(KEYNAME_MAP_NAME)));
}

//-----------------------------------------------------------------------------
// Purpose: Displays the current map info (from contextmenu)
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnViewMapInfo()
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    // get the map
    KeyValues *pMap = m_pMapList->GetItem(m_pMapList->GetSelectedItem(0));

    //MOM_TODO: pass mapstruct_t data over to the map info dialog!
    //m_vecMaps.

    //This is how the ServerBrowser did it, they used UserData from the list,
    //and called the CUtlMap<int <---(ID), mapstruct_t> Get() method
    //int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(0));

    // Stop the current refresh
    StopRefresh();
    // View the map info
    MapSelectorDialog().OpenMapInfoDialog(this, pMap);
}