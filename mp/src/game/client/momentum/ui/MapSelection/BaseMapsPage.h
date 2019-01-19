//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once

#include "IMapList.h"
#include "vgui_controls/PropertyPage.h"

#define KEYNAME_MAP_NAME "Name"
#define KEYNAME_MAP_LAYOUT "MapLayout"
#define KEYNAME_MAP_DIFFICULTY "difficulty"
#define KEYNAME_MAP_BEST_TIME "time"
#define KEYNAME_MAP_IMAGE "MapImage"
#define KEYNAME_MAP_PATH "MapPath"
#define KEYNAME_MAP_ZONE_COUNT "ZoneCount"
#define KEYNAME_MAP_ZONE_PATH "ZonePath"
#define KEYNAME_MAP_HASH "Hash"
#define KEYNAME_MAP_ID "id"

class MapFilterPanel;
class CMapListPanel;
struct MapData;

//-----------------------------------------------------------------------------
// Purpose: Base property page for all the games lists (internet/favorites/lan/etc.)
//-----------------------------------------------------------------------------
class CBaseMapsPage : public vgui::PropertyPage, public IMapList
{
    DECLARE_CLASS_SIMPLE(CBaseMapsPage, vgui::PropertyPage);

public:
    CBaseMapsPage(vgui::Panel *parent, const char *name);
    ~CBaseMapsPage();

    virtual void PerformLayout();
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

    //uint32 GetServerFilters(MatchMakingKeyValuePair_t **pFilters); Used by server browser, this will translate
    //into API call filters
    
    // loads filter settings from disk
    virtual void LoadFilters();

    // Called by CGameList when the enter key is pressed.
    // This is overridden in the add server dialog - since there is no Connect button, the message
    // never gets handled, but we want to add a server when they dbl-click or press enter.
    virtual bool OnGameListEnterPressed();

    int GetSelectedItemsCount();

    // applies games filters to current list
    void ApplyFilters(MapFilterPanel *pFilterPanel);
    
    //STEAM_CALLBACK(CBaseMapsPage, OnFavoritesMsg, FavoritesListChanged_t, m_CallbackFavoritesMsg);
    //MOM_TODO: STEAM_CALLBACK for the HTTPS requests for maps
protected:
    virtual void OnCommand(const char *command);
    virtual void OnKeyCodePressed(vgui::KeyCode code);
    
    MESSAGE_FUNC(OnItemSelected, "ItemSelected");

    // updates server count UI
    void UpdateStatus();

    // Removes map from list
    void RemoveMap(mapdisplay_t&);

    //MOM_TODO: Correlate this to online maps
    virtual bool BShowMap(mapdisplay_t &server) { return server.m_bDoNotRefresh; }

    //Clears the list of maps
    void ClearMapList();

    virtual int GetInvalidMapListID();

    virtual void GetNewMapList();
    //MOM_TODO: Make these methods "search" for maps based on filter data
    virtual void StartRefresh();
    virtual void StopRefresh();
    virtual bool IsRefreshing();
    virtual void SetRefreshing(bool state);
    virtual void OnPageShow();
    virtual void OnPageHide();

    // called when Connect button is pressed
    MESSAGE_FUNC(OnMapStart, "StartMap");
    // called to look at game info
    MESSAGE_FUNC(OnViewMapInfo, "ViewMapInfo");
    // refreshes a single server
    MESSAGE_FUNC_INT(OnRefreshServer, "RefreshServer", serverID);

    // If true, then we automatically select the first item that comes into the games list.
    bool m_bAutoSelectFirstItemInGameList;

    CMapListPanel *m_pMapList;

    // command buttons
    // MOM_TODO: "Search" button


    CUtlVector<mapdisplay_t> m_vecMaps;

    int m_iOnlineMapsCount;

private:
    KeyValues *m_pFilters; // base filter data
    vgui::HFont m_hFont;

    typedef enum
    {
        HEADER_MAP_IMAGE = 0,
        HEADER_MAP_NAME,
        HEADER_MAP_LAYOUT,
        HEADER_DIFFICULTY,
        HEADER_BESTTIME
    } HEADERS;

};