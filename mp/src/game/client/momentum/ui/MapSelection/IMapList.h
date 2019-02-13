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

// Used by map filter panel
struct MapFilters_t
{
    MapFilters_t()
    {
        m_iDifficultyLow = m_iDifficultyHigh = m_iMapLayout = m_iGameMode = -1;
        m_szMapName[0] = '\0';
        m_bHideCompleted = false;
    }

    char m_szMapName[MAX_MAP_NAME];
    int m_iDifficultyLow; // Lower bound for the difficulty, maps have to be >= this
    int m_iDifficultyHigh; // High bound, maps have to be <= this
    int m_iMapLayout; // Map layout (linear/staged)
    int m_iGameMode; // Game mode of the map
    bool m_bHideCompleted; //Hide completed maps

    void ToKV(KeyValues *pInto)
    {
        pInto->SetInt("type", m_iGameMode);
        pInto->SetString("name", m_szMapName);
        pInto->SetInt("difficulty_low", m_iDifficultyLow);
        pInto->SetInt("difficulty_high", m_iDifficultyHigh);
        pInto->SetBool("HideCompleted", m_bHideCompleted);
        pInto->SetInt("layout", m_iMapLayout);
    }
    void FromKV(KeyValues *pFrom)
    {
        //Game-mode selection
        m_iGameMode = pFrom->GetInt("type");

        //"Map"
        Q_strncpy(m_szMapName, pFrom->GetString("name"), sizeof(m_szMapName));

        //Map layout
        m_iMapLayout = pFrom->GetInt("layout");

        //HideCompleted maps
        m_bHideCompleted = pFrom->GetBool("HideCompleted");

        //Difficulty
        m_iDifficultyLow = pFrom->GetInt("difficulty_low");
        m_iDifficultyHigh = pFrom->GetInt("difficulty_high");
    }
    void Reset()
    {
        m_szMapName[0] = '\0';
        m_iDifficultyLow = m_iDifficultyHigh = m_iMapLayout = m_iGameMode = 0;
        m_bHideCompleted = false;
    }
    void operator=(const MapFilters_t &other)
    {
        Q_strncpy(m_szMapName, other.m_szMapName, sizeof(m_szMapName));
        m_iDifficultyLow = other.m_iDifficultyLow;
        m_iDifficultyHigh = other.m_iDifficultyHigh;
        m_iMapLayout = other.m_iMapLayout;
        m_iGameMode = other.m_iGameMode;
        m_bHideCompleted = other.m_bHideCompleted;
    }
    bool operator==(const MapFilters_t &other) const 
    {
        return FStrEq(m_szMapName, other.m_szMapName) && m_iDifficultyLow == other.m_iDifficultyLow &&
        m_iDifficultyHigh == other.m_iDifficultyHigh && m_iMapLayout == other.m_iMapLayout && m_iGameMode == other.m_iGameMode
        && m_bHideCompleted == other.m_bHideCompleted;
    }
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
    virtual void ApplyFilters(MapFilters_t filters) = 0;

    // invalid server index
    virtual int GetInvalidMapListID() = 0;
};