#include "cbase.h"

#include "BaseMapsPage.h"
#include "CMapListPanel.h"
#include "MapSelectorDialog.h"
#include "MapFilterPanel.h"
#include "MapContextMenu.h"
#include "mom_map_cache.h"
#include "util/mom_util.h"

#include "fmtstr.h"
#include <ctime>

#include "vgui/ILocalize.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImageList.h"
#include "vgui_controls/ComboBox.h"
#include "controls/FileImage.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//Sort functions
static int __cdecl MapNameSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *string1 = item1.kv->GetString(KEYNAME_MAP_NAME);
    const char *string2 = item2.kv->GetString(KEYNAME_MAP_NAME);
    return Q_stricmp(string1, string2);
}

static int __cdecl MapCompletedSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *string1 = item1.kv->GetString(KEYNAME_MAP_TIME);
    const char *string2 = item2.kv->GetString(KEYNAME_MAP_TIME);
    return Q_stricmp(string1, string2);
}

static int __cdecl MapWorldRecordSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *string1 = item1.kv->GetString(KEYNAME_MAP_WORLD_RECORD);
    const char *string2 = item2.kv->GetString(KEYNAME_MAP_WORLD_RECORD);
    return Q_stricmp(string1, string2);
}

static int __cdecl MapLayoutSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *i1 = item1.kv->GetString(KEYNAME_MAP_LAYOUT);
    const char *i2 = item2.kv->GetString(KEYNAME_MAP_LAYOUT);
    return Q_stricmp(i1, i2);
}

static int __cdecl MapCreationDateSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1, const vgui::ListPanelItem &item2)
{
    const char *i1 = item1.kv->GetString(KEYNAME_MAP_CREATION_DATE_SORT);
    const char *i2 = item2.kv->GetString(KEYNAME_MAP_CREATION_DATE_SORT);
    return Q_stricmp(i1, i2);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBaseMapsPage::CBaseMapsPage(vgui::Panel *parent, const char *name) : PropertyPage(parent, name)
{
    SetSize(664, 294);

    m_hFont = INVALID_FONT;

    // Init UI
    m_pMapList = new CMapListPanel(this, "MapList");
    m_pMapList->SetAllowUserModificationOfColumns(true);
    m_pMapList->SetShouldCenterEmptyListText(true);
    
    // Add the column headers
    m_pMapList->AddColumnHeader(HEADER_MAP_IMAGE, KEYNAME_MAP_IMAGE, "", GetScaledVal(90), GetScaledVal(90), GetScaledVal(120), ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_IMAGE_SIZETOFIT | ListPanel::COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO);
    m_pMapList->AddColumnHeader(HEADER_MAP_IN_LIBRARY, KEYNAME_MAP_IN_LIBRARY, "", GetScaledVal(10), GetScaledVal(10), GetScaledVal(12), ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_IMAGE_SIZETOFIT | ListPanel::COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO);
    m_pMapList->AddColumnHeader(HEADER_MAP_IN_FAVORITES, KEYNAME_MAP_IN_FAVORITES, "", GetScaledVal(10), GetScaledVal(10), GetScaledVal(12), ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_IMAGE_SIZETOFIT | ListPanel::COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO);
    m_pMapList->AddColumnHeader(HEADER_MAP_NAME, KEYNAME_MAP_NAME, "#MOM_MapSelector_Maps", GetScaledVal(150), GetScaledVal(150), 9001, ListPanel::COLUMN_UNHIDABLE | ListPanel::COLUMN_RESIZEWITHWINDOW);
    m_pMapList->AddColumnHeader(HEADER_MAP_LAYOUT, KEYNAME_MAP_LAYOUT, "#MOM_MapSelector_MapLayout", GetScaledVal(75), GetScaledVal(75), GetScaledVal(100), 0);
    m_pMapList->AddColumnHeader(HEADER_DIFFICULTY, KEYNAME_MAP_DIFFICULTY, "#MOM_MapSelector_Difficulty", GetScaledVal(55), GetScaledVal(55), GetScaledVal(100), 0);
    m_pMapList->AddColumnHeader(HEADER_WORLD_RECORD, KEYNAME_MAP_WORLD_RECORD, "#MOM_WorldRecord", GetScaledVal(90), GetScaledVal(90), GetScaledVal(105), 0);
    m_pMapList->AddColumnHeader(HEADER_BEST_TIME, KEYNAME_MAP_TIME, "#MOM_PersonalBest", GetScaledVal(90), GetScaledVal(90), GetScaledVal(105), 0);
    m_pMapList->AddColumnHeader(HEADER_DATE_CREATED, KEYNAME_MAP_CREATION_DATE, "#MOM_MapSelector_CreationDate", GetScaledVal(90), GetScaledVal(90), 9001, ListPanel::COLUMN_RESIZEWITHWINDOW);

    // Tooltips
    //MOM_TODO: do we want tooltips?

    // Alignment
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_MAP_LAYOUT, Label::a_center);
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_DIFFICULTY, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_LAYOUT, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_DIFFICULTY, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_IMAGE, Label::a_center);

    // Sort Functions
    m_pMapList->SetSortFunc(HEADER_MAP_NAME, MapNameSortFunc);
    m_pMapList->SetSortFunc(HEADER_WORLD_RECORD, MapWorldRecordSortFunc);
    m_pMapList->SetSortFunc(HEADER_BEST_TIME, MapCompletedSortFunc);
    m_pMapList->SetSortFunc(HEADER_MAP_LAYOUT, MapLayoutSortFunc);
    m_pMapList->SetSortFunc(HEADER_DATE_CREATED, MapCreationDateSortFunc);

    // disable sort for certain columns
    m_pMapList->SetColumnSortable(HEADER_MAP_IMAGE, false);

    // Sort by map name by default
    m_pMapList->SetSortColumn(HEADER_MAP_NAME);

    LoadControlSettings(CFmtStr("resource/ui/MapSelector/%sPage.res", name));

    ListenForGameEvent("map_data_update");
    ListenForGameEvent("map_cache_updated");
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

MapDisplay_t *CBaseMapsPage::GetMapDisplayByID(uint32 id)
{
    if (m_vecMaps.IsEmpty())
        return nullptr;

    FOR_EACH_VEC(m_vecMaps, i)
    {
        if (m_vecMaps[i].m_pMap->m_uID == id)
            return &m_vecMaps[i];
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::PerformLayout()
{
    BaseClass::PerformLayout();
    Repaint();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    // Images
    ImageList *imageList = new ImageList(false);
    imageList->AddImage(scheme()->GetImage("maps/invalid_map", false)); // The ? banner at index 1
    m_pMapList->SetImageList(imageList, true);

    //Font
    m_hFont = pScheme->GetFont("MapListFont", IsProportional());
    if (m_hFont == INVALID_FONT)
        m_hFont = pScheme->GetFont("DefaultSmall", IsProportional());
    m_pMapList->SetFont(m_hFont);
}

void CBaseMapsPage::SetListCellColors(MapData* pData, KeyValues* pKvInto)
{
    KeyValues *pCellColor = new KeyValues("cellcolor");
   // KeyValues *pCellBGColor = new KeyValues("cellbgcolor");
    KeyValues *pSub = pCellColor->CreateNewKey();
    pSub->SetName(CFmtStr("%i", HEADER_MAP_NAME));
    pSub->SetColor("color", pData->m_bInLibrary ? COLOR_BLUE : COLOR_WHITE);
    //pCellBGColor->AddSubKey(pSub->MakeCopy());
    pKvInto->AddSubKey(pCellColor);
    //pKvInto->AddSubKey(pCellBGColor);
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
void CBaseMapsPage::ApplyFilters(MapFilters_t filters)
{
    OnApplyFilters(filters);
}

void CBaseMapsPage::OnApplyFilters(MapFilters_t filters)
{
    // loop through all the maps checking filters
    FOR_EACH_VEC(m_vecMaps, i)
    {
        MapDisplay_t *map = &m_vecMaps[i];

        // Now we can check the filters
        if (!MapPassesFilters(map->m_pMap, filters))
        {
            m_pMapList->SetItemVisible(map->m_iListID, false);
            map->m_bNeedsShown = true;
        }
        else if (map->m_bNeedsShown)
        {
            m_pMapList->SetItemVisible(map->m_iListID, true);
            map->m_bNeedsShown = false;
        }
    }

    UpdateStatus();
    m_pMapList->SortList();
    InvalidateLayout();
    Repaint();
}

bool CBaseMapsPage::MapPassesFilters(MapData *pMap, MapFilters_t filters)
{
    if (!pMap)
        return false;

    // Needs to pass map name filter
    // compare the first few characters of the filter
    const char *szMapNameFilter = filters.m_szMapName;
    int count = Q_strlen(szMapNameFilter);

    // strstr returns null if the substring is not in the base string
    if (count && !Q_strstr(pMap->m_szMapName, szMapNameFilter))
        return false;

    // Difficulty
    int iDiffLowBound = filters.m_iDifficultyLow;
    int iDiffHighBound = filters.m_iDifficultyHigh;
    bool bPassesLower = true;
    bool bPassesHigher = true;
    if (iDiffLowBound)
    {
        bPassesLower = pMap->m_Info.m_iDifficulty >= iDiffLowBound;
    }
    if (iDiffHighBound)
    {
        bPassesHigher = pMap->m_Info.m_iDifficulty <= iDiffHighBound;
    }
    if (!(bPassesLower && bPassesHigher))
        return false;

    //Game mode (if it's a surf/bhop/etc map or not)
    int iGameModeFilter = filters.m_iGameMode;
    if (iGameModeFilter && iGameModeFilter != pMap->m_eType)
        return false;

    bool bHideCompleted = filters.m_bHideCompleted;
    if (bHideCompleted && pMap->m_PersonalBest.m_bValid)
        return false;

    // Map layout (0 = all, 1 = show staged maps only, 2 = show linear maps only)
    int iMapLayoutFilter = filters.m_iMapLayout;
    if (iMapLayoutFilter && pMap->m_Info.m_bIsLinear + 1 == iMapLayoutFilter)
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Resets UI map count
//-----------------------------------------------------------------------------
void CBaseMapsPage::UpdateStatus()
{
    if (m_pMapList->GetItemCount() > 0)
    {
        m_pMapList->SetColumnHeaderText(HEADER_MAP_NAME, CConstructLocalizedString(g_pVGuiLocalize->Find("#MOM_MapSelector_MapCount"), m_pMapList->GetItemCount()));
    }
    else
    {
        m_pMapList->SetColumnHeaderText(HEADER_MAP_NAME, g_pVGuiLocalize->Find("#MOM_MapSelector_Maps"));
        m_pMapList->SetEmptyListText("#MOM_MapSelector_NoMaps");
    }
}

void CBaseMapsPage::AddMapToList(MapData* pData)
{
    // Only add it if it doesn't exist already
    // Updates are handled by an event
    FOR_EACH_VEC(m_vecMaps, i)
    {
        if (m_vecMaps[i].m_pMap->m_uID == pData->m_uID)
            return;
    }

    MapDisplay_t map;
    map.m_pMap = pData;
    map.m_bNeedsShown = true;

    // Add the map to the m_pMapList
    UpdateMapListData(&map, true, true, true, true, true);

    m_vecMaps.AddToTail(map);
}

void CBaseMapsPage::UpdateMapListData(MapDisplay_t *pMap, bool bMain, bool bInfo, bool bPB, bool bWR, bool bThumbnail)
{
    KeyValuesAD kv("Map");
    MapData *pMapData = pMap->m_pMap;
    if (bMain)
    {
        kv->SetString(KEYNAME_MAP_NAME, pMapData->m_szMapName);
        kv->SetInt(KEYNAME_MAP_ID, pMapData->m_uID);
        kv->SetInt(KEYNAME_MAP_TYPE, pMapData->m_eType);
        kv->SetInt(KEYNAME_MAP_STATUS, pMapData->m_eMapStatus);
        kv->SetInt(KEYNAME_MAP_IN_LIBRARY, pMapData->m_bInLibrary);
        kv->SetInt(KEYNAME_MAP_IN_FAVORITES, pMapData->m_bInFavorites);
        // SetListCellColors(pMapData, kv);
    }

    if (bInfo)
    {
        kv->SetInt(KEYNAME_MAP_DIFFICULTY, pMapData->m_Info.m_iDifficulty);
        kv->SetString(KEYNAME_MAP_LAYOUT, pMapData->m_Info.m_bIsLinear ? "LINEAR" : "STAGED");

        kv->SetString(KEYNAME_MAP_CREATION_DATE_SORT, pMapData->m_Info.m_szCreationDate);

        time_t creationDateTime;
        g_pMomentumUtil->ISODateToTimeT(pMapData->m_Info.m_szCreationDate, &creationDateTime);
        char date[32];
        strftime(date, 32, "%b %d, %Y", localtime(&creationDateTime));
        kv->SetString(KEYNAME_MAP_CREATION_DATE, date);
    }

    if (bPB)
    {
        if (pMapData->m_PersonalBest.m_bValid)
        {
            char szBestTime[BUFSIZETIME];
            g_pMomentumUtil->FormatTime(pMapData->m_PersonalBest.m_Run.m_fTime, szBestTime);

            kv->SetString(KEYNAME_MAP_TIME, szBestTime);
        }
        else
        {
            kv->SetString(KEYNAME_MAP_TIME, "#MOM_NotApplicable");
        }
    }

    if (bWR)
    {
        if (pMapData->m_WorldRecord.m_bValid)
        {
            char szBestTime[BUFSIZETIME];
            g_pMomentumUtil->FormatTime(pMapData->m_WorldRecord.m_Run.m_fTime, szBestTime);

            kv->SetString(KEYNAME_MAP_WORLD_RECORD, szBestTime);
        }
        else
        {
            kv->SetString(KEYNAME_MAP_WORLD_RECORD, "#MOM_NotApplicable");
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

        URLImage *pImage = new URLImage;
        if (pImage->LoadFromURL(pMapData->m_Thumbnail.m_szURLSmall))
        {
            pMap->m_pImage = pImage;

            if (pMap->m_iMapImageIndex > 1)
            {
                m_pMapList->GetImageList()->SetImageAtIndex(pMap->m_iMapImageIndex, pImage);
            }
            else
            {
                // Otherwise just add it
                pMap->m_iMapImageIndex = m_pMapList->GetImageList()->AddImage(pImage);
            }
        }
        else
        {
            pMap->m_iMapImageIndex = 1;
            delete pImage;
        }
    }

    kv->SetInt(KEYNAME_MAP_IMAGE, pMap->m_iMapImageIndex);

    KeyValues *pItem = m_pMapList->GetItem(pMap->m_iListID);
    // If it's a valid index, we're just normally updating
    if (pItem)
    {
        kv->RecursiveMergeKeyValues(pItem);
        pItem->Clear();
        kv->CopySubkeys(pItem);
        m_pMapList->ApplyItemChanges(pMap->m_iListID);
    }
    else
    {
        // Otherwise we need to add it
        pMap->m_iListID = m_pMapList->AddItem(kv, NULL, false, false);
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnCommand(const char *command)
{
    if (!Q_stricmp(command, "StartMap"))
    {
        // OnMapStart();
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

void CBaseMapsPage::FireGameEvent(IGameEvent* event)
{
    if (FStrEq(event->GetName(), "map_data_update"))
    {
        // Map updated from cache, do it here
        uint32 id = event->GetInt("id");
        MapDisplay_t *map = GetMapDisplayByID(id);
        if (map)
        {
            UpdateMapListData(map, event->GetBool("main"), event->GetBool("info"), 
                              event->GetBool("pb"), event->GetBool("wr"), 
                              event->GetBool("thumbnail"));
        }
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

MapFilters_t CBaseMapsPage::GetFilters()
{
    KeyValues *pKv = MapSelectorDialog().GetTabFilterData(GetName());
    MapFilters_t filters;
    filters.FromKV(pKv);
    return filters;
}

//-----------------------------------------------------------------------------
// Purpose: removes the server from the UI list
//-----------------------------------------------------------------------------
void CBaseMapsPage::RemoveMap(MapDisplay_t &map)
{
    if (m_pMapList->IsValidItemID(map.m_iListID))
    {
        // Delete its image to free resources
        if (map.m_iMapImageIndex > 1)
            delete map.m_pImage;

        // find the row in the list and kill
        m_pMapList->RemoveItem(map.m_iListID);
        m_vecMaps.FindAndRemove(map);
    }

    UpdateStatus();
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
    // StartRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnPageShow()
{
    // StartRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: Called on page hide, stops any refresh
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnPageHide()
{
    // StopRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: initiates map loading
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnMapStart(int id)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    g_pMapCache->PlayMap(id); // MOM_TODO handle downloads
}

//-----------------------------------------------------------------------------
// Purpose: Displays the current map info (from contextmenu)
//-----------------------------------------------------------------------------
void CBaseMapsPage::OnViewMapInfo(int id)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    // get the map
    MapData *pMapData = g_pMapCache->GetMapDataByID(id);
    if (!pMapData)
        return;

    // View the map info
    MapSelectorDialog().OpenMapInfoDialog(this, pMapData);
}

void CBaseMapsPage::OnAddMapToLibrary(int id)
{
    g_pMapCache->AddMapToLibrary(id);
}

void CBaseMapsPage::OnRemoveMapFromLibrary(int id)
{
    g_pMapCache->RemoveMapFromLibrary(id);
}

void CBaseMapsPage::OnAddMapToFavorites(int id)
{
    g_pMapCache->AddMapToFavorites(id); 
}

void CBaseMapsPage::OnRemoveMapFromFavorites(int id)
{
    g_pMapCache->RemoveMapFromFavorites(id);
}

void CBaseMapsPage::OnOpenContextMenu(int itemID)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    KeyValues *pData = m_pMapList->GetItem(itemID);
    if (!pData)
        return;

    MapData *pMapData = g_pMapCache->GetMapDataByID(pData->GetInt(KEYNAME_MAP_ID));
    if (!pMapData)
        return;

    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu(m_pMapList);
    menu->ShowMenu(this, pMapData);
}