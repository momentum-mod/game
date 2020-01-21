#include "cbase.h"
#include "filesystem.h"
#include <ctime>

#include "MapSelectorDialog.h"
#include "LibraryMaps.h"
#include "BrowseMaps.h"
#include "FavoriteMaps.h"
#include "MapContextMenu.h"
#include "MapInfoDialog.h"
#include "MapFilterPanel.h"
#include "MapDownloadProgress.h"

#include "mom_map_cache.h"
#include "mom_modulecomms.h"
#include "util/mom_util.h"
#include "controls/FileImage.h"

#include "IMessageboxPanel.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/ImageList.h"
#include "vgui/IVGui.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static CMapSelectorDialog *s_MapDlg = nullptr;

CMapSelectorDialog &MapSelectorDialog()
{
    return *s_MapDlg;
}

MapListData::MapListData(): m_pMapData(nullptr), m_pKv(nullptr), m_iThumbnailImageIndx(INDX_MAP_THUMBNAIL_UNKNOWN), m_pImage(nullptr)
{
}

MapListData::~MapListData()
{
    if (m_pKv)
        m_pKv->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapSelectorDialog::CMapSelectorDialog(VPANEL parent) : Frame(nullptr, "CMapSelectorDialog")
{
    SetDefLessFunc(m_mapMapListData);
    SetDefLessFunc(m_mapMapDownloads);
    SetDefLessFunc(m_mapCancelConfirmDlgs);
    SetDefLessFunc(m_mapMapInfoDialogs);
    SetDefLessFunc(m_mapOverwriteConfirmDlgs);

    SetParent(parent);
    SetScheme(scheme()->LoadSchemeFromFile("resource/MapSelectorScheme.res", "MapSelectorScheme"));
    SetProportional(true);
    SetSize(GetScaledVal(600), GetScaledVal(300));
    s_MapDlg = this;
    m_pSavedData = nullptr;
    m_pFilterData = nullptr;

    LoadUserData();

    m_pImageList = new ImageList(false);
    LoadDefaultImageList();

    m_pLibraryMaps = new CLibraryMaps(this);
    m_pBrowseMaps = new CBrowseMaps(this);
    m_pFavoriteMaps = new CFavoriteMaps(this);

    m_pContextMenu = new CMapContextMenu(this);

    // property sheet
    m_pTabPanel = new PropertySheet(this, "MapTabs");
    m_pTabPanel->SetSize(10, 10); // Fix "parent not sized yet" spew
    m_pTabPanel->SetTabWidth(72);
    m_pTabPanel->SetSmallTabs(true);
    // Defaults to first added page
    m_pTabPanel->AddPage(m_pBrowseMaps, "#MOM_MapSelector_BrowseMaps");
    m_pTabPanel->AddPage(m_pFavoriteMaps, "#MOM_MapSelector_FavoriteMaps");
    m_pTabPanel->AddPage(m_pLibraryMaps, "#MOM_MapSelector_LibraryMaps");

    m_pTabPanel->AddActionSignalTarget(this);

    m_pFilterPanel = new MapFilterPanel(this);

    LoadControlSettings("resource/ui/mapselector/DialogMapSelector.res");

    SetMinimumSize(GetScaledVal(340), GetScaledVal(250));

    // load current tab
    MapListType_e current = (MapListType_e) m_pSavedData->GetInt("current", MAP_LIST_BROWSE);
    CBaseMapsPage *pCurrentTab;
    switch (current)
    {
    case MAP_LIST_LIBRARY:
        pCurrentTab = m_pLibraryMaps;
        break;
    case MAP_LIST_FAVORITES:
        pCurrentTab = m_pFavoriteMaps;
        break;
    case MAP_LIST_BROWSE:
    default:
        pCurrentTab = m_pBrowseMaps;
        break;
    }

    m_pTabPanel->SetActivePage(pCurrentTab);

    SetTitle("#MOM_MapSelector_Maps", true);
    SetVisible(false);

    // Listen for map cache events
    m_iMapDataIndx = g_pModuleComms->ListenForEvent("map_data_update", UtlMakeDelegate(this, &CMapSelectorDialog::OnMapDataUpdated));
    m_iMapCacheUpdateIndx = g_pModuleComms->ListenForEvent("map_cache_updated", UtlMakeDelegate(this, &CMapSelectorDialog::OnMapCacheUpdated));
    // Listen for download events
    m_iDownloadQueueIndx = g_pModuleComms->ListenForEvent("map_download_queued", UtlMakeDelegate(this, &CMapSelectorDialog::OnMapDownloadQueued));
    m_iDownloadStartIndx = g_pModuleComms->ListenForEvent("map_download_start", UtlMakeDelegate(this, &CMapSelectorDialog::OnMapDownloadStart));
    m_iDownloadSizeIndx = g_pModuleComms->ListenForEvent("map_download_size", UtlMakeDelegate(this, &CMapSelectorDialog::OnMapDownloadSize));
    m_iDownloadProgressIndx = g_pModuleComms->ListenForEvent("map_download_progress", UtlMakeDelegate(this, &CMapSelectorDialog::OnMapDownloadProgress));
    m_iDownloadEndIndx = g_pModuleComms->ListenForEvent("map_download_end", UtlMakeDelegate(this, &CMapSelectorDialog::OnMapDownloadEnd));
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
        m_pSavedData->deleteThis();

    m_mapMapDownloads.PurgeAndDeleteElements();
    m_mapCancelConfirmDlgs.PurgeAndDeleteElements();
    m_mapMapListData.PurgeAndDeleteElements();

    CloseAllMapInfoDialogs();

    // Map cache events
    g_pModuleComms->RemoveListener("map_data_update", m_iMapDataIndx);
    g_pModuleComms->RemoveListener("map_cache_updated", m_iMapCacheUpdateIndx);
    // Download events
    g_pModuleComms->RemoveListener("map_download_queued", m_iDownloadQueueIndx);
    g_pModuleComms->RemoveListener("map_download_start", m_iDownloadStartIndx);
    g_pModuleComms->RemoveListener("map_download_size", m_iDownloadSizeIndx);
    g_pModuleComms->RemoveListener("map_download_progress", m_iDownloadProgressIndx);
    g_pModuleComms->RemoveListener("map_download_end", m_iDownloadEndIndx);
}

//-----------------------------------------------------------------------------
// Purpose: Activates and gives the tab focus
//-----------------------------------------------------------------------------
void CMapSelectorDialog::Open()
{
    BaseClass::Activate();
    PostActionSignal(new KeyValues("MapSelectorOpened"));
    m_pTabPanel->RequestFocus();
    m_pCurrentMapList->LoadFilters();
}

void CMapSelectorDialog::OnClose()
{
    SaveUserData();
    BaseClass::OnClose();
    PostActionSignal(new KeyValues("MapSelectorClosed"));
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

inline IImage* LoadFileImage(const char *pPath, int wide, int tall, IImage *pDefault)
{
    FileImage *pFileImage = new FileImage(pDefault);
    if (pFileImage->LoadFromFile(pPath))
        pFileImage->SetSize(wide, tall);
    return pFileImage;
}

void CMapSelectorDialog::LoadDefaultImageList()
{
    // Work backwards, since the first call will do the growth and fill with nulls,
    // and subsequent calls only replace nulls with actual images
    IImage *pNullImage = scheme()->GetImage("", false);
    const int wide = GetScaledVal(HEADER_ICON_SIZE);
    const int tall = GetScaledVal(HEADER_ICON_SIZE);
    const int layoutDim = GetScaledVal(20);
    m_pImageList->SetImageAtIndex(INDX_MAP_IS_STAGED, LoadFileImage("materials/vgui/icon/map_selector/Staged.png", layoutDim, layoutDim, pNullImage));
    m_pImageList->SetImageAtIndex(INDX_MAP_IS_LINEAR, LoadFileImage("materials/vgui/icon/map_selector/Linear.png", layoutDim, layoutDim, pNullImage));
    m_pImageList->SetImageAtIndex(INDX_MAP_NOT_IN_FAVORITES, LoadFileImage("materials/vgui/icon/map_selector/NotInFavorites.png", wide, tall, pNullImage));
    m_pImageList->SetImageAtIndex(INDX_MAP_IN_FAVORITES, LoadFileImage("materials/vgui/icon/map_selector/InFavorites.png", wide, tall, pNullImage));
    m_pImageList->SetImageAtIndex(INDX_MAP_NOT_IN_LIBRARY, LoadFileImage("materials/vgui/icon/map_selector/NotInLibrary.png", wide, tall, pNullImage));
    m_pImageList->SetImageAtIndex(INDX_MAP_IN_LIBRARY, LoadFileImage("materials/vgui/icon/map_selector/InLibrary.png", wide, tall, pNullImage));
    m_pImageList->SetImageAtIndex(INDX_MAP_THUMBNAIL_UNKNOWN, LoadFileImage("materials/vgui/icon/map_selector/invalid_map.png", GetScaledVal(50), GetScaledVal(28), pNullImage));
}

void CMapSelectorDialog::OnMapCacheUpdated(KeyValues *pKv)
{
    // Pass through to the base maps pages
    const auto pMsg = pKv->MakeCopy();
    pMsg->SetName("MapCacheUpdated");
    PostActionSignal(pMsg);
}

void CMapSelectorDialog::OnMapDataUpdated(KeyValues *pKv)
{
    // Map updated from cache, do it here
    uint32 mapID = pKv->GetInt("id");
    UpdateMapListData(mapID, pKv->GetBool("main"), pKv->GetBool("info"),
                      pKv->GetBool("pb"), pKv->GetBool("wr"),
                      pKv->GetBool("thumbnail"));

    // Update the map info dialog as well, if it exists
    const auto indx = m_mapMapInfoDialogs.Find(mapID);
    if (m_mapMapInfoDialogs.IsValidIndex(indx))
        m_mapMapInfoDialogs[indx]->OnMapDataUpdate(pKv);
}

void CMapSelectorDialog::CreateMapListData(MapData* pData)
{
    if (pData)
    {
        MapListData *mapListData = new MapListData;
        mapListData->m_pMapData = pData;
        mapListData->m_pKv = new KeyValues("Map");
        m_mapMapListData.Insert(pData->m_uID, mapListData);
        UpdateMapListData(pData->m_uID, true, true, true, true, true);
    }
}

void CMapSelectorDialog::UpdateMapListData(uint32 uMapID, bool bMain, bool bInfo, bool bPB, bool bWR, bool bThumbnail)
{
    const auto indx = m_mapMapListData.Find(uMapID);
    if (!m_mapMapListData.IsValidIndex(indx))
    {
        CreateMapListData(g_pMapCache->GetMapDataByID(uMapID));
        return;
    }

    MapListData *pMap = m_mapMapListData[indx];
    MapData *pMapData = pMap->m_pMapData;
    KeyValues *pDataKv = pMap->m_pKv;

    if (bMain)
    {
        pDataKv->SetString(KEYNAME_MAP_NAME, pMapData->m_szMapName);
        pDataKv->SetInt(KEYNAME_MAP_ID, pMapData->m_uID);
        pDataKv->SetInt(KEYNAME_MAP_TYPE, pMapData->m_eType);
        pDataKv->SetInt(KEYNAME_MAP_STATUS, pMapData->m_eMapStatus);
        pDataKv->SetInt(KEYNAME_MAP_IN_LIBRARY, pMapData->m_bInLibrary ? INDX_MAP_IN_LIBRARY : INDX_MAP_NOT_IN_LIBRARY);
        pDataKv->SetInt(KEYNAME_MAP_IN_FAVORITES, pMapData->m_bInFavorites ? INDX_MAP_IN_FAVORITES : INDX_MAP_NOT_IN_FAVORITES);

        pDataKv->SetUint64(KEYNAME_MAP_LAST_PLAYED_SORT, pMapData->m_tLastPlayed);
        if (pMapData->m_tLastPlayed > 0)
        {
            char timeAgo[16];
            bool bRes = MomUtil::GetTimeAgoString(&pMapData->m_tLastPlayed, timeAgo, 16);
            pDataKv->SetString(KEYNAME_MAP_LAST_PLAYED, bRes ? timeAgo : "#MOM_NotApplicable");
        }
        else
            pDataKv->SetString(KEYNAME_MAP_LAST_PLAYED, "#MOM_NotApplicable");

        // Set colors based on download state
        if (pMapData->m_bMapFileNeedsUpdate)
        {
            if (!IsMapDownloading(uMapID))
            {
                if (g_pMapCache->IsMapQueuedToDownload(uMapID))
                    pDataKv->SetColor("cellcolor", m_cMapDownloadQueued);
                else
                    pDataKv->SetColor("cellcolor", m_cMapDownloadNeeded);
            }
        }
        else
        {
            KeyValues *pCell = pDataKv->FindKey("cellcolor");
            if (pCell)
            {
                pDataKv->RemoveSubKey(pCell);
                pCell->deleteThis();
            }
        }
    }

    if (bInfo)
    {
        pDataKv->SetInt(KEYNAME_MAP_DIFFICULTY, pMapData->m_MainTrack.m_iDifficulty);
        pDataKv->SetInt(KEYNAME_MAP_LAYOUT, pMapData->m_MainTrack.m_bIsLinear ? INDX_MAP_IS_LINEAR : INDX_MAP_IS_STAGED);

        pDataKv->SetString(KEYNAME_MAP_CREATION_DATE_SORT, pMapData->m_Info.m_szCreationDate);

        time_t creationDateTime;
        if (MomUtil::ISODateToTimeT(pMapData->m_Info.m_szCreationDate, &creationDateTime))
        {
            wchar_t date[32];
            wcsftime(date, 32, L"%b %d, %Y", localtime(&creationDateTime));
            pDataKv->SetWString(KEYNAME_MAP_CREATION_DATE, date);
        }
    }

    if (bPB)
    {
        if (pMapData->m_PersonalBest.m_bValid)
        {
            const auto fRunTime = pMapData->m_PersonalBest.m_Run.m_fTime;
            char szBestTime[BUFSIZETIME];
            MomUtil::FormatTime(fRunTime, szBestTime);

            pDataKv->SetString(KEYNAME_MAP_PERSONAL_BEST, szBestTime);
            pDataKv->SetFloat(KEYNAME_MAP_PERSONAL_BEST_SORT, fRunTime);
        }
        else
        {
            pDataKv->SetString(KEYNAME_MAP_PERSONAL_BEST, "#MOM_NotApplicable");
        }
    }

    if (bWR)
    {
        if (pMapData->m_WorldRecord.m_bValid)
        {
            const auto fRunTime = pMapData->m_WorldRecord.m_Run.m_fTime;
            char szBestTime[BUFSIZETIME];
            MomUtil::FormatTime(fRunTime, szBestTime);

            pDataKv->SetString(KEYNAME_MAP_WORLD_RECORD, szBestTime);
            pDataKv->SetFloat(KEYNAME_MAP_WORLD_RECORD_SORT, fRunTime);
        }
        else
        {
            pDataKv->SetString(KEYNAME_MAP_WORLD_RECORD, "#MOM_NotApplicable");
        }
    }

    if (bThumbnail)
    {
        // Remove the old image if there
        if (pMap->m_pImage)
        {
            delete pMap->m_pImage;
            pMap->m_pImage = nullptr;
        }

        URLImage *pImage = new URLImage(m_pImageList->GetImage(INDX_MAP_THUMBNAIL_UNKNOWN));
        if (pImage->LoadFromURL(pMapData->m_Thumbnail.m_szURLSmall))
        {
            pMap->m_pImage = pImage;

            if (pMap->m_iThumbnailImageIndx >= INDX_RESERVED_COUNT)
            {
                m_pImageList->SetImageAtIndex(pMap->m_iThumbnailImageIndx, pImage);
            }
            else
            {
                // Otherwise just add it
                pMap->m_iThumbnailImageIndx = m_pImageList->AddImage(pImage);
            }
        }
        else
        {
            pMap->m_iThumbnailImageIndx = INDX_MAP_THUMBNAIL_UNKNOWN;
            delete pImage;
        }
    }

    pDataKv->SetInt(KEYNAME_MAP_IMAGE, pMap->m_iThumbnailImageIndx);

    PostActionSignal(new KeyValues("MapListDataUpdate", "id", pMapData->m_uID));
}

MapListData* CMapSelectorDialog::GetMapListDataByID(uint32 uMapID)
{
    const auto indx = m_mapMapListData.Find(uMapID);
    if (m_mapMapListData.IsValidIndex(indx))
        return m_mapMapListData[indx];
    return nullptr;
}

void CMapSelectorDialog::OnMapDownloadQueued(KeyValues* pKv)
{
    const uint32 uID = pKv->GetInt("id");
    UpdateMapListData(uID, true, false, false, false, false);
    UpdateMapInfoDialog(uID);
}

void CMapSelectorDialog::OnMapDownloadStart(KeyValues* pKv)
{
    const uint32 uID = pKv->GetInt("id");

    // First check if we're downloading already
    const auto indx = m_mapMapDownloads.Find(uID);
    if (m_mapMapDownloads.IsValidIndex(indx))
    {
        DevLog("Already downloading!\n");
        return;
    }

    m_mapMapDownloads.Insert(uID, new MapDownloadProgress(pKv->GetString("name")));

    UpdateMapInfoDialog(uID);
}

void CMapSelectorDialog::OnMapDownloadSize(KeyValues* pKv)
{
    const auto indx = m_mapMapDownloads.Find(pKv->GetInt("id"));
    if (m_mapMapDownloads.IsValidIndex(indx))
    {
        m_mapMapDownloads[indx]->SetDownloadSize(pKv->GetUint64("size"));
    }
}

void CMapSelectorDialog::OnMapDownloadProgress(KeyValues* pKv)
{
    const auto indx = m_mapMapDownloads.Find(pKv->GetInt("id"));
    if (m_mapMapDownloads.IsValidIndex(indx))
    {
        m_mapMapDownloads[indx]->SetDownloadProgress(pKv->GetInt("offset") + pKv->GetInt("size"));
    }
    else
    {
        Warning("Map download end with invalid index!\n");
    }
}

void CMapSelectorDialog::OnMapDownloadEnd(KeyValues* pKv)
{
    uint32 uID = pKv->GetInt("id");
    const auto indx = m_mapMapDownloads.Find(uID);
    if (m_mapMapDownloads.IsValidIndex(indx))
    {
        m_mapMapDownloads[indx]->DeletePanel();

        m_mapMapDownloads.RemoveAt(indx);

        UpdateMapInfoDialog(uID);
    }
    else
    {
        Warning("Map download end with invalid index!\n");
    }

    const auto indxCancel = m_mapCancelConfirmDlgs.Find(uID);
    if (m_mapCancelConfirmDlgs.IsValidIndex(indxCancel))
    {
        m_mapCancelConfirmDlgs[indxCancel]->OnCommand("Close");
    }

    const auto pMsg = pKv->MakeCopy();
    pMsg->SetName("MapDownloadEnd");
    PostActionSignal(pMsg);
}

bool CMapSelectorDialog::IsMapDownloading(uint32 uMapID) const
{
    return m_mapMapDownloads.IsValidIndex(m_mapMapDownloads.Find(uMapID));
}

MapDownloadProgress* CMapSelectorDialog::GetDownloadProgressPanel(uint32 uMapID)
{
    const auto indx = m_mapMapDownloads.Find(uMapID);
    if (m_mapMapDownloads.IsValidIndex(indx))
    {
        return m_mapMapDownloads[indx];
    }
    return nullptr;
}

void CMapSelectorDialog::OnStartMapDownload(int id)
{
    const auto response = g_pMapCache->DownloadMap(id);
    if (response != MAP_DL_OK)
    {
        if (response == MAP_DL_WILL_OVERWRITE_EXISTING)
        {
            const auto index = m_mapOverwriteConfirmDlgs.Find(id);
            if (m_mapOverwriteConfirmDlgs.IsValidIndex(index))
            {
                m_mapOverwriteConfirmDlgs[index]->MoveToFront();
                m_mapOverwriteConfirmDlgs[index]->RequestFocus();
            }
            else
            {
                const auto pPanel = messageboxpanel->CreateConfirmationBox(this, "#MOM_MapSelector_ConfirmOverwrite", 
                                                                           "#MOM_MapSelector_ConfirmOverwriteMsg",
                                                       new KeyValues("ConfirmOverwrite", "id", id),
                                                       new KeyValues("RejectOverwrite", "id", id),
                                                       "#GameUI_Yes", "#GameUI_No");
                m_mapOverwriteConfirmDlgs.Insert(id, pPanel);
            }
        }
        else if (response == MAP_DL_FAIL)
        {
            Warning("Failed to download map with ID %i!\n", id);
        }
    }
}

void CMapSelectorDialog::OnRemoveFromQueue(int id)
{
    g_pMapCache->RemoveMapFromDownloadQueue(id);
}

void CMapSelectorDialog::OnCancelMapDownload(int id)
{
    if (ConVarRef("mom_map_download_cancel_confirm").GetBool())
    {
        const auto indx = m_mapCancelConfirmDlgs.Find(id);
        if (!m_mapCancelConfirmDlgs.IsValidIndex(indx))
        {
            Panel *pConfirm = messageboxpanel->CreateConfirmationBox(this, "#MOM_MapSelector_ConfirmCancel", "#MOM_MapSelector_ConfirmCancelMsg",
                                                                     new KeyValues("ConfirmCancelDownload", "id", id),
                                                                     new KeyValues("RejectCancelDownload", "id", id),
                                                                     "#GameUI_Yes", "#GameUI_No");
            m_mapCancelConfirmDlgs.Insert(id, pConfirm);
        }
    }
    else
        g_pMapCache->CancelDownload(id);
}

void CMapSelectorDialog::OnConfirmCancelMapDownload(int id)
{
    m_mapCancelConfirmDlgs.RemoveAt(m_mapCancelConfirmDlgs.Find(id));
    g_pMapCache->CancelDownload(id);
}

void CMapSelectorDialog::OnRejectCancelMapDownload(int id)
{
    m_mapCancelConfirmDlgs.RemoveAt(m_mapCancelConfirmDlgs.Find(id));
}

void CMapSelectorDialog::OnConfirmOverwrite(int id)
{
    m_mapOverwriteConfirmDlgs.RemoveAt(m_mapOverwriteConfirmDlgs.Find(id));
    const auto resp = g_pMapCache->DownloadMap(id, true);
    if (resp == MAP_DL_FAIL)
    {
        Warning("Failed to download map with ID %i!\n", id);
    }
}

void CMapSelectorDialog::OnRejectOverwrite(int id)
{
    m_mapOverwriteConfirmDlgs.RemoveAt(m_mapOverwriteConfirmDlgs.Find(id));
    Msg("Rejected overwrite for map ID %i\n", id);
}

void CMapSelectorDialog::OnAddMapToFavorites(int id)
{
    g_pMapCache->AddMapToFavorites(id);
}

void CMapSelectorDialog::OnAddMapToLibrary(int id)
{
    g_pMapCache->AddMapToLibrary(id);
}

void CMapSelectorDialog::OnMapStart(int id)
{
    if (g_pMapCache->PlayMap(id))
    {
        CloseMapInfoDialog(id);
    }
}

void CMapSelectorDialog::OnRemoveMapFromFavorites(int id)
{
    g_pMapCache->RemoveMapFromFavorites(id);
}

void CMapSelectorDialog::OnRemoveMapFromLibrary(int id)
{
    g_pMapCache->RemoveMapFromLibrary(id);
}

void CMapSelectorDialog::OnRefreshMapInfo(int id)
{
    g_pMapCache->UpdateMapInfo(id);
}

void CMapSelectorDialog::OnViewMapInfo(int id)
{
    // get the map
    MapData *pMapData = g_pMapCache->GetMapDataByID(id);
    if (!pMapData)
        return;

    // View the map info
    OpenMapInfoDialog(pMapData);
}

void CMapSelectorDialog::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_cMapDownloadQueued = pScheme->GetColor("MapList.DownloadQueued", COLOR_BLUE);
    m_cMapDownloadNeeded = pScheme->GetColor("MapList.DownloadNeeded", Color(153, 204, 255, 255));

    // Reload them
    for (int i = 0; i < m_pImageList->GetImageCount(); i++)
    {
        IImage *pImage = m_pImageList->GetImage(i);
        if (pImage)
            pImage->Evict();
    }
}

void CMapSelectorDialog::OnReloadControls()
{
    BaseClass::OnReloadControls();

    MoveToCenterOfScreen();
}


//-----------------------------------------------------------------------------
// Purpose: Updates when the tabs are changed
//-----------------------------------------------------------------------------
void CMapSelectorDialog::OnTabChanged()
{
    m_pCurrentMapList = dynamic_cast<IMapList *>(m_pTabPanel->GetActivePage());

    if (IsVisible())
    {
        m_pCurrentMapList->LoadFilters();
        m_pCurrentMapList->OnTabSelected();
    }

    InvalidateLayout();
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMapContextMenu *CMapSelectorDialog::GetContextMenu()
{
    // create a drop down for this object's states
    if (m_pContextMenu)
        delete m_pContextMenu;
    m_pContextMenu = new CMapContextMenu(this);
    m_pContextMenu->SetAutoDelete(false);
    m_pContextMenu->SetVisible(false);
    return m_pContextMenu;
}

//-----------------------------------------------------------------------------
// Purpose: opens a game info dialog from a game list
//-----------------------------------------------------------------------------
void CMapSelectorDialog::OpenMapInfoDialog(MapData *pMapData)
{
    const auto indx = m_mapMapInfoDialogs.Find(pMapData->m_uID);
    if (m_mapMapInfoDialogs.IsValidIndex(indx))
    {
        // Just bring the opened one to front
        m_mapMapInfoDialogs[indx]->MoveToFront();
    }
    else
    {
        // Add a new one
        CDialogMapInfo *gameDialog = new CDialogMapInfo(this, pMapData);
        gameDialog->AddActionSignalTarget(this);
        gameDialog->Run();
        const auto newIndx = m_mapMapInfoDialogs.Insert(pMapData->m_uID);
        m_mapMapInfoDialogs[newIndx] = gameDialog;
    }
}

void CMapSelectorDialog::UpdateMapInfoDialog(uint32 uMapID)
{
    const auto indx = m_mapMapInfoDialogs.Find(uMapID);
    if (m_mapMapInfoDialogs.IsValidIndex(indx))
    {
        m_mapMapInfoDialogs[indx]->UpdateMapDownloadState();
    }
}

inline void CloseInfoDialog(Panel *pDlg)
{
    if (pDlg)
        ivgui()->PostMessage(pDlg->GetVPanel(), new KeyValues("Close"), NULL);
}

//-----------------------------------------------------------------------------
// Purpose: closes all the game info dialogs
//-----------------------------------------------------------------------------
void CMapSelectorDialog::CloseAllMapInfoDialogs()
{
    FOR_EACH_MAP_FAST(m_mapMapInfoDialogs, i)
    {
        Panel *pDlg = m_mapMapInfoDialogs[i];
        CloseInfoDialog(pDlg);
    }
}

void CMapSelectorDialog::CloseMapInfoDialog(uint32 uMapID)
{
    const auto indx = m_mapMapInfoDialogs.Find(uMapID);
    if (m_mapMapInfoDialogs.IsValidIndex(indx))
    {
        Panel *pDlg = m_mapMapInfoDialogs[indx];
        CloseInfoDialog(pDlg);
    }
}

void CMapSelectorDialog::RemoveMapInfoDialog(uint32 uMapID)
{
    m_mapMapInfoDialogs.RemoveAt(m_mapMapInfoDialogs.Find(uMapID));
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

int CMapSelectorDialog::GetFilteredItemsCount() 
{ 
     if (m_pCurrentMapList)
        return m_pCurrentMapList->GetFilteredItemsCount();
    return 0; 
}

void CMapSelectorDialog::StartRandomMapFromCurrentTab()
{
    if (m_pCurrentMapList)
        m_pCurrentMapList->StartRandomMap();
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
