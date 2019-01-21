#include "cbase.h"
#include "filesystem.h"

#include "MapSelectorDialog.h"
#include "LocalMaps.h"
#include "OnlineMaps.h"
#include "MapContextMenu.h"
#include "MapInfoDialog.h"
#include "MapFilterPanel.h"

#include "vgui_controls/PropertySheet.h"
#include "vgui/IVGui.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

static CMapSelectorDialog *s_InternetDlg = nullptr;

CMapSelectorDialog &MapSelectorDialog()
{
    return *s_InternetDlg;
}

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapSelectorDialog::CMapSelectorDialog(VPANEL parent) : Frame(nullptr, "CMapSelectorDialog")
{
    SetParent(parent);
    SetProportional(true);
    s_InternetDlg = this;
    m_pSavedData = nullptr;
    m_pFilterData = nullptr;

    LoadUserData();

    m_pLibraryMaps = new CLocalMaps(this);
    m_pOnline = new COnlineMaps(this);

    m_pCurrentMapList = static_cast<IMapList*>(m_pLibraryMaps);

    m_pContextMenu = new CMapContextMenu(this);

    // property sheet
    m_pTabPanel = new PropertySheet(this, "MapTabs");
    m_pTabPanel->SetSize(10, 10); // Fix "parent not sized yet" spew
    m_pTabPanel->SetTabWidth(72);
    m_pTabPanel->SetSmallTabs(true);
    m_pTabPanel->AddPage(m_pLibraryMaps, "#MOM_MapSelector_LibraryMaps");
    m_pTabPanel->AddPage(m_pOnline, "#MOM_MapSelector_BrowseMaps");

    m_pTabPanel->AddActionSignalTarget(this);

    m_pStatusLabel = new Label(this, "StatusLabel", "");

    m_pFilterPanel = new MapFilterPanel(this);

    LoadControlSettings("resource/ui/MapSelector/DialogMapSelector.res");

    SetMinimumSize(680, 400);

    m_pStatusLabel->SetText("");

    // load current tab
    const char *mapList = m_pSavedData->GetString("MapList", "local");
    if (!Q_stricmp(mapList, "local"))
    {
        m_pTabPanel->SetActivePage(m_pLibraryMaps);
    }
    else if (!Q_stricmp(mapList, "online"))
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
    MoveToCenterOfScreen();
}


//-----------------------------------------------------------------------------
// Purpose: Activates and gives the tab focus
//-----------------------------------------------------------------------------
void CMapSelectorDialog::Open()
{
    BaseClass::Activate();
    m_pTabPanel->RequestFocus();
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
    {
        m_pSavedData->deleteThis();
    }

    m_pSavedData = new KeyValues("Filters");
    if (!m_pSavedData->LoadFromFile(g_pFullFileSystem, "cfg/MapSelector.vdf", "MOD"))
    {
        // doesn't matter if the file is not found, defaults will work successfully and file will be created on exit
    }

    KeyValues *filters = m_pSavedData->FindKey("Filters", false);
    if (filters)
    {
        m_pFilterData = filters->MakeCopy();
        m_pSavedData->RemoveSubKey(filters);
    }
    else
    {
        m_pFilterData = new KeyValues("Filters");
    }

    MoveToCenterOfScreen();

    InvalidateLayout(true);
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapSelectorDialog::SaveUserData()
{
    if (!g_pFullFileSystem) return;

    m_pSavedData->Clear();
    m_pSavedData->LoadFromFile(g_pFullFileSystem, "cfg/MapSelector.vdf", "MOD");

    // set the current tab
    if (m_pCurrentMapList == m_pLibraryMaps)
    {
        m_pSavedData->SetString("MapList", "local");
    }
    else if (m_pCurrentMapList == m_pOnline)
    {
        m_pSavedData->SetString("MapList", "online");//MOM_TODO
    }

    m_pSavedData->RemoveSubKey(m_pSavedData->FindKey("Filters")); // remove the saved subkey and add our subkey
    m_pSavedData->AddSubKey(m_pFilterData->MakeCopy());
    m_pSavedData->SaveToFile(g_pFullFileSystem, "cfg/MapSelector.vdf", "MOD");

    // save per-page config
    SaveUserConfig();
}

//-----------------------------------------------------------------------------
// Purpose: refreshes the page currently visible
//-----------------------------------------------------------------------------
void CMapSelectorDialog::RefreshCurrentPage()
{
    if (m_pCurrentMapList)
    {
        m_pCurrentMapList->StartRefresh();
    }
}

void CMapSelectorDialog::OnSizeChanged(int wide, int tall)
{
    BaseClass::OnSizeChanged(wide, tall);
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
CDialogMapInfo *CMapSelectorDialog::OpenMapInfoDialog(IMapList *gameList, KeyValues *pMap)
{
    //mapstruct_t *pServer = gameList->GetMap(serverIndex);
    //if (!pServer)
    

    //MOM_TODO: complete the following so people can see information on the map 

    //We're going to send just the map name to the CDialogMapInfo() constructor,
    //then to the server and populate it with leaderboard times, replays, personal bests, etc
    const char *pMapName = pMap->GetString("name", "");
    CDialogMapInfo *gameDialog = new CDialogMapInfo(nullptr, pMapName);
    gameDialog->SetParent(GetVParent());
    gameDialog->AddActionSignalTarget(this);
    gameDialog->Run(pMapName);
    int i = m_vecMapInfoDialogs.AddToTail();
    m_vecMapInfoDialogs[i] = gameDialog;
    return gameDialog;
    //return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: opens a game info dialog by a specified IP, not attached to any game list
//-----------------------------------------------------------------------------
CDialogMapInfo *CMapSelectorDialog::OpenMapInfoDialog(int serverIP, uint16 connPort, uint16 queryPort)
{
    CDialogMapInfo *gameDialog = new CDialogMapInfo(nullptr, "");
    gameDialog->AddActionSignalTarget(this);
    gameDialog->SetParent(GetVParent());
    gameDialog->Run("");
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
    return m_pFilterData->FindKey(m_pTabPanel->GetActivePage()->GetName(), true);
}

void CMapSelectorDialog::LoadTabFilterData(const char *pTabName)
{
    m_pFilterPanel->LoadFilterSettings(m_pFilterData->FindKey(pTabName, true));
}

void CMapSelectorDialog::ApplyFiltersToCurrentTab()
{
    CBaseMapsPage *pCurrent = dynamic_cast<CBaseMapsPage*>(m_pTabPanel->GetActivePage());
    if (pCurrent)
        pCurrent->ApplyFilters(m_pFilterPanel);
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