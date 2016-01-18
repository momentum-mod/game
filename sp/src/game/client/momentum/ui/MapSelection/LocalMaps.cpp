#include "pch_mapselection.h"

using namespace vgui;

const float BROADCAST_LIST_TIMEOUT = 0.4f;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLocalMaps::CLocalMaps(vgui::Panel *parent, bool bAutoRefresh, const char *pCustomResFilename) :
CBaseMapsPage(parent, "LocalMaps", pCustomResFilename)
{
    m_iServerRefreshCount = 0;
    m_bRequesting = false;
    m_bAutoRefresh = bAutoRefresh;//MOM_TODO: Consider making this false
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLocalMaps::~CLocalMaps()
{
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh
//-----------------------------------------------------------------------------
void CLocalMaps::OnPageShow()
{
    if (m_bAutoRefresh)
        StartRefresh();
}


//-----------------------------------------------------------------------------
// Purpose: Called every frame
//-----------------------------------------------------------------------------
void CLocalMaps::OnTick()
{
    BaseClass::OnTick();
    CheckRetryRequest();
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CLocalMaps::SupportsItem(InterfaceItem_e item)
{
    switch (item)
    {
    case FILTERS:
        return true;

    case GETNEWLIST:
    default:
        return false;
    }
}

//-----------------------------------------------------------------------------
// Purpose: starts the servers refreshing
//-----------------------------------------------------------------------------
void CLocalMaps::StartRefresh()
{
    Log("STARTING REFRESH!\n");
    BaseClass::StartRefresh();
    //Populate the main list
    FileFindHandle_t found;
    //MOM_TODO: make this by *.mom
    const char *pMapName = g_pFullFileSystem->FindFirstEx("maps/*.bsp", "MOD", &found);
    while (pMapName)
    {
        Log("FOUND MAP %s!\n", pMapName);
        //MOM_TODO: Read the .mom file and create the struct based off of it
        //KeyValues* kv = new KeyValues("Map");
        //kv->LoadFromFile(g_pFullFileSystem, pMapName, "MOD");
        //Q_FileBase() can be used to strip the maps/ and .mom, returns file name

        //Q_SetExtension can set the extension for the maps file
        mapdisplay_t map;
        mapstruct_t m;
        map.m_bDoNotRefresh = true;

        Q_FileBase(pMapName, m.m_szMapName, MAX_PATH);
        Log("Stripped name: %s\n", m.m_szMapName);
        m.m_iGameMode = GAMEMODE_SURF;//Temp to show data, MOM_TODO change this upon reading .mom file
        m.m_iDifficulty = 1;
        m.m_bHasStages = false;
        m.m_bCompleted = false;
        Q_strcpy(m.m_szBestTime, "10 seconds");
        map.m_mMap = m;
        m_vecMaps.AddToTail(map);

        pMapName = g_pFullFileSystem->FindNext(found);
    }
    g_pFullFileSystem->FindClose(found);
    BaseClass::UpdateLocalMaps();
    //m_fRequestTime = Plat_FloatTime();
}


void CLocalMaps::OnLoadFilter(KeyValues *kv)
{

}


//-----------------------------------------------------------------------------
// Purpose: Control which button are visible.
//-----------------------------------------------------------------------------
void CLocalMaps::ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter)
{
    m_pStartMap->SetVisible(bShowConnect);
    m_pRefreshAll->SetVisible(bShowRefreshAll);
    m_pFilter->SetVisible(bShowFilter);
}


//-----------------------------------------------------------------------------
// Purpose: stops current refresh/GetNewServerList()
//-----------------------------------------------------------------------------
void CLocalMaps::StopRefresh()
{
    BaseClass::StopRefresh();
    // clear update states
    m_bRequesting = false;
}

//-----------------------------------------------------------------------------
// Purpose: Check to see if we've finished looking for local servers
//-----------------------------------------------------------------------------
void CLocalMaps::CheckRetryRequest()
{
    if (!m_bRequesting)
        return;

    double curtime = Plat_FloatTime();
    if (curtime - m_fRequestTime <= BROADCAST_LIST_TIMEOUT)
    {
        return;
    }

    // time has elapsed, finish up
    m_bRequesting = false;
}

//-----------------------------------------------------------------------------
// Purpose: called when a server response has timed out, remove it
//-----------------------------------------------------------------------------
void CLocalMaps::ServerFailedToRespond(HServerListRequest hReq, int iServer)
{
    //int iServerMap = m_mapServers.Find(iServer);
    //if (iServerMap != m_mapServers.InvalidIndex())
    //    RemoveMap(m_mapServers[iServerMap]);
}

//-----------------------------------------------------------------------------
// Purpose: called when the current refresh list is complete
//-----------------------------------------------------------------------------
void CLocalMaps::RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response)
{
    SetRefreshing(false);
    m_pGameList->SortList();
    m_iServerRefreshCount = 0;
    m_pGameList->SetEmptyListText("#ServerBrowser_NoLanServers");
    SetEmptyListText();
}

void CLocalMaps::SetEmptyListText()
{
    m_pGameList->SetEmptyListText("#ServerBrowser_NoLanServers");
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void CLocalMaps::OnOpenContextMenu(int row)
{
    if (!m_pGameList->GetSelectedItemsCount())
        return;

    // get the server
    int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(0));
    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu(m_pGameList);
    menu->ShowMenu(this, serverID, true, true, true, false);
}