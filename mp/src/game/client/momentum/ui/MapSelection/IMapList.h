#pragma once

struct MapData;

namespace vgui
{
    class IImage;
}

// Used by the MapSelectorDialog, encapsulates a map object for the list
struct MapDisplay_t
{
    MapDisplay_t()
    {
        m_iMapImageIndex = 1; // Defaults to 1 as it's the invalid map index
        m_iListID = -1;
        m_bNeedsShown = true;
        m_bNeedsUpdate = true;
        m_pMap = nullptr;
        m_pImage = nullptr;
    }
    MapData *m_pMap;      // the map struct, containing the information for the map
    int m_iListID;        // the VGUI2 list panel index for displaying this server
    int m_iMapImageIndex; // the map's image index in the map list's image list
    vgui::IImage *m_pImage; // The map's image
    bool m_bNeedsShown, m_bNeedsUpdate;
    bool operator==(const MapDisplay_t &rhs) const { return m_iListID == rhs.m_iListID; }
};

enum MapListType_e
{
    MAP_LIST_BROWSE = 0,
    MAP_LIST_LIBRARY,
    MAP_LIST_FAVORITES,
    MAP_LIST_TESTING,
};

//-----------------------------------------------------------------------------
// Purpose: Interface to accessing a game list
//-----------------------------------------------------------------------------
abstract_class IMapList
{
  public:
    // Gets the map list type
    virtual MapListType_e GetMapListType() = 0;

    // gets a new map list
    virtual void GetNewMapList() = 0;

    // Loads the filters from disk
    virtual void LoadFilters() = 0;

    // Applies filters to the list
    virtual void ApplyFilters(KeyValues *pFilters) = 0;

    // invalid server index
    virtual int GetInvalidMapListID() = 0;
};