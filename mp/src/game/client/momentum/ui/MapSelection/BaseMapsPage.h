//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once

#include "IMapList.h"
#include "vgui_controls/PropertyPage.h"

class MapFilterPanel;
class CMapListPanel;
struct MapData;

// Map keynames
#define KEYNAME_MAP_ID                  "id"
#define KEYNAME_MAP_NAME                "name"
#define KEYNAME_MAP_HASH                "hash"
#define KEYNAME_MAP_PERSONAL_BEST       "time"
#define KEYNAME_MAP_PERSONAL_BEST_SORT  "time_s"
#define KEYNAME_MAP_TYPE                "MapType"
#define KEYNAME_MAP_STATUS              "MapStatus"
#define KEYNAME_MAP_ZONE_COUNT          "numZones"
#define KEYNAME_MAP_LAYOUT              "MapLayout"
#define KEYNAME_MAP_DIFFICULTY          "difficulty"
#define KEYNAME_MAP_WORLD_RECORD        "WorldRecord"
#define KEYNAME_MAP_WORLD_RECORD_SORT   "WorldRecord_s"
#define KEYNAME_MAP_IMAGE               "MapImage"
#define KEYNAME_MAP_PATH                "MapPath"
#define KEYNAME_MAP_CREATION_DATE       "creationDate"
#define KEYNAME_MAP_CREATION_DATE_SORT  "creationDate_s"
#define KEYNAME_MAP_IN_LIBRARY          "inLibrary"
#define KEYNAME_MAP_IN_FAVORITES        "inFavorites"
#define KEYNAME_MAP_LAST_PLAYED         "lastPlayed"
#define KEYNAME_MAP_LAST_PLAYED_SORT    "lastPlayed_s"

enum HEADERS
{
    HEADER_MAP_IMAGE = 0,
    HEADER_MAP_IN_LIBRARY,
    HEADER_MAP_IN_FAVORITES,
    HEADER_MAP_NAME,
    HEADER_MAP_LAYOUT,
    HEADER_DIFFICULTY,
    HEADER_WORLD_RECORD,
    HEADER_BEST_TIME,
    HEADER_DATE_CREATED,
    HEADER_LAST_PLAYED,
};

//-----------------------------------------------------------------------------
// Purpose: Base property page for all the games lists (internet/favorites/lan/etc.)
//-----------------------------------------------------------------------------
class CBaseMapsPage : public vgui::PropertyPage, public IMapList
{
    DECLARE_CLASS_SIMPLE(CBaseMapsPage, vgui::PropertyPage);

public:
    CBaseMapsPage(Panel *parent, const char *name);
    ~CBaseMapsPage();

    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

    virtual void SetListCellColors(MapData *pData, KeyValues *pKvInto);

    // Called by CGameList when the enter key is pressed.
    // This is overridden in the add server dialog - since there is no Connect button, the message
    // never gets handled, but we want to add a server when they dbl-click or press enter.
    virtual bool OnGameListEnterPressed();

    int GetSelectedItemsCount();

    // Filters
    // loads filter settings from disk
    virtual void LoadFilters();
    virtual MapFilters_t GetFilters();
    void ApplyFilters(MapFilters_t filters) OVERRIDE;
    virtual void OnApplyFilters(MapFilters_t filters);
    virtual bool MapPassesFilters(MapData *pData, MapFilters_t filters);

    // Called when the Feeling Lucky button is pressed
    virtual void StartRandomMap() OVERRIDE;
    virtual int GetFilteredItemsCount() OVERRIDE;

    // Called when the map selector has a map updated in its list
    MESSAGE_FUNC_INT(OnMapListDataUpdate, "MapListDataUpdate", id);
    // Called when the map selector opens
    MESSAGE_FUNC(OnMapSelectorOpened, "MapSelectorOpened") { OnTabSelected(); }
    // Right clicking a map
    MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);
    MESSAGE_FUNC(OnItemSelected, "ItemSelected");

    // Modulecomm events passed in through the MapSelectorDialog
    MESSAGE_FUNC_PARAMS(OnMapDownloadEnd, "MapDownloadEnd", pKv);
    MESSAGE_FUNC_PARAMS(OnMapCacheUpdated, "MapCacheUpdated", pKv) {}
protected:
    virtual void OnCommand(const char *command);

    // updates map count
    void UpdateStatus();

    virtual void AddMapToList(MapData *pData);

    // Removes map from list
    void RemoveMap(MapDisplay_t&);

    //Clears the list of maps
    void ClearMapList();

    virtual int GetInvalidMapListID();
    MapDisplay_t *GetMapDisplayByID(uint32 id);

    virtual void OnTabSelected();
    virtual void GetNewMapList();
    virtual void OnGetNewMapList();

    CMapListPanel *m_pMapList;

    CUtlMap<uint32, MapDisplay_t> m_mapMaps;

private:
    vgui::HFont m_hFont;

    Color m_cMapDLFailed, m_cMapDLSuccess;
    uint32 m_uStartMapWhenReady;
};