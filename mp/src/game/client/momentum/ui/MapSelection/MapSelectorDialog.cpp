#include "cbase.h"
#include "filesystem.h"

#include "MapSelectorDialog.h"
#include "LibraryMaps.h"
#include "BrowseMaps.h"
#include "MapContextMenu.h"
#include "MapInfoDialog.h"
#include "MapFilterPanel.h"
#include "mom_map_cache.h"

#include "vgui_controls/PropertySheet.h"
#include "vgui/IVGui.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static CMapSelectorDialog *s_InternetDlg = nullptr;

CMapSelectorDialog &MapSelectorDialog()
{
    return *s_InternetDlg;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapSelectorDialog::CMapSelectorDialog(VPANEL parent) : Frame(nullptr, "CMapSelectorDialog")
{
    SetParent(parent);
    SetProportional(true);
    SetSize(680, 400);
    s_InternetDlg = this;
    m_pSavedData = nullptr;
    m_pFilterData = nullptr;

    LoadUserData();

    m_pLibraryMaps = new CLibraryMaps(this);
    m_pOnline = new CBrowseMaps(this);

    m_pCurrentMapList = static_cast<IMapList*>(m_pLibraryMaps);

    m_pContextMenu = new CMapContextMenu(this);

    // property sheet
    m_pTabPanel = new PropertySheet(this, "MapTabs");
    m_pTabPanel->SetSize(10, 10); // Fix "parent not sized yet" spew
    m_pTabPanel->SetTabWidth(72);
    m_pTabPanel->SetSmallTabs(true);
    // Defaults to m_pLibraryMaps being selected here, since it is added first
    m_pTabPanel->AddPage(m_pLibraryMaps, "#MOM_MapSelector_LibraryMaps");
    m_pTabPanel->AddPage(m_pOnline, "#MOM_MapSelector_BrowseMaps");

    m_pTabPanel->AddActionSignalTarget(this);

    m_pStatusLabel = new Label(this, "StatusLabel", "");

    m_pFilterPanel = new MapFilterPanel(this);

    LoadControlSettings("resource/ui/MapSelector/DialogMapSelector.res");

    SetMinimumSize(680, 400);

    m_pStatusLabel->SetText("");

    // load current tab
    MapListType_e current = (MapListType_e) m_pSavedData->GetInt("current", MAP_LIST_LIBRARY);
    if (current == MAP_LIST_BROWSE)
    {
        m_pTabPanel->SetActivePage(m_pOnline);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapSelectorDialog::~CMapSelectorDialog()
{
    if (m_pContextMenu)
        m_pContextMenu->DeletePanel();
    
    // Attempt to save user data, if not that's okay
    SaveUserData();

    if (m_pSavedData)
    {
        m_pSavedData->deleteThis();
    }
}


//-----------------------------------------------------------------------------
// Purpose: Called once to set up
//-----------------------------------------------------------------------------
void CMapSelectorDialog::Initialize()
{
    SetTitle("#MOM_MapSelector_Maps", true);
    SetVisible(false);
}


//-----------------------------------------------------------------------------
// Purpose: Activates and gives the tab focus
//-----------------------------------------------------------------------------
void CMapSelectorDialog::Open()
{
    MoveToCenterOfScreen();
    BaseClass::Activate();
    m_pTabPanel->RequestFocus();
    dynamic_cast<PropertyPage*>(m_pTabPanel->GetActivePage())->OnPageShow();
}

void CMapSelectorDialog::OnClose()
{
    SaveUserData();
    BaseClass::OnClose();
}


//-----------------------------------------------------------------------------
// Purpose: Loads filter settings from disk
//-----------------------------------------------------------------------------
void CMapSelectorDialog::LoadUserData()
{
    // free any old filters
    if (m_pSavedData)
        m_pSavedData->deleteThis();

    m_pSavedData = new KeyValues("Filters");
    m_pSavedData->LoadFromFile(g_pFullFileSystem, "cfg/MapSelector.vdf", "MOD");

    m_pFilterData = m_pSavedData->FindKey("Filters", true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapSelectorDialog::SaveUserData()
{
    if (!g_pFullFileSystem) return;

    // set the current tab
    m_pSavedData->SetInt("current", m_pCurrentMapList->GetMapListType());

    m_pSavedData->SaveToFile(g_pFullFileSystem, "cfg/MapSelector.vdf", "MOD");

    // save per-page config
    SaveUserConfig();
}

//-----------------------------------------------------------------------------
// Purpose: Updates status test at bottom of window
//-----------------------------------------------------------------------------
void CMapSelectorDialog::UpdateStatusText(const char *fmt, ...)
{
    if (!m_pStatusLabel)
        return;

    if (fmt && strlen(fmt) > 0)
    {
        char str[1024];
        va_list argptr;
        va_start(argptr, fmt);
        _vsnprintf(str, sizeof(str), fmt, argptr);
        va_end(argptr);

        m_pStatusLabel->SetText(str);
    }
    else
    {
        // clear
        m_pStatusLabel->SetText("");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Updates when the tabs are changed
//-----------------------------------------------------------------------------
void CMapSelectorDialog::OnTabChanged()
{
    m_pCurrentMapList = dynamic_cast<IMapList *>(m_pTabPanel->GetActivePage());
    m_pCurrentMapList->LoadFilters();

    UpdateStatusText(nullptr);

    InvalidateLayout();
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMapContextMenu *CMapSelectorDialog::GetContextMenu(Panel *pPanel)
{
    // create a drop down for this object's states
    if (m_pContextMenu)
        delete m_pContextMenu;
    m_pContextMenu = new CMapContextMenu(this);
    m_pContextMenu->SetAutoDelete(false);

    if (!pPanel)
    {
        m_pContextMenu->SetParent(this);
    }
    else
    {
        m_pContextMenu->SetParent(pPanel);
    }

    m_pContextMenu->SetVisible(false);
    return m_pContextMenu;
}

//-----------------------------------------------------------------------------
// Purpose: opens a game info dialog from a game list
//-----------------------------------------------------------------------------
CDialogMapInfo *CMapSelectorDialog::OpenMapInfoDialog(IMapList *gameList, MapData *pMapData)
{
    //We're going to send just the map name to the CDialogMapInfo() constructor,
    //then to the server and populate it with leaderboard times, replays, personal bests, etc
    CDialogMapInfo *gameDialog = new CDialogMapInfo(nullptr, pMapData);
    gameDialog->SetParent(GetVPanel());
    gameDialog->AddActionSignalTarget(this);
    gameDialog->Run();
    int i = m_vecMapInfoDialogs.AddToTail();
    m_vecMapInfoDialogs[i] = gameDialog;
    return gameDialog;
}

//-----------------------------------------------------------------------------
// Purpose: closes all the game info dialogs
//-----------------------------------------------------------------------------
void CMapSelectorDialog::CloseAllMapInfoDialogs()
{
    for (int i = 0; i < m_vecMapInfoDialogs.Count(); i++)
    {
        Panel *dlg = m_vecMapInfoDialogs[i];
        if (dlg)
        {
            ivgui()->PostMessage(dlg->GetVPanel(), new KeyValues("Close"), NULL);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: accessor to the filter save data
//-----------------------------------------------------------------------------
KeyValues* CMapSelectorDialog::GetCurrentTabFilterData()
{
    return GetTabFilterData(m_pTabPanel->GetActivePage()->GetName());
}

KeyValues* CMapSelectorDialog::GetTabFilterData(const char* pTabName)
{
    return m_pFilterData->FindKey(pTabName, true);
}

void CMapSelectorDialog::LoadTabFilterData(const char *pTabName)
{
    m_pFilterPanel->LoadFilterSettings(m_pFilterData->FindKey(pTabName, true));
}

void CMapSelectorDialog::ApplyFiltersToCurrentTab(MapFilters_t filters)
{
    if (m_pCurrentMapList)
        m_pCurrentMapList->ApplyFilters(filters);
}

//-----------------------------------------------------------------------------
// Purpose: Adds server to the history, saves as currently connected server
//-----------------------------------------------------------------------------
void CMapSelectorDialog::OnConnectToGame(KeyValues *pMessageValues)
{
    //MOM_TODO: Make this OnStartMap/OnDownloadMap or similar

    int ip = pMessageValues->GetInt("ip");
    int connectionPort = pMessageValues->GetInt("connectionport");
    int queryPort = pMessageValues->GetInt("queryport");

    if (!ip || !queryPort)
        return;

    /*memset(&m_CurrentConnection, 0, sizeof(gameserveritem_t));
    m_CurrentConnection.m_NetAdr.SetIP(ip);
    m_CurrentConnection.m_NetAdr.SetQueryPort(queryPort);
    m_CurrentConnection.m_NetAdr.SetConnectionPort((unsigned short) connectionPort);*/
#ifndef NO_STEAM
    //if (m_pHistory && SteamMatchmaking())
    //{
    //    SteamMatchmaking()->AddFavoriteGame2(0, ::htonl(ip), connectionPort, queryPort, k_unFavoriteFlagHistory, time(NULL));
    //    m_pHistory->SetRefreshOnReload();
    //}
#endif
    // tell the game info dialogs, so they can cancel if we have connected
    // to a server they were auto-retrying
    for (int i = 0; i < m_vecMapInfoDialogs.Count(); i++)
    {
        Panel *dlg = m_vecMapInfoDialogs[i];
        if (dlg)
        {
            KeyValues *kv = new KeyValues("ConnectedToGame", "ip", ip, "connectionport", connectionPort);
            kv->SetInt("queryport", queryPort);
            ivgui()->PostMessage(dlg->GetVPanel(), kv, NULL);
        }
    }

    // forward to favorites
    //m_pFavorites->OnConnectToGame();

    m_bCurrentlyConnected = true;
}

//-----------------------------------------------------------------------------
// Purpose: Clears currently connected server
//-----------------------------------------------------------------------------
void CMapSelectorDialog::OnDisconnectFromGame(void)
{
    m_bCurrentlyConnected = false;
}

//-----------------------------------------------------------------------------
// Purpose: Passes build mode activation down into the pages
//-----------------------------------------------------------------------------
void CMapSelectorDialog::ActivateBuildMode()
{
    // no subpanel, no build mode
    EditablePanel *panel = dynamic_cast<EditablePanel *>(m_pTabPanel->GetActivePage());
    if (!panel)
        return;

    if (panel->GetBuildGroup()->IsEnabled())
        BaseClass::ActivateBuildMode();
    else
        panel->ActivateBuildMode();
}