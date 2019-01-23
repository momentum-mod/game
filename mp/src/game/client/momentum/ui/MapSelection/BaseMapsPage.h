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

//-----------------------------------------------------------------------------
// Purpose: Base property page for all the games lists (internet/favorites/lan/etc.)
//-----------------------------------------------------------------------------
class CBaseMapsPage : public vgui::PropertyPage, public IMapList, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(CBaseMapsPage, vgui::PropertyPage);

public:
    CBaseMapsPage(Panel *parent, const char *name);
    ~CBaseMapsPage();

    virtual void PerformLayout();
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    

    // Called by CGameList when the enter key is pressed.
    // This is overridden in the add server dialog - since there is no Connect button, the message
    // never gets handled, but we want to add a server when they dbl-click or press enter.
    virtual bool OnGameListEnterPressed();

    int GetSelectedItemsCount();

    // Filters
    // loads filter settings from disk
    virtual void LoadFilters();
    virtual KeyValues *GetFilters();
    void ApplyFilters(KeyValues *pFilters) OVERRIDE;
    virtual bool MapPassesFilters(MapData *pData, KeyValues *pFilters);
    
protected:
    virtual void OnCommand(const char *command);
    virtual void OnKeyCodePressed(vgui::KeyCode code);

    void FireGameEvent(IGameEvent* event) OVERRIDE;
    
    MESSAGE_FUNC(OnItemSelected, "ItemSelected");

    // updates server count UI
    void UpdateStatus();

    virtual void AddMapToList(MapData *pData);
    void UpdateMapListData(MapDisplay_t *pMap, bool bMain, bool bInfo, bool bPB, bool bWR, bool bThumbnail);

    // Removes map from list
    void RemoveMap(MapDisplay_t&);

    //Clears the list of maps
    void ClearMapList();

    virtual int GetInvalidMapListID();
    MapDisplay_t *GetMapDisplayByID(uint32 id);

    virtual void GetNewMapList();
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

    // If true, then we automatically select the first item that comes into the games list.
    bool m_bAutoSelectFirstItemInGameList;

    CMapListPanel *m_pMapList;

    CUtlVector<MapDisplay_t> m_vecMaps;

    int m_iOnlineMapsCount;

private:
    vgui::HFont m_hFont;

    typedef enum
    {
        HEADER_MAP_IMAGE = 0,
        HEADER_MAP_NAME,
        HEADER_MAP_LAYOUT,
        HEADER_DIFFICULTY,
        HEADER_WORLD_RECORD,
        HEADER_BEST_TIME
    } HEADERS;

};