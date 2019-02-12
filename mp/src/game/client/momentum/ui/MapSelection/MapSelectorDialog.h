#pragma once

#include "vgui_controls/Frame.h"

class CMapContextMenu;
class CDialogMapInfo;
class CLibraryMaps;
class COnlineMaps;
class IMapList;
class MapFilterPanel;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CMapSelectorDialog : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CMapSelectorDialog, vgui::Frame);

public:
    // Construction/destruction
    CMapSelectorDialog(vgui::VPANEL parent);
    ~CMapSelectorDialog(void);

    void		Initialize(void);

    // displays the dialog, moves it into focus, updates if it has to
    void		Open(void);
    
    void OnClose() OVERRIDE;

    // updates status text at bottom of window
    void UpdateStatusText(const char *format, ...);

    // context menu access
    CMapContextMenu *GetContextMenu(Panel *pParent);

    // opens a game info dialog from a game list
    CDialogMapInfo *OpenMapInfoDialog(IMapList *gameList, KeyValues *pMap);

    // opens a game info dialog by a specified IP, not attached to any game list
    CDialogMapInfo *OpenMapInfoDialog(int serverIP, uint16 connPort, uint16 queryPort);

    // closes all the map info dialogs
    void CloseAllMapInfoDialogs();

    // accessor to the filter save data
    KeyValues *GetCurrentTabFilterData();
    KeyValues *GetTabFilterData(const char *pTabName);
    void LoadTabFilterData(const char *pTabName);
    void ApplyFiltersToCurrentTab();

    // load/saves filter & favorites settings from disk
    void		LoadUserData();
    void		SaveUserData();

private:

    // current game list change
    MESSAGE_FUNC(OnTabChanged, "PageChanged");

    // notification that we connected / disconnected
    MESSAGE_FUNC_PARAMS(OnConnectToGame, "ConnectedToGame", kv);
    MESSAGE_FUNC(OnDisconnectFromGame, "DisconnectedFromGame");

    virtual void ActivateBuildMode();

private:
    // list of all open game info dialogs
    CUtlVector<vgui::DHANDLE<CDialogMapInfo> > m_vecMapInfoDialogs;

    // pointer to current game list
    IMapList *m_pCurrentMapList;

    // Status text
    vgui::Label	*m_pStatusLabel;

    // property sheet
    vgui::PropertySheet *m_pTabPanel;

    //Map tabs
    CLibraryMaps *m_pLibraryMaps;
    COnlineMaps *m_pOnline;

    // Filters
    MapFilterPanel *m_pFilterPanel;

    //Filter data
    KeyValues *m_pSavedData;//Saved on disk filter data
    KeyValues *m_pFilterData;//Current filter data in the Dialog

    // context menu
    CMapContextMenu *m_pContextMenu;

    // currently connected game
    bool m_bCurrentlyConnected;
};

// singleton accessor
extern CMapSelectorDialog &MapSelectorDialog();