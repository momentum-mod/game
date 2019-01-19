#pragma once

struct MapData;

// Used by the MapSelectorDialog, encapsulates a map object for the list
struct mapdisplay_t
{
    mapdisplay_t()
    {
        m_iMapImageIndex = 1; // Defaults to 1 as it's the invalid map index
        m_iListID = -1;
        m_bDoNotRefresh = true;
        m_pMap = nullptr;
    }
    MapData *m_pMap;      // the map struct, containing the information for the map
    int m_iListID;        // the VGUI2 list panel index for displaying this server
    int m_iMapImageIndex; // the map's image index in the map list's image list
    bool m_bDoNotRefresh;
    bool operator==(const mapdisplay_t &rhs) const { return m_iListID == rhs.m_iListID; }
};

//-----------------------------------------------------------------------------
// Purpose: Interface to accessing a game list
//-----------------------------------------------------------------------------
class IMapList
{
  public:
    enum InterfaceItem_e
    {
        FILTERS,
        GETNEWLIST,       // MOM_TODO: Change this to be "MAPSEARCH" ? Local uses its own update methods
        ADDSERVER,        // MOM_TODO: remove?
        ADDCURRENTSERVER, // MOM_TODO: remove?
    };

    // returns true if the game list supports the specified ui elements
    virtual bool SupportsItem(InterfaceItem_e item) = 0;

    // starts the servers refreshing
    virtual void StartRefresh() = 0;

    // gets a new map list
    virtual void GetNewMapList() = 0;

    // stops current refresh/GetNewServerList()
    virtual void StopRefresh() = 0;

    // Loads the filters from disk
    virtual void LoadFilters() = 0;

    // returns true if the list is currently refreshing servers
    virtual bool IsRefreshing() = 0;

    // called when Connect button is pressed
    virtual void OnMapStart() = 0;

    // invalid server index
    virtual int GetInvalidMapListID() = 0;
};