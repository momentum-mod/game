#include "cbase.h"

#include "BaseMapsPage.h"
#include "CMapListPanel.h"
#include "MapContextMenu.h"
#include "MapFilterPanel.h"
#include "MapSelectorDialog.h"
#include "mom_map_cache.h"

#include <ctime>
#include "fmtstr.h"

#include "vgui/ILocalize.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImageList.h"
#include "vgui_controls/ListPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

// Sort functions
static int __cdecl MapNameSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1,
                                   const vgui::ListPanelItem &item2)
{
    const char *string1 = item1.kv->GetString(KEYNAME_MAP_NAME);
    const char *string2 = item2.kv->GetString(KEYNAME_MAP_NAME);
    return Q_stricmp(string1, string2);
}

static int __cdecl MapPersonalBestSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1,
                                           const vgui::ListPanelItem &item2)
{
    const auto left = item1.kv->GetFloat(KEYNAME_MAP_PERSONAL_BEST_SORT);
    const auto right = item2.kv->GetFloat(KEYNAME_MAP_PERSONAL_BEST_SORT);
    if (left < right)
        return -1;
    if (right < left)
        return 1;
    return 0;
}

static int __cdecl MapWorldRecordSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1,
                                          const vgui::ListPanelItem &item2)
{
    const auto left = item1.kv->GetFloat(KEYNAME_MAP_WORLD_RECORD_SORT);
    const auto right = item2.kv->GetFloat(KEYNAME_MAP_WORLD_RECORD_SORT);
    if (left < right)
        return -1;
    if (right < left)
        return 1;
    return 0;
}

static int __cdecl MapLayoutSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1,
                                     const vgui::ListPanelItem &item2)
{
    const char *i1 = item1.kv->GetString(KEYNAME_MAP_LAYOUT);
    const char *i2 = item2.kv->GetString(KEYNAME_MAP_LAYOUT);
    return Q_stricmp(i1, i2);
}

static int __cdecl MapCreationDateSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1,
                                           const vgui::ListPanelItem &item2)
{
    const char *i1 = item1.kv->GetString(KEYNAME_MAP_CREATION_DATE_SORT);
    const char *i2 = item2.kv->GetString(KEYNAME_MAP_CREATION_DATE_SORT);
    return Q_stricmp(i1, i2);
}

static int __cdecl MapLastPlayedSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &item1,
                                         const vgui::ListPanelItem &item2)
{
    uint64 left = item1.kv->GetUint64(KEYNAME_MAP_LAST_PLAYED_SORT);
    uint64 right = item2.kv->GetUint64(KEYNAME_MAP_LAST_PLAYED_SORT);
    if (left < right)
        return -1;
    if (left == right)
        return 0;
    return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBaseMapsPage::CBaseMapsPage(vgui::Panel *parent, const char *name) : PropertyPage(parent, name)
{
    SetDefLessFunc(m_mapMaps);

    int pWide, pTall;
    parent->GetSize(pWide, pTall);
    SetSize(pWide, pTall);

    m_hFont = INVALID_FONT;
    parent->AddActionSignalTarget(this);

    // Init UI
    m_pMapList = new CMapListPanel(this, "MapList");
    m_pMapList->SetAllowUserModificationOfColumns(true);
    m_pMapList->SetMultiselectEnabled(false);
    m_pMapList->SetShouldCenterEmptyListText(true);
    m_pMapList->SetAutoResize(PIN_TOPLEFT, AUTORESIZE_DOWNANDRIGHT, 0, 0, 0, 0);
    m_pMapList->CalculateAutoResize(pWide, pTall);

    // Images
    m_pMapList->SetImageList(MapSelectorDialog().GetImageList(), false);

    // Add the column headers
    m_pMapList->AddColumnHeader(HEADER_MAP_IMAGE, KEYNAME_MAP_IMAGE, "",
								GetScaledVal(50), GetScaledVal(50), GetScaledVal(50),
                                ListPanel::COLUMN_IMAGE| ListPanel::COLUMN_IMAGE_SIZETOFIT);
    m_pMapList->AddColumnHeader(HEADER_MAP_IN_LIBRARY, KEYNAME_MAP_IN_LIBRARY, "", GetScaledVal(HEADER_ICON_SIZE),
                                GetScaledVal(HEADER_ICON_SIZE), GetScaledVal(HEADER_ICON_SIZE),
                                ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO);
    m_pMapList->AddColumnHeader(HEADER_MAP_IN_FAVORITES, KEYNAME_MAP_IN_FAVORITES, "", GetScaledVal(HEADER_ICON_SIZE),
                                GetScaledVal(HEADER_ICON_SIZE), GetScaledVal(HEADER_ICON_SIZE),
                                ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO);
    m_pMapList->AddColumnHeader(HEADER_MAP_NAME, KEYNAME_MAP_NAME, "#MOM_MapSelector_Maps",
								GetScaledVal(100), GetScaledVal(50), GetScaledVal(1000),
                                ListPanel::COLUMN_UNHIDABLE | ListPanel::COLUMN_RESIZEWITHWINDOW);
    m_pMapList->AddColumnHeader(HEADER_MAP_LAYOUT, KEYNAME_MAP_LAYOUT, "#MOM_MapSelector_MapLayout",
								GetScaledVal(30), GetScaledVal(30), GetScaledVal(30),
								ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_IMAGE_SIZETOFIT |
								ListPanel::COLUMN_RESIZEWITHWINDOW | ListPanel::COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO);
    m_pMapList->AddColumnHeader(HEADER_DIFFICULTY, KEYNAME_MAP_DIFFICULTY, "#MOM_MapSelector_Difficulty",
                                GetScaledVal(20), GetScaledVal(20), GetScaledVal(20), 0);
    m_pMapList->AddColumnHeader(HEADER_WORLD_RECORD, KEYNAME_MAP_WORLD_RECORD, "#MOM_WorldRecord",
								GetScaledVal(70), 0, GetScaledVal(100), ListPanel::COLUMN_RESIZEWITHWINDOW);
    m_pMapList->AddColumnHeader(HEADER_BEST_TIME, KEYNAME_MAP_PERSONAL_BEST, "#MOM_PersonalBest",
								GetScaledVal(70), 0, GetScaledVal(100), ListPanel::COLUMN_RESIZEWITHWINDOW);
    m_pMapList->AddColumnHeader(HEADER_DATE_CREATED, KEYNAME_MAP_CREATION_DATE, "#MOM_MapSelector_CreationDate",
                                GetScaledVal(65), 0, GetScaledVal(100), 0);
    m_pMapList->AddColumnHeader(HEADER_LAST_PLAYED, KEYNAME_MAP_LAST_PLAYED, "#MOM_MapSelector_LastPlayed",
                                GetScaledVal(1), GetScaledVal(1), 9001, 0);

    // Images happen in ApplySchemeSettings

    // Tooltips
    m_pMapList->SetColumnHeaderTooltip(HEADER_MAP_LAYOUT, "#MOM_MapSelector_MapLayout_Tooltip");
    m_pMapList->SetColumnHeaderTooltip(HEADER_MAP_IN_LIBRARY, "#MOM_MapSelector_Library_Tooltip");
    m_pMapList->SetColumnHeaderTooltip(HEADER_MAP_IN_FAVORITES, "#MOM_MapSelector_Favorites_Tooltip");

    // Alignment
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_MAP_LAYOUT, Label::a_center);
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_DIFFICULTY, Label::a_center);
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_MAP_IN_LIBRARY, Label::a_center);
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_MAP_IN_FAVORITES, Label::a_center);
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_WORLD_RECORD, Label::a_center);
    m_pMapList->SetColumnHeaderTextAlignment(HEADER_BEST_TIME, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_LAYOUT, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_DIFFICULTY, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_IMAGE, Label::a_northwest);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_IN_LIBRARY, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_MAP_IN_FAVORITES, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_WORLD_RECORD, Label::a_center);
    m_pMapList->SetColumnTextAlignment(HEADER_BEST_TIME, Label::a_center);

    // Sort Functions
    m_pMapList->SetSortFunc(HEADER_MAP_NAME, MapNameSortFunc);
    m_pMapList->SetSortFunc(HEADER_WORLD_RECORD, MapWorldRecordSortFunc);
    m_pMapList->SetSortFunc(HEADER_BEST_TIME, MapPersonalBestSortFunc);
    m_pMapList->SetSortFunc(HEADER_MAP_LAYOUT, MapLayoutSortFunc);
    m_pMapList->SetSortFunc(HEADER_DATE_CREATED, MapCreationDateSortFunc);
    m_pMapList->SetSortFunc(HEADER_LAST_PLAYED, MapLastPlayedSortFunc);

    // disable sort for certain columns
    m_pMapList->SetColumnSortable(HEADER_MAP_IMAGE, false);

    // Sort by map name by default
    m_pMapList->SetSortColumn(HEADER_MAP_NAME);

    m_pMapList->MakeReadyForUse();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBaseMapsPage::~CBaseMapsPage() {}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CBaseMapsPage::GetInvalidMapListID() { return m_pMapList->InvalidItemID(); }

MapDisplay_t *CBaseMapsPage::GetMapDisplayByID(uint32 id)
{
    if (m_mapMaps.Count() == 0)
        return nullptr;

    const auto indx = m_mapMaps.Find(id);
    if (m_mapMaps.IsValidIndex(indx))
        return &m_mapMaps[indx];

    return nullptr;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CBaseMapsPage::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    // Apply images
    if (m_pMapList->GetImageList()->GetImageCount() > 1)
    {
        m_pMapList->SetColumnHeaderImage(HEADER_MAP_IN_LIBRARY, INDX_MAP_IN_LIBRARY);
        m_pMapList->SetColumnHeaderImage(HEADER_MAP_IN_FAVORITES, INDX_MAP_IN_FAVORITES);
    }

    // Font
    m_hFont = pScheme->GetFont("MapListFont", IsProportional());
    if (m_hFont == INVALID_FONT)
        m_hFont = pScheme->GetFont("DefaultSmall", IsProportional());
    m_pMapList->SetFont(m_hFont);

    m_cMapDLFailed = pScheme->GetColor("MapList.DownloadFailColor", COLOR_RED);
    m_cMapDLSuccess = pScheme->GetColor("MapList.DownloadSuccessColor", COLOR_GREEN);
}

void CBaseMapsPage::SetListCellColors(MapData *pData, KeyValues *pKvInto)
{
    KeyValues *pCellColor = new KeyValues("cellcolor");
    // KeyValues *pCellBGColor = new KeyValues("cellbgcolor");
    KeyValues *pSub = pCellColor->CreateNewKey();
    pSub->SetName(CFmtStr("%i", HEADER_MAP_NAME));
    pSub->SetColor("color", pData->m_bInLibrary ? COLOR_BLUE : COLOR_WHITE);
    // pCellBGColor->AddSubKey(pSub->MakeCopy());
    pKvInto->AddSubKey(pCellColor);
    // pKvInto->AddSubKey(pCellBGColor);
}

//-----------------------------------------------------------------------------
// Purpose: loads filter settings (from disk) from the keyvalues
//-----------------------------------------------------------------------------
void CBaseMapsPage::LoadFilters() { MapSelectorDialog().LoadTabFilterData(GetName()); }

//-----------------------------------------------------------------------------
// Purpose: applies only the game filter to the current list
//-----------------------------------------------------------------------------
void CBaseMapsPage::ApplyFilters(MapFilters_t filters) { OnApplyFilters(filters); }

void CBaseMapsPage::OnApplyFilters(MapFilters_t filters)
{
    // loop through all the maps checking filters
    FOR_EACH_MAP_FAST(m_mapMaps, i)
    {
        MapDisplay_t *pMap = &m_mapMaps[i];
        // Now we can check the filters
        if (!MapPassesFilters(pMap->m_pMap, filters))
        {
            m_pMapList->SetItemVisible(pMap->m_iListID, false);
            pMap->m_bNeedsShown = true;
        }
        else if (pMap->m_bNeedsShown)
        {
            m_pMapList->SetItemVisible(pMap->m_iListID, true);
            pMap->m_bNeedsShown = false;
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
        bPassesLower = pMap->m_MainTrack.m_iDifficulty >= iDiffLowBound;
    }
    if (iDiffHighBound)
    {
        bPassesHigher = pMap->m_MainTrack.m_iDifficulty <= iDiffHighBound;
    }
    if (!(bPassesLower && bPassesHigher))
        return false;

    // Game mode (if it's a surf/bhop/etc map or not)
    int iGameModeFilter = filters.m_iGameMode;
    if (iGameModeFilter && iGameModeFilter != pMap->m_eType)
        return false;

    bool bHideCompleted = filters.m_bHideCompleted;
    if (bHideCompleted && pMap->m_PersonalBest.m_bValid)
        return false;

    // Map layout (0 = all, 1 = show staged maps only, 2 = show linear maps only)
    int iMapLayoutFilter = filters.m_iMapLayout;
    if (iMapLayoutFilter && pMap->m_MainTrack.m_bIsLinear + 1 != iMapLayoutFilter)
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
        m_pMapList->SetColumnHeaderText(
            HEADER_MAP_NAME,
            CConstructLocalizedString(g_pVGuiLocalize->Find("#MOM_MapSelector_MapCount"), m_pMapList->GetItemCount()));
    }
    else
    {
        m_pMapList->SetColumnHeaderText(HEADER_MAP_NAME, g_pVGuiLocalize->Find("#MOM_MapSelector_Maps"));
        m_pMapList->SetEmptyListText("#MOM_MapSelector_NoMaps");
    }
}

void CBaseMapsPage::AddMapToList(MapData *pData)
{
    // Only add it if it doesn't exist already
    // Updates are handled by an event
    MapDisplay_t *pFound = GetMapDisplayByID(pData->m_uID);
    if (pFound)
        return;

    MapDisplay_t map;
    map.m_pMap = pData;
    map.m_bNeedsShown = true;
    m_mapMaps.Insert(pData->m_uID, map);

    // Add the map to the m_pMapList
    OnMapListDataUpdate(pData->m_uID);

    UpdateStatus();
}

uint32 CBaseMapsPage::TryStartMapFromRow(int itemID)
{
    const auto iMapID = m_pMapList->GetItemUserData(itemID);
    if (iMapID == 0)
        return 0;

    const auto pMapData = g_pMapCache->GetMapDataByID(iMapID);
    if (!pMapData)
        return 0;

    if (pMapData->m_bInLibrary)
    {
        if (pMapData->m_bMapFileNeedsUpdate)
        {
            MapSelectorDialog().OnStartMapDownload(iMapID);
        }
        else
        {
            MapSelectorDialog().OnMapStart(iMapID);
        }
    }
    else
    {
        MapSelectorDialog().OnAddMapToLibrary(iMapID);
    }

    return iMapID;
}

void CBaseMapsPage::OnMapListDataUpdate(int mapID)
{
    const auto pMapDisplay = GetMapDisplayByID(mapID);

    if (!pMapDisplay)
        return;

    if (m_pMapList->IsValidItemID(pMapDisplay->m_iListID))
    {
        m_pMapList->ApplyItemChanges(pMapDisplay->m_iListID);
    }
    else
    {
        // Otherwise we need to add it
        const auto pData = MapSelectorDialog().GetMapListDataByID(mapID);
        if (pData)
        {
            pMapDisplay->m_iListID = m_pMapList->AddItem(pData->m_pKv, mapID, false, false, false);
        }
        else
        {
            MapSelectorDialog().CreateMapListData(pMapDisplay->m_pMap);
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
void CBaseMapsPage::OnItemSelected() {}

void CBaseMapsPage::OnMapDownloadEnd(KeyValues *pKv)
{
    uint32 id = pKv->GetInt("id");
    MapDisplay_t *map = GetMapDisplayByID(id);
    if (map)
    {
        KeyValues *pKvInto = m_pMapList->GetItem(map->m_iListID);
        pKvInto->SetColor("cellcolor", pKv->GetBool("error") ? m_cMapDLFailed : m_cMapDLSuccess);
        m_pMapList->ApplyItemChanges(map->m_iListID);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle enter pressed in the games list page. Return true
// to intercept the message instead of passing it on through vgui.
//-----------------------------------------------------------------------------
bool CBaseMapsPage::OnGameListEnterPressed()
{
    if (GetSelectedItemsCount() == 0)
        return false;

    return TryStartMapFromRow(m_pMapList->GetSelectedItem(0)) != 0;
}

int CBaseMapsPage::GetFilteredItemsCount() 
{ 
    return m_pMapList->GetItemCount(); 
}

void CBaseMapsPage::StartRandomMap()
{
    const auto iVisibleItemsCount = GetFilteredItemsCount();

    if (iVisibleItemsCount == 0)
        return;

    if (MapSelectorDialog().GetMapToStart() > 0)
        return;

    const auto iListID = m_pMapList->GetItemIDFromRow(RandomInt(0, iVisibleItemsCount - 1));
    const auto iMapID = m_pMapList->GetItemUserData(iListID);
    if (iMapID == 0)
        return;

    MapSelectorDialog().SetMapToStart(iMapID);

    TryStartMapFromRow(iListID);
}

//-----------------------------------------------------------------------------
// Purpose: Get the # items selected in the game list.
//-----------------------------------------------------------------------------
int CBaseMapsPage::GetSelectedItemsCount() { return m_pMapList->GetSelectedItemsCount(); }

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
        // find the row in the list and kill
        m_pMapList->RemoveItem(map.m_iListID);
        m_mapMaps.Remove(map.m_pMap->m_uID);
    }

    UpdateStatus();
}

//-----------------------------------------------------------------------------
// Purpose: Remove all the maps we currently have
//-----------------------------------------------------------------------------
void CBaseMapsPage::ClearMapList()
{
    m_mapMaps.RemoveAll();
    m_pMapList->RemoveAll();
}

void CBaseMapsPage::OnTabSelected() { GetNewMapList(); }

void CBaseMapsPage::GetNewMapList()
{
    CUtlVector<MapData *> vecMaps;
    g_pMapCache->GetMapList(vecMaps, GetMapListType());

    FOR_EACH_VEC(vecMaps, i)
    {
        AddMapToList(vecMaps[i]);
    }

    OnGetNewMapList();
}

void CBaseMapsPage::OnGetNewMapList() { ApplyFilters(GetFilters()); }

void CBaseMapsPage::OnOpenContextMenu(int itemID)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    uint32 uMapID = m_pMapList->GetItemUserData(itemID);
    if (uMapID == 0)
        return;

    MapData *pMapData = g_pMapCache->GetMapDataByID(uMapID);
    if (!pMapData)
        return;

    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu();
    menu->ShowMenu(pMapData);
}
