#include "pch_mapselection.h"

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
CBaseMapsPage::CBaseMapsPage(vgui::Panel *parent, const char *name, const char *pCustomResFilename)
    : PropertyPage(parent, name), m_pCustomResFilename(pCustomResFilename)
{
    SetSize(664, 294);
    
    m_iGameModeFilter = 0;
    m_szMapFilter[0] = 0;
    m_iDifficultyFilter = 0;
    m_bFilterHideCompleted = false;
    m_iMapLayoutFilter = 0;
    m_iOnlineMapsCount = 0;

    m_hFont = NULL;

    // get the 'all' text
    wchar_t *all = g_pVGuiLocalize->Find("#MOM_MapSelector_All");
    if (all)
        Q_UnicodeToUTF8(all, m_szComboAllText, sizeof(m_szComboAllText));

    // Init UI
    m_pStartMap = new Button(this, "StartMapButton", "#MOM_MapSelector_StartMap");
    m_pStartMap->SetEnabled(false);
    m_pQueryMaps = new Button(this, "RefreshButton", "#ServerBrowser_Refresh");//Needed for online maps
    m_pQueryMapsQuick = new Button(this, "RefreshQuickButton", "#ServerBrowser_RefreshQuick");//Needed for online maps
    m_pMapList = new CMapListPanel(this, "MapList");
    m_pMapList->SetAllowUserModificationOfColumns(true);
    
    // Add the column headers
    m_pMapList->AddColumnHeader(HEADER_MAP_IMAGE, KEYNAME_MAP_IMAGE, "", 140, ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_IMAGE);
    m_pMapList->AddColumnHeader(HEADER_MAP_NAME, KEYNAME_MAP_NAME, "#MOM_MapSelector_Maps", 150, 150, 9001, ListPanel::COLUMN_RESIZEWITHWINDOW | ListPanel::COLUMN_UNHIDABLE);
    m_pMapList->AddColumnHeader(HEADER_MAP_LAYOUT, KEYNAME_MAP_LAYOUT, "#MOM_MapSelector_MapLayout", 75, 75, 100, ListPanel::COLUMN_RESIZEWITHWINDOW);
    m_pMapList->AddColumnHeader(HEADER_DIFFICULTY, KEYNAME_MAP_DIFFICULTY, "#MOM_MapSelector_Difficulty", 55, 55, 100, 0);
    m_pMapList->AddColumnHeader(HEADER_BESTTIME, KEYNAME_MAP_BEST_TIME, "#MOM_MapSelector_BestTime", 90, 90, 9001, ListPanel::COLUMN_RESIZEWITHWINDOW);
    
    //Tooltips
    //MOM_TODO: do we want tooltips?
    
    // Alignment
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

    CreateFilters();
    LoadFilterSettings();

    m_bAutoSelectFirstItemInGameList = false;
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

    if (m_pMapList->GetSelectedItemsCount() < 1)
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
#endif
    Repaint();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    OnButtonToggled(m_pFilter, false);

    // Images
    ImageList *imageList = new ImageList(false);
    //MOM_TODO: Load custom images for the map selector
    imageList->AddImage(scheme()->GetImage("maps/invalid_map", false)); // The ? banner at index 1
    m_pMapList->SetImageList(imageList, true);

    //Font
    m_hFont = pScheme->GetFont("MapListFont", IsProportional());
    if (!m_hFont)
        m_hFont = pScheme->GetFont("DefaultSmall", IsProportional());
    m_pMapList->SetFont(m_hFont);
}

//-----------------------------------------------------------------------------
// Purpose: gets information about specified map
//-----------------------------------------------------------------------------
mapstruct_t *CBaseMapsPage::GetMap(unsigned int serverID)
{
    //MOM_TODO: This may not be needed
    /*
#ifndef NO_STEAM
    if (!SteamMatchmakingServers())
    return NULL;

    if (serverID >= 0)
    {
    return SteamMatchmakingServers()->GetServerDetails(m_eMatchMakingType, serverID);
    }
    else
    {
    Assert(!"Unable to return a useful entry");
    return NULL; // bugbug Alfred: temp Favorites/History objects won't return a good value here...
    }
    #else*/
    return nullptr;
    //#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::CreateFilters()
{
    m_bFiltersVisible = false;
    m_pFilter = new ToggleButton(this, "Filter", "#MOM_MapSelector_Filter");
    m_pFilterString = new Label(this, "FilterString", "");

    // filter controls
    m_pGameModeFilter = new ComboBox(this, "GameModeFilter", 3, false);//"Game mode"
    m_pGameModeFilter->AddItem("#MOM_MapSelector_All", nullptr);//All
    m_pGameModeFilter->AddItem("#MOM_MapSelector_SurfOnly", nullptr);//Surf only
    m_pGameModeFilter->AddItem("#MOM_MapSelector_BhopOnly", nullptr);//Bhop only
    m_pGameModeFilter->AddActionSignalTarget(this);
    //MOM_TODO: add extra game mode filter types?


    m_pMapFilter = new TextEntry(this, "MapFilter");//As-is, people can search by map name
    m_pMapFilter->AddActionSignalTarget(this);

    //Difficulty filter
    m_pDifficultyFilter = new ComboBox(this, "DifficultyFilter", 6, false);
    m_pDifficultyFilter->AddItem("#MOM_MapSelector_All", nullptr);
    m_pDifficultyFilter->AddItem("#MOM_MapSelector_LessThanDiff2", nullptr);//"Less than Tier 2"
    m_pDifficultyFilter->AddItem("#MOM_MapSelector_LessThanDiff3", nullptr);//"Less than Tier 3" 
    m_pDifficultyFilter->AddItem("#MOM_MapSelector_LessThanDiff4", nullptr);//etc
    m_pDifficultyFilter->AddItem("#MOM_MapSelector_LessThanDiff5", nullptr);//etc
    m_pDifficultyFilter->AddItem("#MOM_MapSelector_LessThanDiff6", nullptr);//MOM_TODO: Is "tier 6" difficulty the highest?
    m_pDifficultyFilter->AddActionSignalTarget(this);

    //Hide completed maps
    m_pHideCompletedFilterCheck = new CheckButton(this, "HideCompletedFilterCheck", ""); //for Maps the player has not completed
    m_pHideCompletedFilterCheck->AddActionSignalTarget(this);

    //Filter staged/linear
    m_pMapLayoutFilter = new ComboBox(this, "MapLayoutFilter", 3, false);
    m_pMapLayoutFilter->AddItem("#MOM_MapSelector_All", nullptr);
    m_pMapLayoutFilter->AddItem("#MOM_MapSelector_StagedOnly", nullptr);
    m_pMapLayoutFilter->AddItem("#MOM_MapSelector_LinearOnly", nullptr);
    m_pMapLayoutFilter->AddActionSignalTarget(this);
}



//-----------------------------------------------------------------------------
// Purpose: loads filter settings (from disk) from the keyvalues
//-----------------------------------------------------------------------------
void CBaseMapsPage::LoadFilterSettings()
{
    KeyValues *filter = MapSelectorDialog().GetFilterSaveData(GetName());

    //Game-mode selection
    m_iGameModeFilter = filter->GetInt("gamemode", 0);
    m_pGameModeFilter->ActivateItemByRow(m_iGameModeFilter);

    //"Map"
    Q_strncpy(m_szMapFilter, filter->GetString("map"), sizeof(m_szMapFilter));
    m_pMapFilter->SetText(m_szMapFilter);

    //Map layout
    m_iMapLayoutFilter = filter->GetInt("maplayout", 0);
    m_pMapLayoutFilter->ActivateItemByRow(m_iMapLayoutFilter);

    //HideCompleted maps
    m_bFilterHideCompleted = filter->GetBool("HideCompleted", false);
    m_pHideCompletedFilterCheck->SetSelected(m_bFilterHideCompleted);

    //Difficulty
    m_iDifficultyFilter = filter->GetInt("difficulty");
    if (m_iDifficultyFilter)
    {
        char buf[32];
        Q_snprintf(buf, sizeof(buf), "< %d", m_iDifficultyFilter);
        m_pDifficultyFilter->SetText(buf);
    }

    // apply to the controls
    OnLoadFilter(filter);
    UpdateFilterSettings();
    ApplyGameFilters();
}

//-----------------------------------------------------------------------------
// Purpose: Handles incoming server refresh data
//			updates the server browser with the refreshed information from the server itself
//-----------------------------------------------------------------------------
/*void CBaseMapsPage::ServerResponded(gameserveritem_t &server)
{
int nIndex = -1; // start at -1 and work backwards to find the next free slot for this adhoc query
while (m_mapServers.Find(nIndex) != m_mapServers.InvalidIndex())
nIndex--;
ServerResponded(nIndex, &server);
}*/


//-----------------------------------------------------------------------------
// Purpose: Callback for ISteamMatchmakingServerListResponse
//-----------------------------------------------------------------------------
/*void CBaseMapsPage::ServerResponded(int iServer)
{
/*
#ifndef NO_STEAM
gameserveritem_t *pServerItem = SteamMatchmakingServers()->GetServerDetails(m_eMatchMakingType, iServer);
if (!pServerItem)
{
Assert(!"Missing server response");
return;
}
ServerResponded(iServer, pServerItem);
#endif
}*/


//-----------------------------------------------------------------------------
// Purpose: Handles incoming server refresh data
//			updates the server browser with the refreshed information from the server itself
//-----------------------------------------------------------------------------
//MOM_TODO: Use custom Steam HTML callbacks to handle data to parse from API


//*******

//THIS WILL BE USED FOR ONLINE!! DO NOT REMOVE!!!!!!!!!!

//********
/*void CBaseMapsPage::ServerResponded(int iServer, gameserveritem_t *pServerItem)
{
int iServerMap = m_mapServers.Find(iServer);
if (iServerMap == m_mapServers.InvalidIndex())
{
netadr_t netAdr(pServerItem->m_NetAdr.GetIP(), pServerItem->m_NetAdr.GetQueryPort());
int iServerIP = m_mapServerIP.Find(netAdr);
if (iServerIP != m_mapServerIP.InvalidIndex())
{
// if we already had this entry under another index remove the old entry
int iServerMap = m_mapServers.Find(m_mapServerIP[iServerIP]);
if (iServerMap != m_mapServers.InvalidIndex())
{
serverdisplay_t &server = m_mapServers[iServerMap];
if (m_pGameList->IsValidItemID(server.m_iListID))
m_pGameList->RemoveItem(server.m_iListID);
m_mapServers.RemoveAt(iServerMap);
}
m_mapServerIP.RemoveAt(iServerIP);
}

serverdisplay_t serverFind;
serverFind.m_iListID = -1;
serverFind.m_bDoNotRefresh = false;
iServerMap = m_mapServers.Insert(iServer, serverFind); // MOM_TODO : ADDS TO
m_mapServerIP.Insert(netAdr, iServer);
}

serverdisplay_t *pServer = &m_mapServers[iServerMap];
pServer->m_iServerID = iServer;
Assert(pServerItem->m_NetAdr.GetIP() != 0);

// check filters
bool removeItem = false;
if (!CheckPrimaryFilters(*pServerItem))
{
// server has been filtered at a primary level
// remove from lists
pServer->m_bDoNotRefresh = true;

// remove from UI list
removeItem = true;
}
else if (!CheckSecondaryFilters(*pServerItem))
{
// we still ping this server in the future; however it is removed from UI list
removeItem = true;
}

if (removeItem)
{
if (m_pGameList->IsValidItemID(pServer->m_iListID))
{
m_pGameList->RemoveItem(pServer->m_iListID);
pServer->m_iListID = GetInvalidServerListID();
}
return;
}

// update UI
KeyValues *kv;
if (m_pGameList->IsValidItemID(pServer->m_iListID))
{
// we're updating an existing entry
kv = m_pGameList->GetItem(pServer->m_iListID);
m_pGameList->SetUserData(pServer->m_iListID, pServer->m_iServerID);
}
else
{
// new entry
kv = new KeyValues("Server");
}

kv->SetString("name", pServerItem->GetName());
kv->SetString("map", pServerItem->m_szMap);
kv->SetString("GameDir", pServerItem->m_szGameDir);
kv->SetString("GameDesc", pServerItem->m_szGameDescription);
kv->SetInt("password", pServerItem->m_bPassword ? 1 : 0);

if (pServerItem->m_nBotPlayers > 0)
kv->SetInt("bots", pServerItem->m_nBotPlayers);
else
kv->SetString("bots", "");

//if (pServerItem->m_bSecure)
//{
// show the denied icon if banned from secure servers, the secure icon otherwise
//    kv->SetInt("secure", ServerBrowser().IsVACBannedFromGame(pServerItem->m_nAppID) ? 4 : 3);
//}
//else
{
kv->SetInt("secure", 0);
}

kv->SetString("IPAddr", pServerItem->m_NetAdr.GetConnectionAddressString());

int nAdjustedForBotsPlayers = max(0, pServerItem->m_nPlayers - pServerItem->m_nBotPlayers);
int nAdjustedForBotsMaxPlayers = max(0, pServerItem->m_nMaxPlayers - pServerItem->m_nBotPlayers);

char buf[32];
Q_snprintf(buf, sizeof(buf), "%d / %d", nAdjustedForBotsPlayers, nAdjustedForBotsMaxPlayers);
kv->SetString("Players", buf);

kv->SetInt("Ping", pServerItem->m_nPing);

kv->SetString("Tags", pServerItem->m_szGameTags);

if (pServerItem->m_ulTimeLastPlayed)
{
// construct a time string for last played time
struct tm *now;
now = localtime((time_t*) &pServerItem->m_ulTimeLastPlayed);

if (now)
{
char buf[64];
strftime(buf, sizeof(buf), "%a %d %b %I:%M%p", now);
Q_strlower(buf + strlen(buf) - 4);
kv->SetString("LastPlayed", buf);
}
}

if (pServer->m_bDoNotRefresh)
{
// clear out the vars
kv->SetString("Ping", "");
kv->SetWString("GameDesc", g_pVGuiLocalize->Find("#ServerBrowser_NotResponding"));
kv->SetString("Players", "");
kv->SetString("map", "");
}

if (!m_pGameList->IsValidItemID(pServer->m_iListID))
{
// new server, add to list
pServer->m_iListID = m_pGameList->AddItem(kv, pServer->m_iServerID, false, false);
if (m_bAutoSelectFirstItemInGameList && m_pGameList->GetItemCount() == 1)
{
m_pGameList->AddSelectedItem(pServer->m_iListID);
}

kv->deleteThis();
}
else
{
// tell the list that we've changed the data
m_pGameList->ApplyItemChanges(pServer->m_iListID);
m_pGameList->SetItemVisible(pServer->m_iListID, true);
}

UpdateStatus();
m_iServerRefreshCount++;
}*/

//-----------------------------------------------------------------------------
// Purpose: Handles filter dropdown being toggled
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnButtonToggled(Panel *panel, int state)
{
    if (panel == m_pFilter)
    {
        int wide, tall;
        GetSize(wide, tall);
        SetSize(624, 278);
        if (m_pCustomResFilename)//MOM_TODO: this will never happen, consider removing?
        {
            m_bFiltersVisible = false;
        }
        else
        {
            if (m_pFilter->IsSelected())
            {
                // drop down
                m_bFiltersVisible = true;
            }
            else
            {
                // hide filter area
                m_bFiltersVisible = false;
            }
        }

        UpdateDerivedLayouts();
        m_pFilter->SetSelected(m_bFiltersVisible);

        if (m_hFont)
        {
            SETUP_PANEL(m_pMapList);
            m_pMapList->SetFont(m_hFont);
        }

        SetSize(wide, tall);

        InvalidateLayout();
    }
    else if (panel == m_pHideCompletedFilterCheck)
    {
        // treat changing these buttons like any other filter has changed
        OnTextChanged(panel, "");
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::UpdateDerivedLayouts(void)
{
    char rgchControlSettings[MAX_PATH];
    if (m_pCustomResFilename)//MOM_TODO: this will never be custom, look into removing this?
    {
        Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "%s", m_pCustomResFilename);
    }
    else
    {
        if (m_pFilter->IsSelected())
        {
            // drop down
            Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "resource/ui/%sPage_Filters.res", GetName());
        }
        else
        {
            // hide filter area
            Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "resource/ui/%sPage.res", GetName());
        }
    }

    LoadControlSettings(rgchControlSettings);
}

//-----------------------------------------------------------------------------
// Purpose: Called when the game dir combo box is changed
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnTextChanged(Panel *panel, const char *text)
{
    if (!Q_stricmp(text, m_szComboAllText))
    {
        ComboBox *box = dynamic_cast<ComboBox *>(panel);
        if (box)
        {
            box->SetText("");
            text = "";
        }
    }

    // get filter settings from controls
    UpdateFilterSettings();
    // apply settings
    ApplyGameFilters();

    if (m_bFiltersVisible && (panel == m_pGameModeFilter || panel == m_pDifficultyFilter || panel == m_pMapFilter
         || panel == m_pMapLayoutFilter))
    {
        // if they changed filter settings then cancel the refresh because the old list they are getting
        // will be for the wrong map or gametype, so stop and start a refresh
        StopRefresh();
        //If the filters have changed, we'll want to send a new request with the new data
        //MOM_TODO: uncomment this: StartRefresh()
    }
}

//-----------------------------------------------------------------------------
// Purpose: applies only the game filter to the current list
//-----------------------------------------------------------------------------
void CBaseMapsPage::ApplyGameFilters()
{
    // loop through all the maps checking filters
    FOR_EACH_VEC(m_vecMaps, i)
    {
        mapdisplay_t &map = m_vecMaps[i];
        mapstruct_t* mapinfo = &map.m_mMap;
        //DevLog("CURRENTLY FILTERING %s\n", mapinfo->m_szMapName);
        if (!CheckPrimaryFilters(*mapinfo) || !CheckSecondaryFilters(*mapinfo))//MOM_TODO: change this to just one filter check?
        {
            //Failed filters, remove the map
            map.m_bDoNotRefresh = true;
            if (m_pMapList->IsValidItemID(map.m_iListID))
            {
                m_pMapList->SetItemVisible(map.m_iListID, false);
            }
        }
        else if (BShowMap(map))
        {
            map.m_bDoNotRefresh = false;
            if (!m_pMapList->IsValidItemID(map.m_iListID))
            {
                //DevLog("ADDING MAP TO LIST! %s\n ", mapinfo->m_szMapName);
                KeyValues *kv = new KeyValues("Map");
                kv->SetString(KEYNAME_MAP_NAME, mapinfo->m_szMapName);
                kv->SetString("map", mapinfo->m_szMapName);//I think this is needed somewhere
                //kv->SetInt(KEYNAME_MAP_GAME_MODE, mapinfo->m_iGameMode);
                kv->SetInt(KEYNAME_MAP_DIFFICULTY, mapinfo->m_iDifficulty);
                kv->SetString(KEYNAME_MAP_LAYOUT, mapinfo->m_bHasStages ? "STAGED" : "LINEAR");
                //kv->SetBool(KEYNAME_MAP_COMPLETED, mapinfo->m_bCompleted);
                kv->SetString(KEYNAME_MAP_BEST_TIME, mapinfo->m_szBestTime);
                //MOM_TODO: Recalculate the image index just in case (only if it's 0)
                kv->SetInt(KEYNAME_MAP_IMAGE, map.m_iMapImageIndex);

                map.m_iListID = m_pMapList->AddItem(kv, NULL, false, false);
                kv->deleteThis();
            }
            // make sure the map is visible
            m_pMapList->SetItemVisible(map.m_iListID, true);
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
        wchar_t header[256];
        wchar_t count[128];

        V_snwprintf(count, ARRAYSIZE(count), L"%d", m_pMapList->GetItemCount());
        g_pVGuiLocalize->ConstructString(header, sizeof(header), g_pVGuiLocalize->Find("#MOM_MapSelector_MapCount"), 1, count);
        m_pMapList->SetColumnHeaderText(HEADER_MAP_NAME, header);
    }
    else
    {
        m_pMapList->SetColumnHeaderText(HEADER_MAP_NAME, g_pVGuiLocalize->Find("#MOM_MapSelector_Maps"));
    }
}

//-----------------------------------------------------------------------------
// Purpose: gets filter settings from controls
//-----------------------------------------------------------------------------
void CBaseMapsPage::UpdateFilterSettings()
{
    //Gamemode
    m_iGameModeFilter = m_pGameModeFilter->GetActiveItem();

    // map
    m_pMapFilter->GetText(m_szMapFilter, sizeof(m_szMapFilter) - 1);
    Q_strlower(m_szMapFilter);

    // Difficulty
    char buf[256];
    m_pDifficultyFilter->GetText(buf, sizeof(buf));
    if (buf[0])
    {
        //"< #"
        //The + 2 goes over the '<' and ' '
        m_iDifficultyFilter = Q_atoi(buf + 2);
    }
    else
    {
        m_iDifficultyFilter = 0;
    }

    // Hide completed maps
    m_bFilterHideCompleted = m_pHideCompletedFilterCheck->IsSelected();
    // Showing specfic map layouts?
    m_iMapLayoutFilter = m_pMapLayoutFilter->GetActiveItem();

    // copy filter settings into filter file
    KeyValues *filter = MapSelectorDialog().GetFilterSaveData(GetName());

    filter->SetInt("gamemode", m_iGameModeFilter);
    filter->SetString("map", m_szMapFilter);
    filter->SetInt("difficulty", m_iDifficultyFilter);
    filter->SetBool("HideCompleted", m_bFilterHideCompleted);
    filter->SetInt("maplayout", m_iMapLayoutFilter);

    MapSelectorDialog().SaveUserData();
    OnSaveFilter(filter);

    RecalculateFilterString();
}


//-----------------------------------------------------------------------------
// Purpose: allow derived classes access to the saved filter string
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnSaveFilter(KeyValues *filter)
{
}

//-----------------------------------------------------------------------------
// Purpose: allow derived classes access to the saved filter string
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnLoadFilter(KeyValues *filter)
{
}

//-----------------------------------------------------------------------------
// Purpose: reconstructs the filter description string from the current filter settings
//-----------------------------------------------------------------------------
void CBaseMapsPage::RecalculateFilterString()
{
    //MOM_TODO: What is the purpose of this function?


    /*wchar_t unicode[2048], tempUnicode[128], spacerUnicode[8];
    unicode[0] = 0;
    int iTempUnicodeSize = sizeof(tempUnicode);

    Q_UTF8ToUnicode("; ", spacerUnicode, sizeof(spacerUnicode));

    if (m_szGameFilter[0])
    {
    //Q_UTF8ToUnicode(ModList().GetModNameForModDir(m_szGameFilter, m_iLimitToAppID), tempUnicode, iTempUnicodeSize);
    //wcscat(unicode, tempUnicode);
    //wcscat(unicode, spacerUnicode);
    }

    if (m_iSecureFilter == FILTER_SECURESERVERSONLY)
    {
    wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescSecureOnly"));
    wcscat(unicode, spacerUnicode);
    }
    else if (m_iSecureFilter == FILTER_INSECURESERVERSONLY)
    {
    wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescInsecureOnly"));
    wcscat(unicode, spacerUnicode);
    }

    if (m_pLocationFilter->GetActiveItem() > 0)
    {
    m_pLocationFilter->GetText(tempUnicode, sizeof(tempUnicode));
    wcscat(unicode, tempUnicode);
    wcscat(unicode, spacerUnicode);
    }

    if (m_iDifficultyFilter)
    {
    char tmpBuf[16];
    Q_snprintf(tmpBuf, sizeof(tmpBuf), "%d", m_iDifficultyFilter);

    wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescLatency"));
    Q_UTF8ToUnicode(" < ", tempUnicode, iTempUnicodeSize);
    wcscat(unicode, tempUnicode);
    Q_UTF8ToUnicode(tmpBuf, tempUnicode, iTempUnicodeSize);
    wcscat(unicode, tempUnicode);
    wcscat(unicode, spacerUnicode);
    }

    if (m_bFilterNoFullServers)
    {
    wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNotFull"));
    wcscat(unicode, spacerUnicode);
    }

    if (m_bFilterNoEmptyServers)
    {
    wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNotEmpty"));
    wcscat(unicode, spacerUnicode);
    }

    if (m_bFilterNoPasswordedServers)
    {
    wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNoPassword"));
    wcscat(unicode, spacerUnicode);
    }

    if (m_szMapFilter[0])
    {
    Q_UTF8ToUnicode(m_szMapFilter, tempUnicode, iTempUnicodeSize);
    wcscat(unicode, tempUnicode);
    }

    m_pFilterString->SetText(unicode);*/
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if the map passes the primary filters
//			if the server fails the filters, it will not be refreshed again
//-----------------------------------------------------------------------------
bool CBaseMapsPage::CheckPrimaryFilters(mapstruct_t &map)
{
    // Needs to pass map name filter
    // compare the first few characters of the filter
    int count = Q_strlen(m_szMapFilter);

    if (!Q_strnicmp(map.m_szMapName, "credits", Q_strlen(map.m_szMapName)))
    {
        DevLog("Ignoring credits map\n");
        return false;
    }
    if (!Q_strnicmp(map.m_szMapName, "background_01", Q_strlen(map.m_szMapName)))
    {
        DevLog("Ignoring background map\n");
        return false;
    }

    if (count && !Q_strstr(map.m_szMapName, m_szMapFilter))//strstr returns null if the substring is not in the base string
    {
        DevLog("Map %s does not pass filter %s \n", map.m_szMapName, m_szMapFilter);
        return false;
    }

    //Difficulty needs to pass as well
    if (m_iDifficultyFilter != 0 && map.m_iDifficulty >= m_iDifficultyFilter)
    {
        DevLog("Map %s's difficulty %i is greater than difficulty %i !\n", map.m_szMapName, map.m_iDifficulty, m_iDifficultyFilter);
        return false;
    }
    //Game mode (if it's a surf/bhop/etc map or not)
    if (m_iGameModeFilter != 0 && m_iGameModeFilter != map.m_iGameMode)
    {
        DevLog("Map %s's gamemode %i is not filter gamemode %i !\n", map.m_szMapName, map.m_iGameMode, m_iGameModeFilter);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if a server passes the secondary set of filters
//			server will be continued to be pinged if it fails the filter, since
//			the relvent server data is dynamic
//-----------------------------------------------------------------------------
bool CBaseMapsPage::CheckSecondaryFilters(mapstruct_t &map)
{
    //Completion is only secondary
    if (m_bFilterHideCompleted && map.m_bCompleted)//If we're HIDING completed maps and we've completed it
    {
        DevLog("Map is completed %i and the player is filtering maps %i \n", map.m_bCompleted, m_bFilterHideCompleted);
        return false;//It fails the filter, hide it
    }

    // Map layout (0 = all, 1 = show staged maps only, 2 = show linear maps only)
    if (m_iMapLayoutFilter && ((int) map.m_bHasStages) + 1 == m_iMapLayoutFilter)
    {
        DevLog("Map %s has stages %i and the user is filtering maps %i\n", map.m_szMapName, map.m_bHasStages, m_iMapLayoutFilter);
        return false;
    }

    return CheckTagFilter(map);
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

    if (state)
    {
        MapSelectorDialog().UpdateStatusText("#MOM_MapSelector_SearchingForMaps");

        // clear message in panel
        m_pMapList->SetEmptyListText("");
        m_pQueryMaps->SetText("#MOM_MapSelector_StopSearching");
        m_pQueryMaps->SetCommand("stoprefresh");
        m_pQueryMapsQuick->SetEnabled(false);
    }
    else
    {
        MapSelectorDialog().UpdateStatusText("");
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
    }
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
    if (m_pMapList->GetSelectedItemsCount() < 1)
    {
        m_pStartMap->SetEnabled(false);
    }
    else
    {
        m_pStartMap->SetEnabled(true);
    }
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
// Purpose: 
//-----------------------------------------------------------------------------
/*void CBaseMapsPage::ServerFailedToRespond(int iServer)
{
ServerResponded(iServer);
}*/


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
    engine->ExecuteClientCmd(VarArgs("map %s\n", kv->GetString("map")));
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