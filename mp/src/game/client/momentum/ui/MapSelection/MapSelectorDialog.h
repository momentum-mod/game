#pragma once

#include "vgui_controls/Frame.h"

struct MapDisplay_t;
struct MapFilters_t;
struct MapData;
class CMapContextMenu;
class CDialogMapInfo;
class CLibraryMaps;
class CBrowseMaps;
class CFavoriteMaps;
class IMapList;
class MapFilterPanel;
class MapDownloadProgress;

struct MapListData
{
    MapData *m_pMapData;
    KeyValues *m_pKv;
    int m_iThumbnailImageIndx;
    vgui::IImage *m_pImage;

    MapListData();
    ~MapListData();
};

enum RESERVED_IMAGE_INDICES
{
    // Start index is 1 because the 0th element is a BlankImage inside ImageLists
    INDX_MAP_THUMBNAIL_UNKNOWN = 1,
    INDX_MAP_IN_LIBRARY,
    INDX_MAP_NOT_IN_LIBRARY,
    INDX_MAP_IN_FAVORITES,
    INDX_MAP_NOT_IN_FAVORITES,
    INDX_MAP_IS_LINEAR,
    INDX_MAP_IS_STAGED,


    // MAKE SURE THIS IS LAST!
    INDX_RESERVED_COUNT,
};

#define HEADER_ICON_SIZE 14

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

    // displays the dialog, moves it into focus, updates if it has to
    void Open(void);

    void OnClose() OVERRIDE;

    // context menu access
    CMapContextMenu *GetContextMenu();

    // opens a game info dialog from a game list
    void OpenMapInfoDialog(MapData *pMapData);
    void UpdateMapInfoDialog(uint32 uMapID);

    // closes all the map info dialogs
    void CloseAllMapInfoDialogs();
    void CloseMapInfoDialog(uint32 uMapID);
    void RemoveMapInfoDialog(uint32 uMapID);

    // accessor to the filter save data
    KeyValues *GetCurrentTabFilterData();
    KeyValues *GetTabFilterData(const char *pTabName);
    void LoadTabFilterData(const char *pTabName);
    void ApplyFiltersToCurrentTab(MapFilters_t filters);

    int GetFilteredItemsCount();
    void StartRandomMapFromCurrentTab();

    // load/saves filter & favorites settings from disk
    void LoadUserData();
    void SaveUserData();

    void LoadDefaultImageList();
    vgui::ImageList *GetImageList() { return m_pImageList; }

    // Map data handling
    void OnMapCacheUpdated(KeyValues *pKv);
    void OnMapDataUpdated(KeyValues *pKv);
    void CreateMapListData(MapData *pData);
    void UpdateMapListData(uint32 uMapID, bool bMain, bool bInfo, bool bPB, bool bWR, bool bThumbnail);
    MapListData *GetMapListDataByID(uint32 uMapID);

    // Callbacks for download
    void OnMapDownloadQueued(KeyValues *pKv);
    void OnMapDownloadStart(KeyValues *pKv);
    void OnMapDownloadSize(KeyValues *pKv);
    void OnMapDownloadProgress(KeyValues *pKv);
    void OnMapDownloadEnd(KeyValues *pKv);

    bool IsMapDownloading(uint32 uMapID) const;
    MapDownloadProgress *GetDownloadProgressPanel(uint32 uMapID);

    // Called when map should be added to/removed from library
    MESSAGE_FUNC_INT(OnAddMapToLibrary, "AddToLibrary", id);
    MESSAGE_FUNC_INT(OnRemoveMapFromLibrary, "RemoveFromLibrary", id);
    // Called when map should be added to/removed from favorites
    MESSAGE_FUNC_INT(OnAddMapToFavorites, "AddToFavorites", id);
    MESSAGE_FUNC_INT(OnRemoveMapFromFavorites, "RemoveFromFavorites", id);
    // Called when user wants to download/cancel download
    MESSAGE_FUNC_INT(OnStartMapDownload, "DownloadMap", id);
    MESSAGE_FUNC_INT(OnRemoveFromQueue, "RemoveFromQueue", id);
    MESSAGE_FUNC_INT(OnCancelMapDownload, "CancelDownload", id);
    MESSAGE_FUNC_INT(OnConfirmCancelMapDownload, "ConfirmCancelDownload", id);
    MESSAGE_FUNC_INT(OnRejectCancelMapDownload, "RejectCancelDownload", id);
    MESSAGE_FUNC_INT(OnConfirmOverwrite, "ConfirmOverwrite", id);
    MESSAGE_FUNC_INT(OnRejectOverwrite, "RejectOverwrite", id);
    // Called when map should be started
    MESSAGE_FUNC_INT(OnMapStart, "StartMap", id);
    // Refresh this map's info
    MESSAGE_FUNC_INT(OnRefreshMapInfo, "RefreshMapInfo", id);
    // called to look at map info
    MESSAGE_FUNC_INT(OnViewMapInfo, "ViewMapInfo", id);

protected:
    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;
    void OnReloadControls() OVERRIDE;

  private:
    // current game list change
    MESSAGE_FUNC(OnTabChanged, "PageChanged");

    virtual void ActivateBuildMode();

    // list of all open game info dialogs
    CUtlMap<uint32, vgui::DHANDLE<CDialogMapInfo>> m_mapMapInfoDialogs;

    // Map of all cancel map dialogs
    CUtlMap<uint32, Panel*> m_mapCancelConfirmDlgs;

    // Map of all overwrite dialogs
    CUtlMap<uint32, Panel*> m_mapOverwriteConfirmDlgs;

    // Map of all map list data
    CUtlMap<uint32, MapListData*> m_mapMapListData;

    // Map of all downloads
    CUtlMap<uint32, MapDownloadProgress *> m_mapMapDownloads;

    // pointer to current game list
    IMapList *m_pCurrentMapList;

    // Map image list
    vgui::ImageList *m_pImageList;

    // property sheet
    vgui::PropertySheet *m_pTabPanel;

    // Map tabs
    CLibraryMaps *m_pLibraryMaps;
    CBrowseMaps *m_pBrowseMaps;
    CFavoriteMaps *m_pFavoriteMaps;

    // Filters
    MapFilterPanel *m_pFilterPanel;

    // Filter data
    KeyValues *m_pSavedData;  // Saved on disk filter data
    KeyValues *m_pFilterData; // Current filter data in the Dialog

    // context menu
    CMapContextMenu *m_pContextMenu;

    // Modulecomms event listening
    uint16 m_iMapDataIndx, m_iMapCacheUpdateIndx, m_iDownloadQueueIndx, m_iDownloadSizeIndx, m_iDownloadStartIndx,
        m_iDownloadProgressIndx, m_iDownloadEndIndx;

    Color m_cMapDownloadQueued, m_cMapDownloadNeeded;
};

// singleton accessor
extern CMapSelectorDialog &MapSelectorDialog();